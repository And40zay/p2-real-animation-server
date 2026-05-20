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
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>

/* helper to check for file readability */
static int file_exists_readable(const char *path) {
    if (!path) return 0;
    return access(path, R_OK) == 0;
}

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

static void remove_frames_dir(void) {
    DIR* d = opendir("frames");
    if (!d) return;
    struct dirent* ent;
    while ((ent = readdir(d)) != NULL) {
        const char* name = ent->d_name;
        bool is_reg = false;

#if defined(DT_REG)
        /* If d_type is available and indicates regular file, use it */
        if (ent->d_type == DT_REG) {
            is_reg = true;
        } else if (ent->d_type == DT_UNKNOWN) {
            /* Fall back to stat when type is unknown */
            char path[512];
            snprintf(path, sizeof(path), "frames/%s", name);
            struct stat st;
            if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
                is_reg = true;
            }
        }
#else
        /* DT_REG not available; determine type via stat() */
        {
            char path[512];
            snprintf(path, sizeof(path), "frames/%s", name);
            struct stat st;
            if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
                is_reg = true;
            }
        }
#endif

        if (is_reg) {
            /* remove files named frame_*.ppm */
            if (strncmp(name, "frame_", 6) == 0) {
                char path[512];
                snprintf(path, sizeof(path), "frames/%s", name);
                unlink(path);
            }
        }
    }
    closedir(d);
    rmdir("frames");
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    int rc = 1; /* default to failure; set to 0 on success */
    struct canvas* canvas = NULL;
    struct sprite* s_cross = NULL;
    struct sprite* s_rick = NULL;
    struct sprite_placement* p_cross = NULL;
    struct sprite_placement* p_rick = NULL;
    void* buf = NULL;

    // Canvas size (width x height)
    size_t width = 240, height = 160;
    canvas = animate_create_canvas(height, width, animate_color_argb(0, 20, 20, 30));
    if (!canvas) { fprintf(stderr, "ERROR: Failed to create canvas\n"); goto cleanup; }

    /* check source bitmap files are present and readable */
    if (!file_exists_readable("crosshair.bmp")) {
        fprintf(stderr, "ERROR: crosshair.bmp not found or not readable in %s\n", getcwd(NULL,0));
        goto cleanup;
    }
    if (!file_exists_readable("rick.bmp")) {
        fprintf(stderr, "ERROR: rick.bmp not found or not readable in %s\n", getcwd(NULL,0));
        goto cleanup;
    }

    // Load bitmaps from project root: crosshair.bmp and rick.bmp
    s_cross = animate_create_sprite("crosshair.bmp");
    if (!s_cross) {
        /* fallback: create a small crosshair-like rectangle if bitmap missing */
        fprintf(stderr, "WARNING: crosshair.bmp not found — using generated fallback sprite\n");
        s_cross = animate_create_rectangle(16, 16, animate_color_argb(255, 255, 0, 0), true);
        if (!s_cross) { fprintf(stderr, "ERROR: failed to create fallback cross sprite\n"); goto cleanup; }
    }

    s_rick = animate_create_sprite("rick.bmp");
    if (!s_rick) {
        /* fallback: create a larger rectangle if bitmap missing */
        fprintf(stderr, "WARNING: rick.bmp not found — using generated fallback sprite\n");
        s_rick = animate_create_rectangle(48, 48, animate_color_argb(255, 0, 200, 0), true);
        if (!s_rick) { fprintf(stderr, "ERROR: failed to create fallback rick sprite\n"); goto cleanup; }
    }

    // Place sprites: rick will move horizontally, crosshair static
    p_rick = animate_place_sprite(canvas, s_rick, 0, 60);

    /* use accessors instead of s_cross->width/height (opaque type in header) */
    size_t s_cross_w = animate_sprite_width(s_cross);
    size_t s_cross_h = animate_sprite_height(s_cross);

    p_cross = animate_place_sprite(canvas, s_cross,
                                   (ssize_t)(width/2 - (ssize_t)s_cross_w/2),
                                   (ssize_t)(height/2 - (ssize_t)s_cross_h/2));
    if (!p_rick || !p_cross) { fprintf(stderr, "ERROR: Failed to place sprites (p_rick=%p p_cross=%p)\n", (void*)p_rick, (void*)p_cross); goto cleanup; }

    // Animate rick: ~60 pixels/sec to the right
    animate_set_animation_params(p_rick, 60, 0, 0, 0);

    // Prepare frames directory
    if (mkdir("frames", 0755) != 0 && errno != EEXIST) {
        perror("ERROR: mkdir frames");
        goto cleanup;
    }

    size_t frame_rate = 25;
    size_t frames = 60; // 60 frames (~2.4s)
    size_t frame_bytes = animate_frame_size_bytes(canvas);
    if (frame_bytes == 0) { fprintf(stderr, "ERROR: frame size bytes is zero\n"); goto cleanup; }

    buf = malloc(frame_bytes);
    if (!buf) { perror("ERROR: malloc"); goto cleanup; }

    printf("Generating %zu frames (%zux%zu) into ./frames/\n", frames, width, height);
    for (size_t f = 0; f < frames; f++) {
        animate_generate_frame(canvas, f, frame_rate, buf);
        char out[256];
        snprintf(out, sizeof(out), "frames/frame_%03zu.ppm", f);
        if (write_ppm(out, buf, width, height) != 0) {
            fprintf(stderr, "ERROR: Failed to write %s\n", out);
            goto cleanup;
        }
        if (f % 10 == 0) printf("Wrote %s (frame %zu)\n", out, f);
    }

    /* Interactive keep/delete prompt ("keep" button equivalent) */
    printf("\nFrames written to ./frames/\n");
    printf("Press 'k' then Enter to KEEP frames, 'd' then Enter to DELETE frames: ");
    fflush(stdout);
    int ch = getchar();
    /* consume trailing newline if any */
    while (ch != EOF && ch != '\n') { /* consume until newline */ if (ch == '\r') break; break; }
    if (ch == 'd' || ch == 'D') {
        remove_frames_dir();
        printf("Frames deleted.\n");
    } else {
        printf("Frames kept.\n");
    }

    rc = 0; /* success */

cleanup:
    free(buf);
    if (p_rick) animate_destroy_placement(p_rick);
    if (p_cross) animate_destroy_placement(p_cross);
    if (s_rick) animate_destroy_sprite(s_rick);
    if (s_cross) animate_destroy_sprite(s_cross);
    if (canvas) animate_destroy_canvas(canvas);

    if (rc != 0) fprintf(stderr, "Exiting with code %d\n", rc);
    return rc;
}

