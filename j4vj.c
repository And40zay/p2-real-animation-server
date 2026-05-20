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

#define OUTPUT_FILE "simple.dat"

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    int rc = -1;
    struct canvas* canvas = NULL;
    struct sprite* rect = NULL;
    struct sprite_placement* prect1 = NULL;
    struct sprite_placement* prect2 = NULL;
    void* data = NULL;
    FILE* fp = NULL;

    canvas = animate_create_canvas(10, 10, animate_color_argb(0, 0, 0, 0));
    if (canvas == NULL) {
        fprintf(stderr, "animate_create_canvas failed\n");
        goto cleanup;
    }

    rect = animate_create_rectangle(3, 6, animate_color_argb(0,255,255,0), 1);
    if (rect == NULL) {
        fprintf(stderr, "animate_create_rectangle failed\n");
        goto cleanup;
    }

    prect1 = animate_place_sprite(canvas, rect, 0, 0);
    prect2 = animate_place_sprite(canvas, rect, 2, 1);
    if (prect1 == NULL || prect2 == NULL) {
        fprintf(stderr, "animate_place_sprite failed (prect1=%p, prect2=%p)\n", (void*)prect1, (void*)prect2);
        goto cleanup;
    }

    size_t frame_size_bytes = animate_frame_size_bytes(canvas);
    printf("debug: frame_size_bytes=%zu, canvas=%p, rect=%p\n", frame_size_bytes, (void*)canvas, (void*)rect);

    data = malloc(frame_size_bytes);
    if (data == NULL) {
        perror("malloc failed");
        goto cleanup;
    }

    animate_generate_frame(canvas, 1, 25, data);

    fp = fopen(OUTPUT_FILE, "wb");
    if (fp == NULL) {
        perror("Failed to open target file");
        goto cleanup;
    }

    size_t bytes_written = fwrite(data, 1, frame_size_bytes, fp);
    if (bytes_written != frame_size_bytes) {
        fprintf(stderr, "Failed to write buffer (%zu/%zu): %s\n",
                bytes_written, frame_size_bytes, strerror(errno));
        goto cleanup;
    }

    rc = 0;

cleanup:
    if (fp != NULL) fclose(fp);
    free(data);
    if (canvas != NULL) animate_destroy_canvas(canvas);
    if (rect != NULL) animate_destroy_sprite(rect);
    return rc;
}

