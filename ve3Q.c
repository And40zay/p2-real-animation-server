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

int main(void) {
    const char* bmp_path = "A.bmp";
    struct sprite* bmp = animate_create_sprite(bmp_path);
    if (!bmp) {
        bmp_path = "../p1-scaffold/A.bmp";
        bmp = animate_create_sprite(bmp_path);
    }
    if (!bmp) {
        fprintf(stderr, "Failed to load A.bmp\n");
        return 1;
    }

    size_t canvas_w = 100, canvas_h = 100;
    struct canvas* canvas = animate_create_canvas(canvas_h, canvas_w,
        animate_color_argb(0xFF, 30, 30, 30)); // dark gray background

    animate_place_sprite(canvas, bmp, 10, 10);
    struct sprite* rect = animate_create_rectangle(20, 12, animate_color_argb(0xFF, 255, 255, 0), true);
    animate_place_sprite(canvas, rect, 2, 2);

    size_t frame_sz = animate_frame_size_bytes(canvas);
    color_t* buf = malloc(frame_sz);
    if (!buf) return 1;

    animate_generate_frame(canvas, 0, 25, buf);

    FILE* fd = fopen("frame.dat", "wb");
    if (fd) { fwrite(buf, 1, frame_sz, fd); fclose(fd); }
    write_ppm("frame.ppm", buf, canvas_w, canvas_h);

    free(buf);
    animate_destroy_canvas(canvas);
    animate_destroy_sprite(rect);
    animate_destroy_sprite(bmp);
    return 0;
}
