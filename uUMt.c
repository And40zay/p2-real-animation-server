#include "animate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define OUTPUT_FILE "bitmap_test.dat"

int main(int argc, char** argv) {
    // Load the bitmap sprite
    struct sprite* bmp_sprite = animate_create_sprite("/home/jake/Desktop/P1/p1-scaffold/A.bmp");
    if (bmp_sprite == NULL) {
        fprintf(stderr, "Failed to load bitmap\n");
        return -1;
    }

    // Create a canvas (adjust size as needed for your bitmap)
    struct canvas* canvas = animate_create_canvas(100, 100, animate_color_argb(255, 0, 0, 0));

    // Place the sprite
    struct sprite_placement* placement = animate_place_sprite(canvas, bmp_sprite, 10, 10);

    // Generate frame
    size_t frame_size = animate_frame_size_bytes(canvas);
    void* data = malloc(frame_size);
    animate_generate_frame(canvas, 0, 25, data);

    // Write output
    FILE* fp = fopen(OUTPUT_FILE, "wb");
    if (fp == NULL) {
        perror("Failed to open output file");
        return -1;
    }
    fwrite(data, 1, frame_size, fp);
    fclose(fp);

    printf("Output written to %s\n", OUTPUT_FILE);

    // Cleanup
    free(data);
    animate_destroy_canvas(canvas);
    animate_destroy_sprite(bmp_sprite);

    return 0;
}
