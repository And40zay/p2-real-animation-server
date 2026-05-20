#include "animate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define OUTPUT_FILE "frame.dat"
#define OUTPUT_PPM "frame.ppm"

static void write_ppm(const char* path, const color_t* buf, size_t w, size_t h) {
    FILE* f = fopen(path, "w");
    if (!f) return;
    fprintf(f, "P3\n%zu %zu\n255\n", w, h);
    for (size_t i = 0; i < w * h; i++) {
        color_t c = buf[i];
        unsigned r = (c >> 16) & 0xFF;
        unsigned g = (c >> 8) & 0xFF;
        unsigned b = (c >> 0) & 0xFF;
        fprintf(f, "%u %u %u\n", r, g, b);
    }
    fclose(f);
}

int main(int argc, char** argv) {
    // Load the bitmap sprite
    struct sprite* bmp_sprite = animate_create_sprite("A.bmp");
    if (bmp_sprite == NULL) {
        fprintf(stderr, "Failed to load A.bmp\n");
        return -1;
    }

    size_t canvas_w = bmp_sprite->width + 20;
    size_t canvas_h = bmp_sprite->height + 20;
    // Create a canvas (adjust size as needed for your bitmap)
    struct canvas* canvas = animate_create_canvas(canvas_h, canvas_w,
        animate_color_argb(0xFF, 30, 30, 30)); // dark gray background

    // Place the sprite
    struct sprite_placement* placement = animate_place_sprite(canvas, bmp_sprite, 10, 10);

    // Generate frame
    size_t frame_size = animate_frame_size_bytes(canvas);
    color_t* buf = malloc(frame_size);
    if (!buf) return 1;

    animate_generate_frame(canvas, 0, 25, buf);

    FILE* fd = fopen(OUTPUT_FILE, "wb");
    if (fd) { fwrite(buf, 1, frame_size, fd); fclose(fd); }
    write_ppm(OUTPUT_PPM, buf, canvas_w, canvas_h);

    printf("Output written to %s and %s\n", OUTPUT_FILE, OUTPUT_PPM);

    // Cleanup
    free(buf);
    animate_destroy_canvas(canvas);
    animate_destroy_sprite(bmp_sprite);

    return 0;
}
