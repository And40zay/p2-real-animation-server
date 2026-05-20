#include "animate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

struct bitmap_header {
    uint8_t magic[2];
    uint32_t file_size;
    uint32_t reserved;
    uint32_t pixel_offset;
};

struct bitmap5_header {
    uint32_t size;
    int32_t  bV5Width;
    int32_t  bV5Height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t  x_pixels_per_meter;
    int32_t  y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
};

enum sprite_type { SPRITE_BITMAP, SPRITE_RECT, SPRITE_CIRCLE };

struct sprite {
    enum sprite_type type;
    size_t width, height;
    color_t* pixels;   // ARGB32
    int ref_count;     // active placement count (not owner count)
};

struct canvas {
    size_t height, width;
    color_t background_color;
    struct sprite_placement* top_layer;
    struct sprite_placement* bottom_layer;
};

struct sprite_placement {
    struct canvas* canvas;
    struct sprite* sprite;
    ssize_t x, y;
    ssize_t vx, vy, ax, ay;
    animate_fn custom_fn;
    void* priv;
    struct sprite_placement* next; // above
    struct sprite_placement* prev; // below
};

// Helpers -----------------------------------------------------

static void sprite_add_placement_ref(struct sprite* s) {
    if (s) s->ref_count++;
}

static void sprite_remove_placement_ref(struct sprite* s) {
    if (!s) return;
    if (s->ref_count > 0) s->ref_count--;
}

static bool read_exact(FILE* fp, void* dst, size_t n) {
    return fread(dst, 1, n, fp) == n;
}

static uint16_t le16(const uint8_t* p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t le32(const uint8_t* p) {
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

static int32_t le32s(const uint8_t* p) {
    return (int32_t)le32(p);
}

// Loads BMP (ARGB32), reversing rows
static struct sprite* load_bmp(const char* file) {
    FILE* fp = fopen(file, "rb");
    if (!fp) return NULL;

    uint8_t file_hdr[14];
    if (!read_exact(fp, file_hdr, sizeof(file_hdr))) { fclose(fp); return NULL; }
    if (file_hdr[0] != 'B' || file_hdr[1] != 'M') { fclose(fp); return NULL; }

    uint32_t bmp_file_size = le32(&file_hdr[2]);
    uint32_t pixel_offset  = le32(&file_hdr[10]);

    uint8_t dib_size_raw[4];
    if (!read_exact(fp, dib_size_raw, sizeof(dib_size_raw))) { fclose(fp); return NULL; }
    uint32_t dib_size = le32(dib_size_raw);
    if (dib_size < 40) { fclose(fp); return NULL; }

    uint8_t dib40_rest[36];
    if (!read_exact(fp, dib40_rest, sizeof(dib40_rest))) { fclose(fp); return NULL; }

    int32_t  bV5Width       = le32s(&dib40_rest[0]);
    int32_t  bV5Height      = le32s(&dib40_rest[4]);
    uint16_t planes         = le16(&dib40_rest[8]);
    uint16_t bits_per_pixel = le16(&dib40_rest[10]);
    uint32_t compression    = le32(&dib40_rest[12]);
    uint32_t image_size     = le32(&dib40_rest[16]);

    if (planes != 1) { fclose(fp); return NULL; }
    if (bits_per_pixel != 32) { fclose(fp); return NULL; }
    if (!(compression == 0 || compression == 3)) { fclose(fp); return NULL; }

    if (dib_size > 40) {
        long skip = (long)(dib_size - 40);
        if (skip < 0 || fseek(fp, skip, SEEK_CUR) != 0) { fclose(fp); return NULL; }
    }

    if (bV5Width <= 0 || bV5Height == 0) { fclose(fp); return NULL; }

    bool top_down = bV5Height < 0;
    int64_t h64 = top_down ? -(int64_t)bV5Height : (int64_t)bV5Height;
    if (h64 <= 0) { fclose(fp); return NULL; }

    size_t w = (size_t)bV5Width;
    size_t h = (size_t)h64;

    if (w == 0 || h == 0 || w > SIZE_MAX / h || (w * h) > SIZE_MAX / sizeof(color_t)) {
        fclose(fp);
        return NULL;
    }

    size_t expected_bytes = w * h * sizeof(color_t);
    if (image_size != 0 && image_size < expected_bytes) { fclose(fp); return NULL; }

    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return NULL; }
    long file_len_l = ftell(fp);
    if (file_len_l < 0) { fclose(fp); return NULL; }
    size_t file_len = (size_t)file_len_l;

    if ((size_t)pixel_offset > file_len) { fclose(fp); return NULL; }
    if (expected_bytes > file_len - (size_t)pixel_offset) { fclose(fp); return NULL; }

    if (bmp_file_size != 0 && (size_t)bmp_file_size < (size_t)pixel_offset) { fclose(fp); return NULL; }

    struct sprite* s = malloc(sizeof(*s));
    if (!s) { fclose(fp); return NULL; }

    s->type = SPRITE_BITMAP;
    s->width = w;
    s->height = h;
    s->ref_count = 0;
    s->pixels = malloc(expected_bytes);
    if (!s->pixels) { free(s); fclose(fp); return NULL; }

    if (fseek(fp, (long)pixel_offset, SEEK_SET) != 0) {
        free(s->pixels); free(s); fclose(fp); return NULL;
    }

    for (size_t y = 0; y < h; y++) {
        size_t dest_y = top_down ? y : (h - 1 - y);
        if (fread(&s->pixels[dest_y * w], sizeof(color_t), w, fp) != w) {
            free(s->pixels); free(s); fclose(fp); return NULL;
        }
    }

    fclose(fp);
    return s;
}

// Shapes as pixel sprites
static struct sprite* make_rect(size_t w, size_t h, color_t c, bool filled) {
    if (w == 0 || h == 0 || w > SIZE_MAX / sizeof(color_t) / h) return NULL;
    struct sprite* s = malloc(sizeof(*s));
    if (!s) return NULL;
    s->type = SPRITE_RECT;
    s->width = w;
    s->height = h;
    s->ref_count = 0;
    s->pixels = malloc(w * h * sizeof(color_t));
    if (!s->pixels) { free(s); return NULL; }

    for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++) {
            bool border = (x == 0 || y == 0 || x == w - 1 || y == h - 1);
            bool draw = filled || border;
            s->pixels[y * w + x] = draw ? ((c & 0x00FFFFFF) | 0xFF000000) : 0;
        }
    }
    return s;
}

static struct sprite* make_circle(size_t r, color_t c) {
    size_t d = 2 * r + 1;
    if (d == 0 || d > SIZE_MAX / sizeof(color_t) / d) return NULL;
    struct sprite* s = malloc(sizeof(*s));
    if (!s) return NULL;
    s->type = SPRITE_CIRCLE;
    s->width = d;
    s->height = d;
    s->ref_count = 0;
    s->pixels = malloc(d * d * sizeof(color_t));
    if (!s->pixels) { free(s); return NULL; }

    for (size_t y = 0; y < d; y++) {
        for (size_t x = 0; x < d; x++) {
            ssize_t dx = (ssize_t)x - (ssize_t)r;
            ssize_t dy = (ssize_t)y - (ssize_t)r;
            bool inside = dx * dx + dy * dy <= (ssize_t)(r * r);
            s->pixels[y * d + x] = inside ? ((c & 0x00FFFFFF) | 0xFF000000) : 0;
        }
    }
    return s;
}

// Core API ----------------------------------------------------

struct canvas* animate_create_canvas(size_t height, size_t width, color_t background_color)
{
    struct canvas* canvas = malloc(sizeof(struct canvas));
    if (!canvas) return NULL;
    canvas->height = height;
    canvas->width = width;
    canvas->background_color = background_color;
    canvas->top_layer = NULL;
    canvas->bottom_layer = NULL;
    return canvas;
}

void animate_destroy_canvas(struct canvas* canvas)
{
    if (!canvas) return;
    struct sprite_placement* p = canvas->bottom_layer;
    while (p) {
        struct sprite_placement* next = p->next;
        sprite_remove_placement_ref(p->sprite);
        free(p);
        p = next;
    }
    free(canvas);
}

struct sprite* animate_create_sprite(const char* file) 
{
    return load_bmp(file);
}

struct sprite* animate_create_circle(size_t radius, color_t c, bool filled) 
{
    (void)filled; // reserved
    return make_circle(radius, c);
}

struct sprite* animate_create_rectangle(size_t width, size_t height, color_t c, bool filled)
{
    return make_rect(width, height, c, filled);
}

bool animate_destroy_sprite(struct sprite* sprite) 
{
    if (!sprite) return false;
    if

