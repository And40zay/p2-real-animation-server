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
    int ref_count;
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

static void sprite_ref(struct sprite* s) { if (s) s->ref_count++; }

static bool sprite_unref(struct sprite* s) {
    if (!s) return false;
    if (s->ref_count <= 0) return false;
    s->ref_count--;
    if (s->ref_count == 0) { free(s->pixels); free(s); return true; }
    return false;
}

// Loads BMP (ARGB32), reversing rows
static struct sprite* load_bmp(const char* file) {
    FILE* fp = fopen(file, "rb");
    if (!fp) return NULL;

    struct bitmap_header bmp_header;
    if (fread(&bmp_header, sizeof(bmp_header), 1, fp) != 1 ||
        bmp_header.magic[0] != 'B' || bmp_header.magic[1] != 'M') {
        fclose(fp);
        return NULL;
    }

    struct bitmap5_header b5;
    if (fread(&b5, sizeof(b5), 1, fp) != 1) { fclose(fp); return NULL; }
    if (b5.size < 40) { fclose(fp); return NULL; }
    if (b5.bits_per_pixel != 32) { fclose(fp); return NULL; }
    if (!(b5.compression == 0 || b5.compression == 3)) { fclose(fp); return NULL; }

    int32_t abs_height = b5.bV5Height < 0 ? -b5.bV5Height : b5.bV5Height;
    if (b5.bV5Width <= 0 || abs_height == 0) { fclose(fp); return NULL; }
    size_t w = (size_t)b5.bV5Width;
    size_t h = (size_t)abs_height;

    size_t max_pixels = 0;
    if (bmp_header.file_size > bmp_header.pixel_offset)
        max_pixels = (bmp_header.file_size - bmp_header.pixel_offset) / sizeof(color_t);
    if (w == 0 || h == 0 || w > SIZE_MAX / sizeof(color_t) / h || (max_pixels && w > 0 && h > 0 && w * h > max_pixels)) {
        fclose(fp);
        return NULL;
    }

    size_t expected_bytes = w * h * sizeof(color_t);
    if (b5.image_size && b5.image_size < expected_bytes) { fclose(fp); return NULL; }

    struct sprite* s = malloc(sizeof(*s));
    if (!s) { fclose(fp); return NULL; }
    s->type = SPRITE_BITMAP;
    s->width = w;
    s->height = h;
    s->ref_count = 1;
    s->pixels = malloc(expected_bytes);
    if (!s->pixels) { free(s); fclose(fp); return NULL; }

    if (fseek(fp, bmp_header.pixel_offset, SEEK_SET) != 0) {
        free(s->pixels); free(s); fclose(fp); return NULL;
    }

    // Always flip rows: first row in file is last row in memory
    for (size_t y = 0; y < h; y++) {
        size_t dest_y = h - 1 - y;
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
    s->ref_count = 1;
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
    s->ref_count = 1;
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
        sprite_unref(p->sprite);
        free(p);
        p = next;
    }
    free(canvas);
}

struct sprite* animate_create_sprite(const char* file) 
{
    struct sprite* s = load_bmp(file);
    if (s) s->ref_count = 1;
    return s;
}

struct sprite* animate_create_circle(size_t radius, color_t c, bool filled) 
{
    (void)filled; // reserved
    struct sprite* s = make_circle(radius, c);
    if (s) s->ref_count = 1;
    return s;
}

struct sprite* animate_create_rectangle(size_t width, size_t height, color_t c, bool filled)
{
    struct sprite* s = make_rect(width, height, c, filled);
    if (s) s->ref_count = 1;
    return s;
}

bool animate_destroy_sprite(struct sprite* sprite) 
{
    // Only destroy if not in use (ref_count==1), else return false
    if (!sprite) return false;
    if (sprite->ref_count > 1) return false;
    return sprite_unref(sprite);
}

struct sprite_placement* animate_place_sprite(struct canvas* canvas, struct sprite* sprite, ssize_t x, ssize_t y) 
{
    if (!canvas || !sprite) return NULL;
    struct sprite_placement* placement = malloc(sizeof(struct sprite_placement));
    if (!placement) return NULL;
    placement->canvas = canvas;
    placement->sprite = sprite;
    placement->x = x;
    placement->y = y;
    placement->vx = placement->vy = placement->ax = placement->ay = 0;
    placement->custom_fn = NULL;
    placement->priv = NULL;
    placement->next = NULL;
    placement->prev = canvas->top_layer;

    sprite_ref(sprite);

    if (canvas->top_layer) canvas->top_layer->next = placement;
    else canvas->bottom_layer = placement;
    canvas->top_layer = placement;
    return placement;
}

static void swap_with_next(struct sprite_placement* p) {
    struct canvas* c = p->canvas;
    struct sprite_placement* n = p->next;
    if (!n) return;
    struct sprite_placement* before = p->prev;
    struct sprite_placement* after = n->next;

    if (before) before->next = n; else c->bottom_layer = n;
    if (after) after->prev = p;   else c->top_layer = p;

    n->prev = before;
    n->next = p;
    p->prev = n;
    p->next = after;
}

static void swap_with_prev(struct sprite_placement* p) {
    struct canvas* c = p->canvas;
    struct sprite_placement* n = p->prev;
    if (!n) return;
    struct sprite_placement* before = n->prev;
    struct sprite_placement* after = p->next;

    if (before) before->next = p; else c->bottom_layer = p;
    if (after) after->prev = n;   else c->top_layer = n;

    p->prev = before;
    p->next = n;
    n->prev = p;
    n->next = after;
}

void animate_placement_up(struct sprite_placement* sprite_placement)
{
    if (sprite_placement) swap_with_next(sprite_placement);
}

void animate_placement_down(struct sprite_placement* sprite_placement){
    if (sprite_placement) swap_with_prev(sprite_placement);
}

void animate_placement_top(struct sprite_placement* sprite_placement)
{
    if (!sprite_placement) return;
    while (sprite_placement->next) swap_with_next(sprite_placement);
}

void animate_placement_bottom(struct sprite_placement* sprite_placement)
{
    if (!sprite_placement) return;
    while (sprite_placement->prev) swap_with_prev(sprite_placement);
}

void animate_destroy_placement(struct sprite_placement* sprite_placement)
{
    if (!sprite_placement) return;
    struct canvas* c = sprite_placement->canvas;
    if (sprite_placement->prev) sprite_placement->prev->next = sprite_placement->next;
    else c->bottom_layer = sprite_placement->next;
    if (sprite_placement->next) sprite_placement->next->prev = sprite_placement->prev;
    else c->top_layer = sprite_placement->prev;
    sprite_unref(sprite_placement->sprite);
    free(sprite_placement);
}

void animate_set_animation_params(struct sprite_placement* sprite_placement, ssize_t vx, ssize_t vy, ssize_t ax, ssize_t ay)
{
    if (!sprite_placement) return;
    sprite_placement->vx = vx;
    sprite_placement->vy = vy;
    sprite_placement->ax = ax;
    sprite_placement->ay = ay;
}

void animate_set_animation_function(struct sprite_placement* sprite_placement, animate_fn fn, void* priv) 
{
    if (!sprite_placement) return;
    sprite_placement->custom_fn = fn;
    sprite_placement->priv = priv;
}

size_t animate_frame_size_bytes(struct canvas* canvas)
{
    if (!canvas) return 0;
    return canvas->height * canvas->width * sizeof(color_t);
}

static void draw_sprite(color_t* buf, size_t bw, size_t bh, struct sprite* s, ssize_t ox, ssize_t oy) {
    for (size_t y = 0; y < s->height; y++) {
        for (size_t x = 0; x < s->width; x++) {
            ssize_t fx = ox + (ssize_t)x;
            ssize_t fy = oy + (ssize_t)y;
            if (fx < 0 || fy < 0 || fx >= (ssize_t)bw || fy >= (ssize_t)bh) continue;
            color_t c = s->pixels[y * s->width + x];
            if ((c >> 24) & 0xFF) buf[fy * bw + fx] = (c & 0x00FFFFFF) | 0xFF000000;
        }
    }
}

void animate_generate_frame(const struct canvas* canvas, size_t frame, size_t frame_rate, void* buf) 
{
    if (!canvas || !buf) return;
    color_t* out = buf;
    size_t total = canvas->height * canvas->width;
    for (size_t i = 0; i < total; i++) out[i] = (canvas->background_color & 0x00FFFFFF) | 0xFF000000;

    float t = (float)frame / (float)frame_rate;
    struct sprite_placement* p = canvas->bottom_layer;
    while (p) {
        ssize_t cx = p->x;
        ssize_t cy = p->y;
        if (p->custom_fn) {
            p->custom_fn(p->priv, &cx, &cy, t);
        } else {
            cx = p->x + (ssize_t)(p->vx * t + 0.5f * p->ax * t * t);
            cy = p->y + (ssize_t)(p->vy * t + 0.5f * p->ay * t * t);
        }
        draw_sprite(out, canvas->width, canvas->height, p->sprite, cx, cy);
        p = p->next;
    }
}

