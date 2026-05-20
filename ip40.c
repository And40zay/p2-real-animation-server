/**
 * A simple test file to help you get started
 */

#include "animate.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

int write_ppm(const char* path, color_t* data, size_t w, size_t h) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return -1;
    if (fprintf(fp, "P6\n%zu %zu\n255\n", w, h) < 0) { fclose(fp); return -1; }
    for (size_t i = 0; i < w * h; i++) {
        color_t c = data[i];
        unsigned char rgb[3];
        rgb[0] = (c >> 16) & 0xFF;
        rgb[1] = (c >> 8) & 0xFF;
        rgb[2] = (c >> 0) & 0xFF;
        if (fwrite(rgb, 1, 3, fp) != 3) { fclose(fp); return -1; }
    }
    fclose(fp);
    return 0;
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    int rc = -1;
    struct canvas* canvas = NULL;
    struct sprite* s_cross = NULL;
    struct sprite* s_rick = NULL;
    struct sprite_placement* p_cross = NULL;
    struct sprite_placement* p_rick = NULL;
    void* buf = NULL;

    // Canvas size (width x height)
    size_t width = 240, height = 160;
    canvas = animate_create_canvas(height, width, animate_color_argb(0, 20, 20, 30));
    if (!canvas) { fprintf(stderr, "Failed to create canvas\n"); goto cleanup; }

    // Load bitmaps from project root: crosshair.bmp and rick.bmp
    s_cross = animate_create_sprite("crosshair.bmp");
    if (!s_cross) { fprintf(stderr, "Failed to load crosshair.bmp\n"); goto cleanup; }
    s_rick = animate_create_sprite("rick.bmp");
    if (!s_rick) { fprintf(stderr, "Failed to load rick.bmp\n"); goto cleanup; }

    // Place sprites: rick will move horizontally, crosshair static
    p_rick = animate_place_sprite(canvas, s_rick, 0, 60);

    /* use accessors instead of s_cross->width/height (opaque type in header) */
    size_t s_cross_w = animate_sprite_width(s_cross);
    size_t s_cross_h = animate_sprite_height(s_cross);

    p_cross = animate_place_sprite(canvas, s_cross,
                                   (ssize_t)(width/2 - (ssize_t)s_cross_w/2),
                                   (ssize_t)(height/2 - (ssize_t)s_cross_h/2));
    if (!p_rick || !p_cross) { fprintf(stderr, "Failed to place sprites\n"); goto cleanup; }

    // Animate rick: ~60 pixels/sec to the right
    animate_set_animation_params(p_rick, 60, 0, 0, 0);

    // Prepare frames directory
    mkdir("frames", 0755);

    size_t frame_rate = 25;
    size_t frames = 60; // 60 frames (~2.4s)
    size_t frame_bytes = animate_frame_size_bytes(canvas);
    buf = malloc(frame_bytes);
    if (!buf) { perror("malloc"); goto cleanup; }

    printf("Generating %zu frames (%zux%zu) into ./frames/\n", frames, width, height);
    for (size_t f = 0; f < frames; f++) {
        animate_generate_frame(canvas, f, frame_rate, buf);
        char out[256];
        snprintf(out, sizeof(out), "frames/frame_%03zu.ppm", f);
        if (write_ppm(out, buf, width, height) != 0) {
            fprintf(stderr, "Failed to write %s\n", out);
            goto cleanup;
        }
        // Helpful debug print for stepping in gdb
        if (f % 10 == 0) printf("Wrote %s (frame %zu)\n", out, f);
    }

    rc = 0;

cleanup:
    free(buf);
    if (p_rick) animate_destroy_placement(p_rick);
    if (p_cross) animate_destroy_placement(p_cross);
    if (s_rick) animate_destroy_sprite(s_rick);
    if (s_cross) animate_destroy_sprite(s_cross);
    if (canvas) animate_destroy_canvas(canvas);
    return rc;
}

