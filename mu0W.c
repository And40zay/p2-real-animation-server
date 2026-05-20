#include "animate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct sprite {
    size_t width;
    size_t height;
    color_t* pixels;
    int ref_count;
};

struct sprite_placement {
    struct canvas* canvas;
    struct sprite* sprite;
    ssize_t x, y;
    ssize_t vx, vy, ax, ay;
    struct sprite_placement* next;
    struct sprite_placement* prev;
};

struct canvas {
    size_t height;
    size_t width;
    color_t background_color;
    struct sprite_placement* top;
    struct sprite_placement* bottom;
};

struct canvas* animate_create_canvas(size_t height, size_t width, color_t background_color) {
    struct canvas* c = malloc(sizeof(struct canvas));
    c->height = height;
    c->width = width;
    c->background_color = background_color;
    c->top = NULL;
    c->bottom = NULL;
    return c;
}

void animate_destroy_canvas(struct canvas* canvas) {
    struct sprite_placement* p = canvas->bottom;
    while (p) {
        struct sprite_placement* next = p->next;
        p->sprite->ref_count--;
        free(p);
        p = next;
    }
    free(canvas);
}

struct sprite* animate_create_sprite(const char* file) {
    FILE* fp = fopen(file, "rb");
    if (!fp) return NULL;
    
    uint8_t header[54];
    fread(header, 1, 54, fp);
    
    uint32_t offset = *(uint32_t*)(header + 10);
    uint32_t width = *(uint32_t*)(header + 18);
    uint32_t height = *(uint32_t*)(header + 22);
    
    struct sprite* s = malloc(sizeof(struct sprite));
    s->width = width;
    s->height = height;
    s->ref_count = 0;
    s->pixels = malloc(width * height * sizeof(color_t));
    
    fseek(fp, offset, SEEK_SET);
    for (size_t y = 0; y < height; y++) {
        fread(&s->pixels[(height - 1 - y) * width], sizeof(color_t), width, fp);
    }
    fclose(fp);
    return s;
}

struct sprite* animate_create_rectangle(size_t width, size_t height, color_t c, bool filled) {
    struct sprite* s = malloc(sizeof(struct sprite));
    s->width = width;
    s->height = height;
    s->ref_count = 0;
    s->pixels = malloc(width * height * sizeof(color_t));
    
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            if (filled || x == 0 || y == 0 || x == width-1 || y == height-1) {
                s->pixels[y * width + x] = c | 0xFF000000;
            } else {
                s->pixels[y * width + x] = 0;
            }
        }
    }
    return s;
}

struct sprite* animate_create_circle(size_t radius, color_t c, bool filled) {
    (void)filled;
    size_t size = 2 * radius + 1;
    struct sprite* s = malloc(sizeof(struct sprite));
    s->width = size;
    s->height = size;
    s->ref_count = 0;
    s->pixels = malloc(size * size * sizeof(color_t));
    
    for (size_t y = 0; y < size; y++) {
        for (size_t x = 0; x < size; x++) {
            ssize_t dx = x - radius, dy = y - radius;
            if (dx*dx + dy*dy <= (ssize_t)(radius*radius)) {
                s->pixels[y * size + x] = c | 0xFF000000;
            } else {
                s->pixels[y * size + x] = 0;
            }
        }
    }
    return s;
}

bool animate_destroy_sprite(struct sprite* sprite) {
    if (sprite->ref_count > 0) return false;
    free(sprite->pixels);
    free(sprite);
    return true;
}

struct sprite_placement* animate_place_sprite(struct canvas* canvas, struct sprite* sprite, ssize_t x, ssize_t y) {
    struct sprite_placement* p = malloc(sizeof(struct sprite_placement));
    p->canvas = canvas;
    p->sprite = sprite;
    p->x = x; p->y = y;
    p->vx = p->vy = p->ax = p->ay = 0;
    p->next = NULL;
    p->prev = canvas->top;
    
    sprite->ref_count++;
    if (canvas->top) canvas->top->next = p;
    else canvas->bottom = p;
    canvas->top = p;
    return p;
}

void animate_placement_up(struct sprite_placement* p) {
    if (!p || !p->next) return;
    struct sprite_placement* n = p->next;
    struct canvas* c = p->canvas;
    if (p->prev) p->prev->next = n;
    else c->bottom = n;
    n->prev = p->prev;
    p->prev = n;
    p->next = n->next;
    if (n->next) n->next->prev = p;
    else c->top = p;
    n->next = p;
}

void animate_placement_down(struct sprite_placement* p) {
    if (!p || !p->prev) return;
    struct sprite_placement* n = p->prev;
    struct canvas* c = p->canvas;
    if (p->next) p->next->prev = n;
    else c->top = n;
    n->next = p->next;
    p->next = n;
    p->prev = n->prev;
    if (n->prev) n->prev->next = p;
    else c->bottom = p;
    n->prev = p;
}

void animate_placement_top(struct sprite_placement* p) {
    while (p->next) animate_placement_up(p);
}

void animate_placement_bottom(struct sprite_placement* p) {
    while (p->prev) animate_placement_down(p);
}

void animate_destroy_placement(struct sprite_placement* p) {
    struct canvas* c = p->canvas;
    if (p->prev) p->prev->next = p->next;
    else c->bottom = p->next;
    if (p->next) p->next->prev = p->prev;
    else c->top = p->prev;
    p->sprite->ref_count--;
    free(p);
}

void animate_set_animation_params(struct sprite_placement* p, ssize_t vx, ssize_t vy, ssize_t ax, ssize_t ay) {
    p->vx = vx; p->vy = vy; p->ax = ax; p->ay = ay;
}

void animate_set_animation_function(struct sprite_placement* p, animate_fn fn, void* priv) {
    (void)p; (void)fn; (void)priv;
}

size_t animate_frame_size_bytes(struct canvas* canvas) {
    return canvas->height * canvas->width * sizeof(color_t);
}

void animate_generate_frame(const struct canvas* canvas, size_t frame, size_t frame_rate, void* buf) {
    color_t* px = buf;
    size_t total = canvas->height * canvas->width;
    for (size_t i = 0; i < total; i++) px[i] = canvas->background_color | 0xFF000000;
    
    float t = (float)frame / frame_rate;
    for (struct sprite_placement* p = canvas->bottom; p; p = p->next) {
        ssize_t sx = p->x + p->vx * t + 0.5 * p->ax * t * t;
        ssize_t sy = p->y + p->vy * t + 0.5 * p->ay * t * t;
        struct sprite* s = p->sprite;
        for (size_t dy = 0; dy < s->height; dy++) {
            for (size_t dx = 0; dx < s->width; dx++) {
                ssize_t fx = sx + dx, fy = sy + dy;
                if (fx < 0 || fy < 0 || fx >= (ssize_t)canvas->width || fy >= (ssize_t)canvas->height) continue;
                color_t c = s->pixels[dy * s->width + dx];
                if (c >> 24) px[fy * canvas->width + fx] = c | 0xFF000000;
            }
        }
    }
}

