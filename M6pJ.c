#include "animate.h"

#include <stdio.h>

struct sprite {
    // TODO
    size_t width;
    size_t height;
    color_t* pixels;
    bool filled;
    unsigned refcount;
};

struct sprite_placement {
    // TODO
    struct sprite* sprite;
    struct canvas* canvas;
    ssize_t x;
    ssize_t y;
    ssize_t vx, vy;
    ssize_t ax, ay;
    animate_fn custom_fn;
    struct sprite_placement *prev;
    struct sprite_placement *next;

};

struct canvas {
    // TODO
    size_t height;
    size_t width;
    color_t background_color;
    struct spite_placement *bottom;
    struct spite_placement *top;

};


/* This is the header structure that we can expect to find at position 0 in a bitmap file. */
struct bitmap_header {
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
struct bitmapv5_header {
                               // Offset     Size Description
    uint32_t bV5Size         ; // 0x00          4 Size of this header (124 bytes)
    uint32_t bV5Width        ; // 0x04          4 Width of the bitmap in pixels
    uint32_t bV5Height       ; // 0x08          4 Height of the bitmap in pixels
    uint16_t bV5Planes       ; // 0x0C          2 Number of planes (must be 1)
    uint16_t bV5BitCount     ; // 0x0E          2 Bits per pixel (e.g., 32)
    uint32_t bV5Compression  ; // 0x10          4 BI_RGB (0), BI_BITFIELDS (3)
    uint32_t bV5SizeImage    ; // 0x14          4 Size of image data (0 if uncompressed)
    uint32_t bV5XPelsPerMeter; // 0x18          4 Horizontal pixels per meter
    uint32_t bV5YPelsPerMebitmapvter; // 0x1C          4 Vertical pixels per meter
    uint32_t bV5ClrUsed      ; // 0x20          4 Number of color indices used
    uint32_t bV5ClrImportant ; // 0x24          4 Number of important colors
    uint32_t bV5RedMask      ; // 0x28          4 Color mask for red component
    uint32_t bV5GreenMask    ; // 0x2C          4 Color mask for green component
    uint32_t bV5BlueMask     ; // 0x30          4 Color mask OUTPUT_FILEfor blue component
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
};__attribute__((packed));
static void remove_placement_from_list(struct spite_placement *p)
{
    if(!p || !p->canvas)
    {
        return;
    }
    struct canvas *c = p->canvas;

    if(p->prev)
    {
        p->prev->next = p->next;
        
    }
    else
    {
        c->bottom = p->next;
    }
    if(p->next)
    {
        p->next->prev = p->prev;
    }
    else
    {
        c->top = p->prev;
    }
    p->prev = p->next = NULL;
    p->canvas = NULL;
}


struct canvas* animate_create_canvas(size_t height, size_t width, color_t background_color){
    // TODO
    struct canvas *c = malloc(sizeof(struct canvas));
    if(!c)
    {
        return NULL;
    }
    c->height = height;
    c->width = width;
    c->background_color = background_color;
    c->bottom = c->top = NULL;
    return c;
}

struct sprite* animate_create_sprite(const char* file) {
    // TODO
    FILE *fp = fopen(file, "wb");
    if(fp == NULL)
    {
        perror("Failed to open the file\n");
        return NULL;
    }
    return NULL;
}

struct sprite* animate_create_circle(size_t radius, color_t c, bool filled) {
    // TODO
    (void)filled;

    size_t diameter = 2 * radius +1;
    struct sprite *s = malloc(sizeof(struct sprite));
    if(!s)
    {
        return NULL;
    }
    s->width = diameter;
    s->height = diameter;
    s->filled = true;
    s->refcount = 0;

    s->pixels = malloc(diameter * diameter * sizeof(color_t));
    if(!s-> pixels)
    {
        free(s);
        return NULL;
    }
    for(size_t y = 0; y< height; y++)
    {
        for(size_t x = 0; x< width; x++)
    }
}

struct sprite* animate_create_rectangle(size_t width, size_t height,
                                        color_t c, bool filled){
    // TODO
    struct sprite *s = malloc(sizeof(struct sprite));
    if(!s)
    {
        return NULL;
    }
    s->width = width;
    s->height = height;
    s->filled = filled;
    s->refcount = 0;

    s->pixels = malloc(width * height * sizeof(color_t));
    if(!s-> pixels)
    {
        free(s);
        return NULL;
    }
    for(size_t y = 0; y<height; y++)
    {
        for(size_t x = 0; x<width; x++)
        {
            if(filled)
            {
                s->pixels[y * width + x] = c;
            }
            else
            {
                if(x == 0 || x== width-1 || y == 0 || y == height -1)
                {
                    s->pixels[y * width + x] = c;
                }
                else
                {
                    s->pixels[y * width + x] = 0;
                }
            }
        }
    }
    return s;
}

bool animate_destroy_sprite(struct sprite* sprite) {
    // TODO
    if(!sprite)
    {
        return 1;
    }
    if(sprite->refcount >0)
    {
        return 0;
    }
    free(sprite->pixels);
    free(sprite);
    return 1;
}

struct sprite_placement* animate_place_sprite(struct canvas* canvas,
                                              struct sprite* sprite,
                                              ssize_t x, ssize_t y) {
    // TODO
    return NULL;
}

void animate_placement_up(struct sprite_placement* sprite_placement){
    // TODO COMP9017
}

void animate_placement_down(struct sprite_placement* sprite_placement){
    // TODO COMP9017
}

void animate_placement_top(struct sprite_placement* sprite_placement){
    // TODO
    
}

void animate_placement_bottom(struct sprite_placement* sprite_placement){
    // TODO
}

void animate_destroy_placement(struct sprite_placement* sprite_placement){
    // TODO
}

void animate_set_animation_params(struct sprite_placement* sprite_placement,
                                  ssize_t vx, ssize_t vy,
                                  ssize_t ax, ssize_t ay){
    // TODO
}

void animate_destroy_canvas(struct canvas* canvas){
    // TODO
}

size_t animate_frame_size_bytes(struct canvas* canvas){
    // TODO
    return 0;
}

void animate_generate_frame(const struct canvas* canvas, size_t frame,
                            size_t frame_rate, void* buf) {
    // TODO
}

// Optional extension
void animate_set_animation_function(struct sprite_placement* sprite_placement,
                                    animate_fn, void* priv) {
}

