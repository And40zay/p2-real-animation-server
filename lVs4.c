#include "animate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct sprite {
    enum { BITMAP, RECTANGLE, CIRCLE } type;
    union {
        struct {
            size_t width, height;
            color_t* pixels;  // ARGB32, row-major
        } bitmap;
        struct {
            size_t width, height;
            color_t color;
            bool filled;
        } rect;
        struct {
            size_t radius;
            color_t color;
        } circle;
    };
    int ref_count;
};

struct sprite_placement {
    struct sprite* sprite;
    ssize_t x, y;  // Initial position
    ssize_t vx, vy, ax, ay;
    animate_fn custom_fn;
    void* priv;
    struct sprite_placement* next;  // Doubly-linked for layers
    struct sprite_placement* prev;
};

struct canvas {
    size_t height, width;
    color_t background_color;
    struct sprite_placement* top_layer;
    struct sprite_placement* bottom_layer;
};


/* This is the header structure that we can expect to find at position 0 in a bitmap file. */
struct bitmap_header 
{
    uint8_t  magic[2];          // Expect {'B', 'M'}
    uint32_t size_bytes;        // Size of the file in bytes
    uint16_t reserved[2];
    uint32_t pixel_offset;      // Starting address of pixel data
// Don't pad this struct for alignment
} __attribute__((packed));

/*
 * This header immediately follows bitmap_header in the file.
 * You may ignore all fields except bV5Width and bV5Height, unless you'd like to validate
 * the image format
 */
struct bitmap5_header {
                               // Offset     Size Description
    uint32_t bV5Size         ; // 0x00          4 Size of this header (124 bytes)
    uint32_t bV5Width        ; // 0x04          4 Width of the bitmap in pixels
    uint32_t bV5Height       ; // 0x08          4 Height of the bitmap in pixels
    uint16_t bV5Planes       ; // 0x0C          2 Number of planes (must be 1)
    uint16_t bV5BitCount     ; // 0x0E          2 Bits per pixel (e.g., 32)
    uint32_t bV5Compression  ; // 0x10          4 BI_RGB (0), BI_BITFIELDS (3)
    uint32_t bV5SizeImage    ; // 0x14          4 Size of image data (0 if uncompressed)
    uint32_t bV5XPelsPerMeter; // 0x18          4 Horizontal pixels per meter
    uint32_t bV5YPelsPerMeter; // 0x1C          4 Vertical pixels per meter
    uint32_t bV5ClrUsed      ; // 0x20          4 Number of color indices used
    uint32_t bV5ClrImportant ; // 0x24          4 Number of important colors
    uint32_t bV5RedMask      ; // 0x28          4 Color mask for red component
    uint32_t bV5GreenMask    ; // 0x2C          4 Color mask for green component
    uint32_t bV5BlueMask     ; // 0x30          4 Color mask for blue component
    uint32_t bV5AlphaMask    ; // 0x34          4 Color mask for alpha channel
    uint32_t bV5CSType       ; // 0x38          4 Color space type (e.g., LCS_CALIBRATED_RGB)
    uint8_t  bV5Endpoints[36]; // 0x3C-0x5B    36 CIE XYZ color space endpoints
    uint32_t bV5GammaRed     ; // 0x5C          4 Gamma red component
    uint32_t bV5GammaGreen   ; // 0x60          4 Gamma green component
    uint32_t bV5GammaBlue    ; // 0x64          4 Gamma blue component
    uint32_t bV5Intent       ; // 0x68          4 Rendering intent
    uint32_t bV5ProfileData  ; // 0x6C          4 Offset to ICC profile data
    uint32_t bV5ProfileSize  ; // 0x70          4 Size of embedded profile data
    uint32_t bV5Reserved     ; // 0x74          4 Reserved (must be 0)
};


struct canvas* animate_create_canvas(size_t height, size_t width, color_t background_color) {
    struct canvas* canvas = malloc(sizeof(struct canvas));
    canvas->height = height;
    canvas->width = width;
    canvas->background_color = background_color;
    canvas->top_layer = NULL;
    canvas->bottom_layer = NULL;
    return canvas;
}

struct sprite* animate_create_sprite(const char* file) {
    FILE *fp = fopen(file, "rb");
    if (fp == NULL) return NULL;
    struct bitmap_header bmp_header;
    fread(&bmp_header, sizeof(struct bitmap_header), 1, fp);
    if (bmp_header.magic[0] != 'B' || bmp_header.magic[1] != 'M') {
        fclose(fp);
        return NULL;
    }
    struct bitmap5_header bmp5_header;
    fread(&bmp5_header, sizeof(struct bitmap5_header), 1, fp);
    size_t width = bmp5_header.bV5Width, height = bmp5_header.bV5Height;
    fseek(fp, bmp_header.pixel_offset, SEEK_SET);
    color_t* pixels = malloc(width * height * sizeof(color_t));
    if (!pixels) { fclose(fp); return NULL; }
    // Reverse rows
    for (size_t y = 0; y < height; y++) {
        fread(&pixels[(height - 1 - y) * width], sizeof(color_t), width, fp);
    }
    fclose(fp);
    struct sprite* sprite = malloc(sizeof(struct sprite));
    if (!sprite) { free(pixels); return NULL; }
    sprite->type = BITMAP;
    sprite->bitmap.width = width;
    sprite->bitmap.height = height;
    sprite->bitmap.pixels = pixels;
    sprite->ref_count = 0;
    return sprite;
}

struct sprite* animate_create_circle(size_t radius, color_t c, bool filled) {
    (void)filled;
    struct sprite* sprite = malloc(sizeof(struct sprite));
    if (!sprite) return NULL;
    sprite->type = CIRCLE;
    sprite->circle.radius = radius;
    sprite->circle.color = c;
    sprite->ref_count = 0;
    return sprite;
}

struct sprite* animate_create_rectangle(size_t width, size_t height, color_t c, bool filled) {
    struct sprite* sprite = malloc(sizeof(struct sprite));
    sprite->width = width;  // Keep for compatibility, but use union
    sprite->height = height;
    sprite->filled = filled;
    sprite->pixels = malloc(width * height * sizeof(color_t));
    for (size_t i = 0; i < width * height; i++) {
        sprite->pixels[i] = c;
    }
    // Update to use union
    sprite->type = RECTANGLE;
    sprite->rect.width = width;
    sprite->rect.height = height;
    sprite->rect.color = c;
    sprite->rect.filled = filled;
    free(sprite->pixels);  // Remove old, use union
    sprite->ref_count = 0;
    return sprite;
}

// Helper: Ref/unref sprite
static void sprite_ref(struct sprite* s) { if (s) s->ref_count++; }
static bool sprite_unref(struct sprite* s) {
    if (!s) return false;
    s->ref_count--;
    if (s->ref_count == 0) {
        if (s->type == BITMAP && s->bitmap.pixels) free(s->bitmap.pixels);
        free(s);
        return true;
    }
    return false;
}

// Helper: Calc position
static void calc_position(struct sprite_placement* p, float t, ssize_t* x, ssize_t* y) {
    if (p->custom_fn) {
        p->custom_fn(p->priv, x, y, t);
    } else {
        *x = p->x + p->vx * t + 0.5 * p->ax * t * t;
        *y = p->y + p->vy * t + 0.5 * p->ay * t * t;
    }
}

// Helper: Draw sprite
static void draw_sprite(color_t* buf, size_t fh, size_t fw, struct sprite* s, ssize_t sx, ssize_t sy) {
    if (!s) return;
    size_t sw, sh;
    if (s->type == BITMAP) {
        sw = s->bitmap.width;
        sh = s->bitmap.height;
    } else if (s->type == RECTANGLE) {
        sw = s->rect.width;
        sh = s->rect.height;
    } else {
        sw = sh = 2 * s->circle.radius + 1;
    }
    for (size_t dy = 0; dy < sh; dy++) {
        for (size_t dx = 0; dx < sw; dx++) {
            ssize_t fx = sx + dx, fy = sy + dy;
            if (fx < 0 || fy < 0 || fx >= fw || fy >= fh) continue;
            color_t c = 0;
            bool draw = false;
            if (s->type == BITMAP) {
                c = s->bitmap.pixels[dy * sw + dx];
                draw = ((c >> 24) & 0xFF) != 0;
            } else if (s->type == RECTANGLE) {
                if (s->rect.filled || dx == 0 || dy == 0 || dx == sw - 1 || dy == sh - 1) {
                    c = s->rect.color;
                    draw = true;
                }
            } else {
                ssize_t cx = s->circle.radius, cy = s->circle.radius;
                if ((dx - cx) * (dx - cx) + (dy - cy) * (dy - cy) <= s->circle.radius * s->circle.radius) {
                    c = s->circle.color;
                    draw = true;
                }
            }
            if (draw) buf[fy * fw + fx] = (c & 0x00FFFFFF) | 0xFF000000;
        }
    }
}

struct canvas* animate_create_canvas(size_t height, size_t width, color_t background_color) {
    struct canvas* canvas = malloc(sizeof(struct canvas));
    canvas->height = height;
    canvas->width = width;
    canvas->background_color = background_color;
    canvas->top_layer = NULL;
    canvas->bottom_layer = NULL;
    return canvas;
}

struct sprite* animate_create_sprite(const char* file) {
    FILE *fp = fopen(file, "rb");
    if (fp == NULL) return NULL;
    struct bitmap_header bmp_header;
    fread(&bmp_header, sizeof(struct bitmap_header), 1, fp);
    if (bmp_header.magic[0] != 'B' || bmp_header.magic[1] != 'M') {
        fclose(fp);
        return NULL;
    }
    struct bitmap5_header bmp5_header;
    fread(&bmp5_header, sizeof(struct bitmap5_header), 1, fp);
    size_t width = bmp5_header.bV5Width, height = bmp5_header.bV5Height;
    fseek(fp, bmp_header.pixel_offset, SEEK_SET);
    color_t* pixels = malloc(width * height * sizeof(color_t));
    if (!pixels) { fclose(fp); return NULL; }
    // Reverse rows
    for (size_t y = 0; y < height; y++) {
        fread(&pixels[(height - 1 - y) * width], sizeof(color_t), width, fp);
    }
    fclose(fp);
    struct sprite* sprite = malloc(sizeof(struct sprite));
    if (!sprite) { free(pixels); return NULL; }
    sprite->type = BITMAP;
    sprite->bitmap.width = width;
    sprite->bitmap.height = height;
    sprite->bitmap.pixels = pixels;
    sprite->ref_count = 0;
    return sprite;
}

struct sprite* animate_create_circle(size_t radius, color_t c, bool filled) {
    (void)filled;
    struct sprite* sprite = malloc(sizeof(struct sprite));
    if (!sprite) return NULL;
    sprite->type = CIRCLE;
    sprite->circle.radius = radius;
    sprite->circle.color = c;
    sprite->ref_count = 0;
    return sprite;
}

struct sprite* animate_create_rectangle(size_t width, size_t height, color_t c, bool filled) {
    struct sprite* sprite = malloc(sizeof(struct sprite));
    sprite->width = width;  // Keep for compatibility, but use union
    sprite->height = height;
    sprite->filled = filled;
    sprite->pixels = malloc(width * height * sizeof(color_t));
    for (size_t i = 0; i < width * height; i++) {
        sprite->pixels[i] = c;
    }
    // Update to use union
    sprite->type = RECTANGLE;
    sprite->rect.width = width;
    sprite->rect.height = height;
    sprite->rect.color = c;
    sprite->rect.filled = filled;
    free(sprite->pixels);  // Remove old, use union
    sprite->ref_count = 0;
    return sprite;
}

// Helper: Ref/unref sprite
static void sprite_ref(struct sprite* s) { if (s) s->ref_count++; }
static bool sprite_unref(struct sprite* s) {
    if (!s) return false;
    s->ref_count--;
    if (s->ref_count == 0) {
        if (s->type == BITMAP && s->bitmap.pixels) free(s->bitmap.pixels);
        free(s);
        return true;
    }
    return false;
}

// Helper: Calc position
static void calc_position(struct sprite_placement* p, float t, ssize_t* x, ssize_t* y) {
    if (p->custom_fn) {
        p->custom_fn(p->priv, x, y, t);
    } else {
        *x = p->x + p->vx * t + 0.5 * p->ax * t * t;
        *y = p->y + p->vy * t + 0.5 * p->ay * t * t;
    }
}

// Helper: Draw sprite
static void draw_sprite(color_t* buf, size_t fh, size_t fw, struct sprite* s, ssize_t sx, ssize_t sy) {
    if (!s) return;
    size_t sw, sh;
    if (s->type == BITMAP) {
        sw = s->bitmap.width;
        sh = s->bitmap.height;
    } else if (s->type == RECTANGLE) {
        sw = s->rect.width;
        sh = s->rect.height;
    } else {
        sw = sh = 2 * s->circle.radius + 1;
    }
    for (size_t dy = 0; dy < sh; dy++) {
        for (size_t dx = 0; dx < sw; dx++) {
            ssize_t fx = sx + dx, fy = sy + dy;
            if (fx < 0 || fy < 0 || fx >= fw || fy >= fh) continue;
            color_t c = 0;
            bool draw = false;
            if (s->type == BITMAP) {
                c = s->bitmap.pixels[dy * sw + dx];
                draw = ((c >> 24) & 0xFF) != 0;
            } else if (s->type == RECTANGLE) {
                if (s->rect.filled || dx == 0 || dy == 0 || dx == sw - 1 || dy == sh - 1) {
                    c = s->rect.color;
                    draw = true;
                }
            } else {
                ssize_t cx = s->circle.radius, cy = s->circle.radius;
                if ((dx - cx) * (dx - cx) + (dy - cy) * (dy - cy) <= s->circle.radius * s->circle.radius) {
                    c = s->circle.color;
                    draw = true;
                }
            }
            if (draw) buf[fy * fw + fx] = (c & 0x00FFFFFF) | 0xFF000000;
        }
    }
}

struct canvas* animate_create_canvas(size_t height, size_t width, color_t background_color) {
    struct canvas* canvas = malloc(sizeof(struct canvas));
    canvas->height = height;
    canvas->width = width;
    canvas->background_color = background_color;
    canvas->top_layer = NULL;
    canvas->bottom_layer = NULL;
    return canvas;
}

struct sprite* animate_create_sprite(const char* file) {
    FILE *fp = fopen(file, "rb");
    if (fp == NULL) return NULL;
    struct bitmap_header bmp_header;
    fread(&bmp_header, sizeof(struct bitmap_header), 1, fp);
    if (bmp_header.magic[0] != 'B' || bmp_header.magic[1] != 'M') {
        fclose(fp);
        return NULL;
    }
    struct bitmap5_header bmp5_header;
    fread(&bmp5_header, sizeof(struct bitmap5_header), 1, fp);
    size_t width = bmp5_header.bV5Width, height = bmp5_header.bV5Height;
    fseek(fp, bmp_header.pixel_offset, SEEK_SET);
    color_t* pixels = malloc(width * height * sizeof(color_t));
    if (!pixels) { fclose(fp); return NULL; }
    // Reverse rows
    for (size_t y = 0; y < height; y++) {
        fread(&pixels[(height - 1 - y) * width], sizeof(color_t), width, fp);
    }
    fclose(fp);
    struct sprite* sprite = malloc(sizeof(struct sprite));
    if (!sprite) { free(pixels); return NULL; }
    sprite->type = BITMAP;
    sprite->bitmap.width = width;
    sprite->bitmap.height = height;
    sprite->bitmap.pixels = pixels;
    sprite->ref_count = 0;
    return sprite;
}

struct sprite* animate_create_circle(size_t radius, color_t c, bool filled) {
    (void)filled;
    struct sprite* sprite = malloc(sizeof(struct sprite));
    if (!sprite) return NULL;
    sprite->type = CIRCLE;
    sprite->circle.radius = radius;
    sprite->circle.color = c;
    sprite->ref_count = 0;
    return sprite;
}

struct sprite* animate_create_rectangle(size_t width, size_t height, color_t c, bool filled) {
    struct sprite* sprite = malloc(sizeof(struct sprite));
    sprite->width = width;  // Keep for compatibility, but use union
    sprite->height = height;
    sprite->filled = filled;
    sprite->pixels = malloc(width * height * sizeof(color_t));
    for (size_t i = 0; i < width * height; i++) {
        sprite->pixels[i] = c;
    }
    // Update to use union
    sprite->type = RECTANGLE;
    sprite->rect.width = width;
    sprite->rect.height = height;
    sprite->rect.color = c;
    sprite->rect.filled = filled;
    free(sprite->pixels);  // Remove old, use union
    sprite->ref_count = 0;
    return sprite;
}

bool animate_destroy_sprite(struct sprite* sprite) {
    return sprite_unref(sprite);
}

struct sprite_placement* animate_place_sprite(struct canvas* canvas, struct sprite* sprite, ssize_t x, ssize_t y) {
    struct sprite_placement* placement = malloc(sizeof(struct sprite_placement));
    placement->sprite = sprite;
    sprite_ref(sprite);
    placement->x = x;
    placement->y = y;
    placement->vx = placement->vy = placement->ax = placement->ay = 0;
    placement->custom_fn = NULL;
    placement->priv = NULL;
    placement->next = NULL;
    placement->prev = canvas->top_layer;
    if (canvas->top_layer) canvas->top_layer->next = placement;
    else canvas->bottom_layer = placement;
    canvas->top_layer = placement;
    return placement;
}

void animate_placement_up(struct sprite_placement* sprite_placement) {
    if (!sprite_placement || !sprite_placement->next) return;
    // Swap with next
    struct sprite_placement* next = sprite_placement->next;
    if (sprite_placement->prev) sprite_placement->prev->next = next;
    next->prev = sprite_placement->prev;
    sprite_placement->prev = next;
    sprite_placement->next = next->next;
    if (next->next) next->next->prev = sprite_placement;
    next->next = sprite_placement;
    // Update canvas top
    struct canvas* c = NULL;  // Need to find canvas; assume from placement (hacky)
    // For simplicity, assume canvas is known; in real, store in placement
    // Placeholder: implement swap logic
}

void animate_placement_down(struct sprite_placement* sprite_placement) {
    if (!sprite_placement || !sprite_placement->prev) return;
    // Similar to up, swap with prev
    // ...existing code... (implement swap)
}

void animate_placement_top(struct sprite_placement* sprite_placement) {
    // Move to top
    // ...existing code... (remove and reinsert)
}

void animate_placement_bottom(struct sprite_placement* sprite_placement) {
    // Move to bottom
    // ...existing code... (remove and reinsert)
}

void animate_destroy_placement(struct sprite_placement* sprite_placement) {
    // Remove from canvas list
    if (sprite_placement->prev) sprite_placement->prev->next = sprite_placement->next;
    if (sprite_placement->next) sprite_placement->next->prev = sprite_placement->prev;
    // Update canvas top/bottom
    // ...existing code...
    sprite_unref(sprite_placement->sprite);
    free(sprite_placement);
}

void animate_set_animation_params(struct sprite_placement* sprite_placement, ssize_t vx, ssize_t vy, ssize_t ax, ssize_t ay) {
    sprite_placement->vx = vx;
    sprite_placement->vy = vy;
    sprite_placement->ax = ax;
    sprite_placement->ay = ay;
}

void animate_destroy_canvas(struct canvas* canvas) {
    struct sprite_placement* p = canvas->bottom_layer;
    while (p) {
        struct sprite_placement* next = p->next;
        sprite_unref(p->sprite);
        free(p);
        p = next;
    }
    free(canvas);
}

size_t animate_frame_size_bytes(struct canvas* canvas) {
    return canvas->height * canvas->width * sizeof(color_t);
}

void animate_generate_frame(const struct canvas* canvas, size_t frame, size_t frame_rate, void* buf) {
    color_t* frame_buf = buf;
    size_t pixels = canvas->height * canvas->width;
    for (size_t i = 0; i < pixels; i++) frame_buf[i] = canvas->background_color | 0xFF000000;
    float t = (float)frame / frame_rate;
    struct sprite_placement* p = canvas->bottom_layer;
    while (p) {
        ssize_t x, y;
        calc_position(p, t, &x, &y);
        draw_sprite(frame_buf, canvas->height, canvas->width, p->sprite, x, y);
        p = p->next;
    }
}

// Optional extension
void animate_set_animation_function(struct sprite_placement* sprite_placement, animate_fn fn, void* priv) {
    sprite_placement->custom_fn = fn;
    sprite_placement->priv = priv;
}

