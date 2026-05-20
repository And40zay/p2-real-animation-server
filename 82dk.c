#include "animate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct sprite {
    size_t width;
    size_t height;
    color_t* pixels; // Array of pixels for the sprite
    bool filled;
    int ref_count;   // Track how many placements use this sprite
};

struct sprite_placement {
    struct canvas* canvas;
    struct sprite* sprite;
    ssize_t x;
    ssize_t y;
    ssize_t vx; // velocity in x direction
    ssize_t vy; // velocity in y direction
    ssize_t ax; // acceleration in x direction
    ssize_t ay; // acceleration in y direction
    struct sprite_placement* next;  // Layer above (toward top)
    struct sprite_placement* prev;  // Layer below (toward bottom)
};

struct canvas {
    size_t height;
    size_t width;
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


struct canvas* animate_create_canvas(size_t height, size_t width, color_t background_color)
{
    struct canvas* canvas = malloc(sizeof(struct canvas));
    canvas->height = height;
    canvas->width = width;
    canvas->background_color = background_color;
    canvas->top_layer = NULL;
    canvas->bottom_layer = NULL;
    return canvas;
}

struct sprite* animate_create_sprite(const char* file) 
{
    FILE *fp = fopen(file, "rb");
    if (fp == NULL) {
        return NULL;
    }
    struct bitmap_header bmp_header;
    fread(&bmp_header, sizeof(struct bitmap_header), 1, fp);
    if (bmp_header.magic[0] != 'B' || bmp_header.magic[1] != 'M') {
        fclose(fp);
        return NULL;
    }
    struct bitmap5_header bmp5;
    fread(&bmp5, sizeof(struct bitmap5_header), 1, fp);
    
    size_t width = bmp5.bV5Width;
    size_t height = bmp5.bV5Height;
    
    struct sprite* sprite = malloc(sizeof(struct sprite));
    sprite->width = width;
    sprite->height = height;
    sprite->filled = true;
    sprite->ref_count = 0;
    sprite->pixels = malloc(width * height * sizeof(color_t));
    
    fseek(fp, bmp_header.pixel_offset, SEEK_SET);
    // Read rows in reverse (BMP is bottom-up)
    for (size_t y = 0; y < height; y++) {
        fread(&sprite->pixels[(height - 1 - y) * width], sizeof(color_t), width, fp);
    }
    fclose(fp);
    return sprite;
}

struct sprite* animate_create_circle(size_t radius, color_t c, bool filled) 
{
    size_t diameter = 2 * radius + 1;
    struct sprite* sprite = malloc(sizeof(struct sprite));
    sprite->width = diameter;
    sprite->height = diameter;
    sprite->filled = filled;
    sprite->ref_count = 0;
    sprite->pixels = malloc(diameter * diameter * sizeof(color_t));
    
    for (size_t y = 0; y < diameter; y++) {
        for (size_t x = 0; x < diameter; x++) {
            ssize_t dx = x - radius;
            ssize_t dy = y - radius;
            if (dx * dx + dy * dy <= (ssize_t)(radius * radius)) {
                sprite->pixels[y * diameter + x] = c | 0xFF000000;
            } else {
                sprite->pixels[y * diameter + x] = 0; // Transparent
            }
        }
    }
    return sprite;
}

struct sprite* animate_create_rectangle(size_t width, size_t height, color_t c, bool filled)
{
    struct sprite* sprite = malloc(sizeof(struct sprite));
    sprite->width = width;
    sprite->height = height;
    sprite->filled = filled;
    sprite->ref_count = 0;
    sprite->pixels = malloc(width * height * sizeof(color_t));
    
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            if (filled || x == 0 || y == 0 || x == width - 1 || y == height - 1) {
                sprite->pixels[y * width + x] = c | 0xFF000000;
            } else {
                sprite->pixels[y * width + x] = 0; // Transparent interior
            }
        }
    }
    return sprite;
}

bool animate_destroy_sprite(struct sprite* sprite) 
{
    if (sprite->ref_count > 0) {
        return false; // Still in use
    }
    free(sprite->pixels);
    free(sprite);
    return true;
}

struct sprite_placement* animate_place_sprite(struct canvas* canvas, struct sprite* sprite, ssize_t x, ssize_t y) 
{
    struct sprite_placement* placement = malloc(sizeof(struct sprite_placement));
    placement->canvas = canvas;
    placement->sprite = sprite;
    placement->x = x;
    placement->y = y;
    placement->vx = 0;
    placement->vy = 0;
    placement->ax = 0;
    placement->ay = 0;
    placement->next = NULL;
    placement->prev = canvas->top_layer;
    
    sprite->ref_count++;
    
    if (canvas->top_layer) {
        canvas->top_layer->next = placement;
    } else {
        canvas->bottom_layer = placement;
    }
    canvas->top_layer = placement;
    
    return placement;
}

void animate_placement_up(struct sprite_placement* sprite_placement)
{
    if (!sprite_placement || !sprite_placement->next) return;
    struct sprite_placement* above = sprite_placement->next;
    sprite_placement->next = above->next;
    if (above->next) above->next->prev = sprite_placement;
    above->prev = sprite_placement->prev;
    if (sprite_placement->prev) sprite_placement->prev->next = above;
    else sprite_placement->canvas->bottom_layer = above;
    sprite_placement->prev = above;
    above->next = sprite_placement;
}

void animate_placement_down(struct sprite_placement* sprite_placement)
{
    if (!sprite_placement || !sprite_placement->prev) return;
    struct sprite_placement* below = sprite_placement->prev;
    sprite_placement->prev = below->prev;
    if (below->prev) below->prev->next = sprite_placement;
    below->next = sprite_placement->next;
    if (sprite_placement->next) sprite_placement->next->prev = below;
    else sprite_placement->canvas->top_layer = below;
    below->prev = sprite_placement;
    sprite_placement->next = below;
}

void animate_placement_top(struct sprite_placement* sprite_placement)
{
    struct canvas* c = sprite_placement->canvas;
    if (c->top_layer == sprite_placement) return;
    
    // Remove from current position
    if (sprite_placement->prev) sprite_placement->prev->next = sprite_placement->next;
    if (sprite_placement->next) sprite_placement->next->prev = sprite_placement->prev;
    if (c->bottom_layer == sprite_placement) c->bottom_layer = sprite_placement->next;
    
    // Insert at top
    sprite_placement->prev = c->top_layer;
    sprite_placement->next = NULL;
    if (c->top_layer) c->top_layer->next = sprite_placement;
    c->top_layer = sprite_placement;
}

void animate_placement_bottom(struct sprite_placement* sprite_placement)
{
    struct canvas* c = sprite_placement->canvas;
    if (c->bottom_layer == sprite_placement) return;
    
    // Remove from current position
    if (sprite_placement->prev) sprite_placement->prev->next = sprite_placement->next;
    if (sprite_placement->next) sprite_placement->next->prev = sprite_placement->prev;
    if (c->top_layer == sprite_placement) c->top_layer = sprite_placement->prev;
    
    // Insert at bottom
    sprite_placement->next = c->bottom_layer;
    sprite_placement->prev = NULL;
    if (c->bottom_layer) c->bottom_layer->prev = sprite_placement;
    c->bottom_layer = sprite_placement;
}

void animate_destroy_placement(struct sprite_placement* sprite_placement)
{
    struct canvas* c = sprite_placement->canvas;
    
    if (sprite_placement->prev) sprite_placement->prev->next = sprite_placement->next;
    if (sprite_placement->next) sprite_placement->next->prev = sprite_placement->prev;
    if (c->top_layer == sprite_placement) c->top_layer = sprite_placement->prev;
    if (c->bottom_layer == sprite_placement) c->bottom_layer = sprite_placement->next;
    
    sprite_placement->sprite->ref_count--;
    free(sprite_placement);
}

void animate_set_animation_params(struct sprite_placement* sprite_placement, ssize_t vx, ssize_t vy, ssize_t ax, ssize_t ay)
{
    sprite_placement->vx = vx;
    sprite_placement->vy = vy;
    sprite_placement->ax = ax;
    sprite_placement->ay = ay;
}

void animate_destroy_canvas(struct canvas* canvas)
{
    struct sprite_placement* p = canvas->bottom_layer;
    while (p) {
        struct sprite_placement* next = p->next;
        p->sprite->ref_count--;
        free(p);
        p = next;
    }
    free(canvas);
}

size_t animate_frame_size_bytes(struct canvas* canvas)
{
    return canvas->height * canvas->width * sizeof(color_t);
}

void animate_generate_frame(const struct canvas* canvas, size_t frame, size_t frame_rate, void* buf) 
{
    color_t* pixels = (color_t*)buf;
    size_t total = canvas->height * canvas->width;
    
    // Fill with background (alpha = 0xFF)
    for (size_t i = 0; i < total; i++) {
        pixels[i] = (canvas->background_color & 0x00FFFFFF) | 0xFF000000;
    }
    
    float t = (float)frame / frame_rate;
    
    // Draw sprites from bottom to top
    struct sprite_placement* p = canvas->bottom_layer;
    while (p) {
        ssize_t px = p->x + p->vx * t + 0.5 * p->ax * t * t;
        ssize_t py = p->y + p->vy * t + 0.5 * p->ay * t * t;
        
        struct sprite* s = p->sprite;
        for (size_t sy = 0; sy < s->height; sy++) {
            for (size_t sx = 0; sx < s->width; sx++) {
                ssize_t fx = px + sx;
                ssize_t fy = py + sy;
                if (fx < 0 || fy < 0 || fx >= (ssize_t)canvas->width || fy >= (ssize_t)canvas->height) continue;
                
                color_t c = s->pixels[sy * s->width + sx];
                if ((c >> 24) & 0xFF) {  // Non-zero alpha = opaque
                    pixels[fy * canvas->width + fx] = (c & 0x00FFFFFF) | 0xFF000000;
                }
            }
        }
        p = p->next;
    }
}

// Optional extension
void animate_set_animation_function(struct sprite_placement* sprite_placement, animate_fn fn, void* priv) 
{
    (void)sprite_placement;
    (void)fn;
    (void)priv;
}

