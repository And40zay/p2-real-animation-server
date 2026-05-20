#include "animate.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>  // For file I/O in create_sprite
#include <math.h>  // For circle calculations

// Opaque struct definitions
struct sprite {
    enum { BITMAP, RECTANGLE, CIRCLE } type;
    union {
        struct {
            size_t width, height;
            color_t* data;  // ARGB32, row-major
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
    ssize_t x0, y0;  // Initial position
    ssize_t vx, vy, ax, ay;
    animate_fn custom_fn;
    void* priv;
    struct sprite_placement* next;  // Doubly-linked list for layers
    struct sprite_placement* prev;
};

struct canvas {
    size_t height, width;
    color_t bg;
    struct sprite_placement* top_layer;
    struct sprite_placement* bottom_layer;
};

// Helper: Increment sprite ref count
static void sprite_ref(struct sprite* s) {
    if (s) s->ref_count++;
}

// Helper: Decrement sprite ref count and free if 0
static bool sprite_unref(struct sprite* s) {
    if (!s) return false;
    s->ref_count--;
    if (s->ref_count == 0) {
        if (s->type == BITMAP && s->bitmap.data) free(s->bitmap.data);
        free(s);
        return true;
    }
    return false;
}

// Helper: Calculate position at time t
static void calc_position(struct sprite_placement* p, float t, ssize_t* x, ssize_t* y) {
    if (p->custom_fn) {
        p->custom_fn(p->priv, x, y, t);
    } else {
        *x = p->x0 + p->vx * t + 0.5 * p->ax * t * t;
        *y = p->y0 + p->vy * t + 0.5 * p->ay * t * t;
    }
}

// Helper: Draw sprite to frame buffer (ARGB32, row-major)
static void draw_sprite(color_t* frame, size_t fheight, size_t fwidth, struct sprite* s, ssize_t sx, ssize_t sy) {
    if (!s) return;
    size_t sw, sh;
    if (s->type == BITMAP) {
        sw = s->bitmap.width;
        sh = s->bitmap.height;
    } else if (s->type == RECTANGLE) {
        sw = s->rect.width;
        sh = s->rect.height;
    } else {  // CIRCLE
        sw = sh = 2 * s->circle.radius + 1;
    }
    for (size_t dy = 0; dy < sh; dy++) {
        for (size_t dx = 0; dx < sw; dx++) {
            ssize_t fx = sx + dx;
            ssize_t fy = sy + dy;
            if (fx < 0 || fy < 0 || fx >= fwidth || fy >= fheight) continue;
            color_t c = 0;
            bool draw = false;
            if (s->type == BITMAP) {
                c = s->bitmap.data[dy * sw + dx];
                draw = ((c >> 24) & 0xFF) != 0;
            } else if (s->type == RECTANGLE) {
                if (s->rect.filled || dx == 0 || dy == 0 || dx == sw - 1 || dy == sh - 1) {
                    c = s->rect.color;
                    draw = true;
                }
            } else {  // CIRCLE
                ssize_t cx = s->circle.radius;
                ssize_t cy = s->circle.radius;
                if ((dx - cx) * (dx - cx) + (dy - cy) * (dy - cy) <= s->circle.radius * s->circle.radius) {
                    c = s->circle.color;
                    draw = true;
                }
            }
            if (draw) {
                frame[fy * fwidth + fx] = (c & 0x00FFFFFF) | 0xFF000000;  // Set alpha to 0xFF
            }
        }
    }
}

struct canvas* animate_create_canvas(size_t height, size_t width, color_t background_color) {
    struct canvas* c = malloc(sizeof(*c));
    if (!c) return NULL;
    c->height = height;
    c->width = width;
    c->bg = background_color;
    c->top_layer = NULL;
    c->bottom_layer = NULL;
    return c;
}

void animate_destroy_canvas(struct canvas* canvas) {
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

struct sprite* animate_create_sprite(const char* file) {
    FILE* fp = fopen(file, "rb");
    if (!fp) return NULL;
    // Simplified BMP header read (assume ARGB32, no full validation)
    fseek(fp, 10, SEEK_SET);
    uint32_t data_offset;
    fread(&data_offset, 4, 1, fp);
    fseek(fp, 18, SEEK_SET);
    int32_t w, h;
    fread(&w, 4, 1, fp);
    fread(&h, 4, 1, fp);
    if (w <= 0 || h <= 0) { fclose(fp); return NULL; }
    size_t width = w, height = h;
    fseek(fp, data_offset, SEEK_SET);
    color_t* data = malloc(width * height * sizeof(color_t));
    if (!data) { fclose(fp); return NULL; }
    // Read rows in reverse (BMP bottom-up)
    for (size_t y = 0; y < height; y++) {
        fread(&data[(height - 1 - y) * width], sizeof(color_t), width, fp);
    }
    fclose(fp);
    struct sprite* s = malloc(sizeof(*s));
    if (!s) { free(data); return NULL; }
    s->type = BITMAP;
    s->bitmap.width = width;
    s->bitmap.height = height;
    s->bitmap.data = data;
    s->ref_count = 0;
    return s;
}

struct sprite* animate_create_rectangle(size_t width, size_t height, color_t c, bool filled) {
    struct sprite* s = malloc(sizeof(*s));
    if (!s) return NULL;
    s->type = RECTANGLE;
    s->rect.width = width;
    s->rect.height = height;
    s->rect.color = c;
    s->rect.filled = filled;
    s->ref_count = 0;
    return s;
}

struct sprite* animate_create_circle(size_t radius, color_t c, bool filled) {
    (void)filled;  // Reserved
    struct sprite* s = malloc(sizeof(*s));
    if (!s) return NULL;
    s->type = CIRCLE;
    s->circle.radius = radius;
    s->circle.color = c;
    s->ref_count = 0;
    return s;
}

bool animate_destroy_sprite(struct sprite* sprite) {
    return sprite_unref(sprite);
}

struct sprite_placement* animate_place_sprite(struct canvas* canvas, struct sprite* sprite, ssize_t x, ssize_t y) {
    if (!canvas || !sprite) return NULL;
    struct sprite_placement* p = malloc(sizeof(*p));
    if (!p) return NULL;
    p->sprite = sprite;
    sprite_ref(sprite);
    p->x0 = x;
    p->y0 = y;
    p->vx = p->vy = p->ax = p->ay = 0;
    p->custom_fn = NULL;
    p->priv = NULL;
    p->next = NULL;
    p->prev = canvas->top_layer;
    if (canvas->top_layer) {
        canvas->top_layer->next = p;
    } else {
        canvas->bottom_layer = p;
    }
    canvas->top_layer = p;
    return p;
}

void animate_placement_up(struct sprite_placement* sprite_placement) {
    if (!sprite_placement || !sprite_placement->next) return;
    struct sprite_placement* next = sprite_placement->next;
    if (sprite_placement->prev) sprite_placement->prev->next = next;
    next->prev = sprite_placement->prev;
    sprite_placement->prev = next;
    sprite_placement->next = next->next;
    if (next->next) next->next->prev = sprite_placement;
    next->next = sprite_placement;
    // Update canvas top if needed
    struct canvas* c = (struct canvas*)((char*)sprite_placement - offsetof(struct canvas, top_layer));  // Hacky, but works
    if (c->top_layer == sprite_placement) c->top_layer = next;
}

void animate_placement_down(struct sprite_placement* sprite_placement) {
    if (!sprite_placement || !sprite_placement->prev) return;
    struct sprite_placement* prev = sprite_placement->prev;
    if (sprite_placement->next) sprite_placement->next->prev = prev;
    prev->next = sprite_placement->next;
    sprite_placement->next = prev;
    sprite_placement->prev = prev->prev;
    if (prev->prev) prev->prev->next = sprite_placement;
    prev->prev = sprite_placement;
    // Update canvas bottom if needed
    struct canvas* c = (struct canvas*)((char*)sprite_placement - offsetof(struct canvas, bottom_layer));  // Hacky
    if (c->bottom_layer == sprite_placement) c->bottom_layer = prev;
}

void animate_placement_top(struct sprite_placement* sprite_placement) {
    if (!sprite_placement) return;
    struct canvas* c = (struct canvas*)((char*)sprite_placement - offsetof(struct canvas, top_layer));  // Hacky
    if (c->top_layer == sprite_placement) return;
    // Remove from list
    if (sprite_placement->prev) sprite_placement->prev->next = sprite_placement->next;
    if (sprite_placement->next) sprite_placement->next->prev = sprite_placement->prev;
    if (c->bottom_layer == sprite_placement) c->bottom_layer = sprite_placement->next;
    // Insert at top
    sprite_placement->prev = c->top_layer;
    sprite_placement->next = NULL;
    if (c->top_layer) c->top_layer->next = sprite_placement;
    c->top_layer = sprite_placement;
}

void animate_placement_bottom(struct sprite_placement* sprite_placement) {
    if (!sprite_placement) return;
    struct canvas* c = (struct canvas*)((char*)sprite_placement - offsetof(struct canvas, bottom_layer));  // Hacky
    if (c->bottom_layer == sprite_placement) return;
    // Remove from list
    if (sprite_placement->prev) sprite_placement->prev->next = sprite_placement->next;
    if (sprite_placement->next) sprite_placement->next->prev = sprite_placement->prev;
    if (c->top_layer == sprite_placement) c->top_layer = sprite_placement->prev;
    // Insert at bottom
    sprite_placement->next = c->bottom_layer;
    sprite_placement->prev = NULL;
    if (c->bottom_layer) c->bottom_layer->prev = sprite_placement;
    c->bottom_layer = sprite_placement;
}

void animate_destroy_placement(struct sprite_placement* sprite_placement) {
    if (!sprite_placement) return;
    struct canvas* c = (struct canvas*)((char*)sprite_placement - offsetof(struct canvas, top_layer));  // Hacky
    // Remove from list
    if

