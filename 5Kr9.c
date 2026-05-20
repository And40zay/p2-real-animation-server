#include "animate.h"
#include <limits.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct bitmap_header {
    uint8_t magic[2];
    uint32_t file_size;
    uint32_t reserved;
    uint32_t pixel_offset;
};

struct bitmap5_header {
    uint32_t size;
    int32_t  bV5Width;
    int32_t  bV5Height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    int32_t  x_pixels_per_meter;
    int32_t  y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
};

enum sprite_type { SPRITE_BITMAP, SPRITE_RECT, SPRITE_CIRCLE };

struct sprite {
    enum sprite_type type;
    size_t width, height;
    color_t* pixels;   // ARGB32
    int ref_count;
};

struct canvas {
    size_t height, width;
    color_t background_color;
    struct sprite_placement* top_layer;
    struct sprite_placement* bottom_layer;
};

struct sprite_placement {
    struct canvas* canvas;
    struct sprite* sprite;
    ssize_t x, y;
    ssize_t vx, vy, ax, ay;
    animate_fn custom_fn;
    void* priv;
    struct sprite_placement* next; // above
    struct sprite_placement* prev; // below
};

// Helpers -----------------------------------------------------

static void sprite_ref(struct sprite* s) { if (s) s->ref_count++; }

static bool sprite_unref(struct sprite* s) {
    if (!s) return false;
    if (--s->ref_count == 0) { free(s->pixels); free(s); return true; }
    return false;
}

// Loads BMP (ARGB32), reversing rows
static struct sprite* load_bmp(const char* file) {
    FILE* fp = fopen(file, "rb");
    if (!fp) return NULL;

    struct bitmap_header bmp_header;
    if (fread(&bmp_header, sizeof(bmp_header), 1, fp) != 1 ||
        bmp_header.magic[0] != 'B' || bmp_header.magic[1] != 'M') {
        fclose(fp);
        return NULL;
    }

    struct bitmap5_header b5;
    if (fread(&b5, sizeof(b5), 1, fp) != 1) { fclose(fp); return NULL; }
    if (b5.size < 40) { fclose(fp); return NULL; }
    if (b5.bits_per_pixel != 32) { fclose(fp); return NULL; }
    if (!(b5.compression == 0 || b5.compression == 3)) { fclose(fp); return NULL; }

    if (b5.bV5Width <= 0 || b5.bV5Height == 0) { fclose(fp); return NULL; }
    bool top_down = b5.bV5Height < 0;
    size_t w = (size_t)b5.bV5Width;
    size_t h = (size_t)(top_down ? -(int64_t)b5.bV5Height : b5.bV5Height);

    size_t max_pixels = 0;
    if (bmp_header.file_size > bmp_header.pixel_offset)
        max_pixels = (bmp_header.file_size - bmp_header.pixel_offset) / sizeof(color_t);
    if (w == 0 || h == 0 || w > SIZE_MAX / sizeof(color_t) / h || (max_pixels && w > 0 && h > 0 && w * h > max_pixels)) {
        fclose(fp);
        return NULL;
    }

    size_t expected_bytes = w * h * sizeof(color_t);
    if (b5.image_size && b5.image_size < expected_bytes) { fclose(fp); return NULL; }

    struct sprite* s = malloc(sizeof(*s));
    if (!s) { fclose(fp); return NULL; }
    s->type = SPRITE_BITMAP;
    s->width = w;
    s->height = h;
    s->ref_count = 1;
    s->pixels = malloc(expected_bytes);
    if (!s->pixels) { free(s); fclose(fp); return NULL; }

    if (fseek(fp, bmp_header.pixel_offset, SEEK_SET) != 0) {
        free(s->pixels); free(s); fclose(fp); return NULL;
    }

    for (size_t y = 0; y < h; y++) {
        size_t dest_y = top_down ? y : (h - 1 - y);
        if (fread(&s->pixels[dest_y * w], sizeof(color_t), w, fp) != w) {
            free(s->pixels); free(s); fclose(fp); return NULL;
        }
    }
    fclose(fp);
    return s;
}

// Shapes as pixel sprites
static struct sprite* make_rect(size_t w, size_t h, color_t c, bool filled) {
    if (w == 0 || h == 0 || w > SIZE_MAX / sizeof(color_t) / h) return NULL;
    struct sprite* s = malloc(sizeof(*s));
    if (!s) return NULL;
    s->type = SPRITE_RECT;
    s->width = w;
    s->height = h;
    s->ref_count = 1;
    s->pixels = malloc(w * h * sizeof(color_t));
    if (!s->pixels) { free(s); return NULL; }

    for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++) {
            bool border = (x == 0 || y == 0 || x == w - 1 || y == h - 1);
            bool draw = filled || border;
            s->pixels[y * w + x] = draw ? ((c & 0x00FFFFFF) | 0xFF000000) : 0;
        }
    }
    return s;
}

static struct sprite* make_circle(size_t r, color_t c) {
    size_t d = 2 * r + 1;
    if (d == 0 || d > SIZE_MAX / sizeof(color_t) / d) return NULL;
    struct sprite* s = malloc(sizeof(*s));
    if (!s) return NULL;
    s->type = SPRITE_CIRCLE;
    s->width = d;
    s->height = d;
    s->ref_count = 1;
    s->pixels = malloc(d * d * sizeof(color_t));
    if (!s->pixels) { free(s); return NULL; }

    for (size_t y = 0; y < d; y++) {
        for (size_t x = 0; x < d; x++) {
            ssize_t dx = (ssize_t)x - (ssize_t)r;
            ssize_t dy = (ssize_t)y - (ssize_t)r;
            bool inside = dx * dx + dy * dy <= (ssize_t)(r * r);
            s->pixels[y * d + x] = inside ? ((c & 0x00FFFFFF) | 0xFF000000) : 0;
        }
    }
    return s;
}

// Core API ----------------------------------------------------

struct canvas* animate_create_canvas(size_t height, size_t width, color_t background_color)
{
    struct canvas* canvas = malloc(sizeof(struct canvas));
    if (!canvas) return NULL;
    canvas->height = height;
    canvas->width = width;
    canvas->background_color = background_color;
    canvas->top_layer = NULL;
    canvas->bottom_layer = NULL;
    return canvas;
}

void animate_destroy_canvas(struct canvas* canvas)
{
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

struct sprite* animate_create_sprite(const char* file) 
{
    struct sprite* s = load_bmp(file);
    if (s) s->ref_count = 1; // ensure user owns one ref
    return s;
}

struct sprite* animate_create_circle(size_t radius, color_t c, bool filled) 
{
    (void)filled; // reserved
    struct sprite* s = make_circle(radius, c);
    if (s) s->ref_count = 1;
    return s;
}

struct sprite* animate_create_rectangle(size_t width, size_t height, color_t c, bool filled)
{
    struct sprite* s = make_rect(width, height, c, filled);
    if (s) s->ref_count = 1;
    return s;
}

bool animate_destroy_sprite(struct sprite* sprite) 
{
    return sprite_unref(sprite);
}Program return code 42 (ASAN failure). Log appended.
Rerun your program through a sanitizer such as ASAN or valgrind.
BEGIN: Simple rectangle sanity; Outline
BEGIN: attempting to load functions load and save from shared library
END: loading successful
BEGIN: create_canvas w=10 h=10 c0x=00000000
END: create_canvas
BEGIN: create_rectangle w=5 h=3 c=0xff112233 filled:=False
END: create_rectangle
BEGIN: place_sprite x=1 y=2
END: place_sprite
BEGIN: generate frame f=0 fr=25
END: generate frame
BEGIN: check frame
END: check frame
BEGIN: destroy_placement
END: destroy_placement
BEGIN: destroy_canvas
END: destroy_canvas
BEGIN: destroy_sprite
=================================================================
==65==ERROR: AddressSanitizer: heap-use-after-free on address 0x7b20936224b0 at pc 0x7ee0937172c3 bp 0x7ffd6b2f2840 sp 0x7ffd6b2f2830
READ of size 4 at 0x7b20936224b0 thread T0
    #0 0x7ee0937172c2 in sprite_unref /home/animate.c:61
    #1 0x7ee093718137 in animate_destroy_sprite /home/animate.c:195
    #2 0x7ee0937208b3 in wrapped_destroy_sprite staff/animate_wrap.c:161
    #3 0x7ee095418ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #4 0x7ee09541576a  (/usr/lib/libffi.so.8+0x476a) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #5 0x7ee09541806d in ffi_call (/usr/lib/libffi.so.8+0x706d) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7ee0936d4c4f  (/usr/lib/python3.14/lib-dynload/_ctypes.cpython-314-x86_64-linux-gnu.so+0x13c4f) (BuildId: 4ecdae4bdfcdb64e8aebbddce9d8a5786a091a5d)
    #7 0x7ee0936c6937  (/usr/lib/python3.14/lib-dynload/_ctypes.cpython-314-x86_64-linux-gnu.so+0x5937) (BuildId: 4ecdae4bdfcdb64e8aebbddce9d8a5786a091a5d)
    #8 0x7ee094f7867b in _PyObject_MakeTpCall (/usr/lib/libpython3.14.so.1.0+0x17867b) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #9 0x7ee094f91151 in _PyEval_EvalFrameDefault (/usr/lib/libpython3.14.so.1.0+0x191151) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #10 0x7ee094f8b804  (/usr/lib/libpython3.14.so.1.0+0x18b804) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #11 0x7ee095076d2d in PyEval_EvalCode (/usr/lib/libpython3.14.so.1.0+0x276d2d) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #12 0x7ee0950b70d0  (/usr/lib/libpython3.14.so.1.0+0x2b70d0) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #13 0x7ee0950b624a  (/usr/lib/libpython3.14.so.1.0+0x2b624a) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #14 0x7ee0950b5c06  (/usr/lib/libpython3.14.so.1.0+0x2b5c06) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #15 0x7ee0950b59e6  (/usr/lib/libpython3.14.so.1.0+0x2b59e6) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #16 0x7ee09506c081 in Py_RunMain (/usr/lib/libpython3.14.so.1.0+0x26c081) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #17 0x7ee09506562a in Py_BytesMain (/usr/lib/libpython3.14.so.1.0+0x26562a) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #18 0x7ee094a27634 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
    #19 0x7ee094a276e8 in __libc_start_main_impl ../csu/libc-start.c:360
    #20 0x5eddb8e76044 in _start (/usr/bin/python3.14+0x1044) (BuildId: ddf03afb83452757f6df249b7608c7df7b476e25)
0x7b20936224b0 is located 32 bytes inside of 40-byte region [0x7b2093622490,0x7b20936224b8)
freed by thread T0 here:
    #0 0x7ee09571f79d in free /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:51
    #1 0x7ee095d72cad in free (staff/override_malloc_free.so+0x6cad) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x7ee09371731f in sprite_unref /home/animate.c:61
    #3 0x7ee093718d05 in animate_destroy_placement /home/animate.c:282
    #4 0x7ee093720a35 in wrapped_destroy_placement staff/animate_wrap.c:199
    #5 0x7ee095418ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7ffd6b2f2a5f  ([stack]+0x1fa5f)
previously allocated by thread T0 here:
    #0 0x7ee095720cb5 in malloc /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:67
    #1 0x7ee095d72b85 in malloc (staff/override_malloc_free.so+0x6b85) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x7ee093717967 in make_rect /home/animate.c:109
    #3 0x7ee09371811d in animate_create_rectangle /home/animate.c:190
    #4 0x7ee09372081f in wrapped_create_rectangle staff/animate_wrap.c:147
    #5 0x7ee095418ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7ffd6b2f29cf  ([stack]+0x1f9cf)
SUMMARY: AddressSanitizer: heap-use-after-free /home/animate.c:61 in sprite_unref
Shadow bytes around the buggy address:
  0x7b2093622200: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x7b2093622280: fa fa 00 00 00 00 00 fa fa fa 00 00 00 00 00 fa
  0x7b2093622300: fa fa 00 00 00 00 00 fa fa fa 00 00 00 00 00 fa
  0x7b2093622380: fa fa 00 00 00 00 00 02 fa fa 00 00 00 00 05 fa
  0x7b2093622400: fa fa fd fd fd fd fd fa fa fa fd fd fd fd fd fd
=>0x7b2093622480: fa fa fd fd fd fd[fd]fa fa fa fd fd fd fd fd fd
  0x7b2093622500: fa fa fd fd fd fd fd fd fa fa fd fd fd fd fd fd
  0x7b2093622580: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x7b2093622600: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x7b2093622680: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x7b2093622700: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==65==ABORTINGProgram return code 42 (ASAN failure). Log appended.
Rerun your program through a sanitizer such as ASAN or valgrind.
BEGIN: Simple circle sanity radius 0
BEGIN: attempting to load functions load and save from shared library
END: loading successful
BEGIN: create_canvas w=20 h=20 c0x=00000000
END: create_canvas
BEGIN: create_circle r=0 c=0xff112233 filled:=True
END: create_circle
BEGIN: place_sprite x=1 y=2
END: place_sprite
BEGIN: generate frame f=0 fr=25
END: generate frame
BEGIN: check frame
END: check frame
BEGIN: destroy_canvas
END: destroy_canvas
BEGIN: destroy_sprite
=================================================================
==68==ERROR: AddressSanitizer: heap-use-after-free on address 0x72244ce224b0 at pc 0x75e44cff22c3 bp 0x7fff8d81f1f0 sp 0x7fff8d81f1e0
READ of size 4 at 0x72244ce224b0 thread T0
    #0 0x75e44cff22c2 in sprite_unref /home/animate.c:61
    #1 0x75e44cff3137 in animate_destroy_sprite /home/animate.c:195
    #2 0x75e44cffb8b3 in wrapped_destroy_sprite staff/animate_wrap.c:161
    #3 0x75e44ec11ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #4 0x75e44ec0e76a  (/usr/lib/libffi.so.8+0x476a) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #5 0x75e44ec1106d in ffi_call (/usr/lib/libffi.so.8+0x706d) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x75e44cfd1c4f  (/usr/lib/python3.14/lib-dynload/_ctypes.cpython-314-x86_64-linux-gnu.so+0x13c4f) (BuildId: 4ecdae4bdfcdb64e8aebbddce9d8a5786a091a5d)
    #7 0x75e44cfc3937  (/usr/lib/python3.14/lib-dynload/_ctypes.cpython-314-x86_64-linux-gnu.so+0x5937) (BuildId: 4ecdae4bdfcdb64e8aebbddce9d8a5786a091a5d)
    #8 0x75e44e77867b in _PyObject_MakeTpCall (/usr/lib/libpython3.14.so.1.0+0x17867b) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #9 0x75e44e791151 in _PyEval_EvalFrameDefault (/usr/lib/libpython3.14.so.1.0+0x191151) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #10 0x75e44e78b804  (/usr/lib/libpython3.14.so.1.0+0x18b804) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #11 0x75e44e876d2d in PyEval_EvalCode (/usr/lib/libpython3.14.so.1.0+0x276d2d) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #12 0x75e44e8b70d0  (/usr/lib/libpython3.14.so.1.0+0x2b70d0) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #13 0x75e44e8b624a  (/usr/lib/libpython3.14.so.1.0+0x2b624a) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #14 0x75e44e8b5c06  (/usr/lib/libpython3.14.so.1.0+0x2b5c06) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #15 0x75e44e8b59e6  (/usr/lib/libpython3.14.so.1.0+0x2b59e6) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #16 0x75e44e86c081 in Py_RunMain (/usr/lib/libpython3.14.so.1.0+0x26c081) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #17 0x75e44e86562a in Py_BytesMain (/usr/lib/libpython3.14.so.1.0+0x26562a) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #18 0x75e44e227634 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
    #19 0x75e44e2276e8 in __libc_start_main_impl ../csu/libc-start.c:360
    #20 0x5d5152125044 in _start (/usr/bin/python3.14+0x1044) (BuildId: ddf03afb83452757f6df249b7608c7df7b476e25)
0x72244ce224b0 is located 32 bytes inside of 40-byte region [0x72244ce22490,0x72244ce224b8)
freed by thread T0 here:
    #0 0x75e44ef1f79d in free /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:51
    #1 0x75e44f68dcad in free (staff/override_malloc_free.so+0x6cad) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x75e44cff231f in sprite_unref /home/animate.c:61
    #3 0x75e44cff307d in animate_destroy_canvas /home/animate.c:170
    #4 0x75e44cffb78c in wrapped_destroy_canvas staff/animate_wrap.c:134
    #5 0x75e44ec11ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7fff8d81f40f  ([stack]+0x1e40f)
previously allocated by thread T0 here:
    #0 0x75e44ef20cb5 in malloc /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:67
    #1 0x75e44f68db85 in malloc (staff/override_malloc_free.so+0x6b85) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x75e44cff2c20 in make_circle /home/animate.c:130
    #3 0x75e44cff30ec in animate_create_circle /home/animate.c:185
    #4 0x75e44cffb872 in wrapped_create_circle staff/animate_wrap.c:154
    #5 0x75e44ec11ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7fff8d81f3af  ([stack]+0x1e3af)
SUMMARY: AddressSanitizer: heap-use-after-free /home/animate.c:61 in sprite_unref
Shadow bytes around the buggy address:
  0x72244ce22200: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x72244ce22280: fa fa 00 00 00 00 00 fa fa fa 00 00 00 00 00 fa
  0x72244ce22300: fa fa 00 00 00 00 00 fa fa fa 00 00 00 00 00 fa
  0x72244ce22380: fa fa 00 00 00 00 00 02 fa fa 00 00 00 00 05 fa
  0x72244ce22400: fa fa fd fd fd fd fd fa fa fa fd fd fd fd fd fd
=>0x72244ce22480: fa fa fd fd fd fd[fd]fa fa fa fd fd fd fd fd fd
  0x72244ce22500: fa fa fd fd fd fd fd fd fa fa fd fd fd fd fd fd
  0x72244ce22580: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x72244ce22600: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x72244ce22680: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x72244ce22700: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==68==ABORTINGProgram return code 42 (ASAN failure). Log appended.
Rerun your program through a sanitizer such as ASAN or valgrind.
BEGIN: Image bounds sanity; TopLeft
BEGIN: attempting to load functions load and save from shared library
END: loading successful
BEGIN: create_canvas w=10 h=10 c0x=00000000
END: create_canvas
BEGIN: create_rectangle w=10 h=10 c=0xff112233 filled:=True
END: create_rectangle
BEGIN: place_sprite x=-5 y=-5
END: place_sprite
BEGIN: generate frame f=0 fr=25
END: generate frame
BEGIN: check frame
END: check frame
BEGIN: destroy_placement
END: destroy_placement
BEGIN: destroy_canvas
END: destroy_canvas
BEGIN: destroy_sprite
=================================================================
==71==ERROR: AddressSanitizer: heap-use-after-free on address 0x77e7108224b0 at pc 0x7ba710b262c3 bp 0x7ffc0dcc2b40 sp 0x7ffc0dcc2b30
READ of size 4 at 0x77e7108224b0 thread T0
    #0 0x7ba710b262c2 in sprite_unref /home/animate.c:61
    #1 0x7ba710b27137 in animate_destroy_sprite /home/animate.c:195
    #2 0x7ba710b2f8b3 in wrapped_destroy_sprite staff/animate_wrap.c:161
    #3 0x7ba712f14ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #4 0x7ba712f1176a  (/usr/lib/libffi.so.8+0x476a) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #5 0x7ba712f1406d in ffi_call (/usr/lib/libffi.so.8+0x706d) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7ba7109f3c4f  (/usr/lib/python3.14/lib-dynload/_ctypes.cpython-314-x86_64-linux-gnu.so+0x13c4f) (BuildId: 4ecdae4bdfcdb64e8aebbddce9d8a5786a091a5d)
    #7 0x7ba7109e5937  (/usr/lib/python3.14/lib-dynload/_ctypes.cpython-314-x86_64-linux-gnu.so+0x5937) (BuildId: 4ecdae4bdfcdb64e8aebbddce9d8a5786a091a5d)
    #8 0x7ba71217867b in _PyObject_MakeTpCall (/usr/lib/libpython3.14.so.1.0+0x17867b) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #9 0x7ba712191151 in _PyEval_EvalFrameDefault (/usr/lib/libpython3.14.so.1.0+0x191151) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #10 0x7ba71218b804  (/usr/lib/libpython3.14.so.1.0+0x18b804) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #11 0x7ba712276d2d in PyEval_EvalCode (/usr/lib/libpython3.14.so.1.0+0x276d2d) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #12 0x7ba7122b70d0  (/usr/lib/libpython3.14.so.1.0+0x2b70d0) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #13 0x7ba7122b624a  (/usr/lib/libpython3.14.so.1.0+0x2b624a) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #14 0x7ba7122b5c06  (/usr/lib/libpython3.14.so.1.0+0x2b5c06) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #15 0x7ba7122b59e6  (/usr/lib/libpython3.14.so.1.0+0x2b59e6) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #16 0x7ba71226c081 in Py_RunMain (/usr/lib/libpython3.14.so.1.0+0x26c081) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #17 0x7ba71226562a in Py_BytesMain (/usr/lib/libpython3.14.so.1.0+0x26562a) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #18 0x7ba711c27634 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
    #19 0x7ba711c276e8 in __libc_start_main_impl ../csu/libc-start.c:360
    #20 0x639953d61044 in _start (/usr/bin/python3.14+0x1044) (BuildId: ddf03afb83452757f6df249b7608c7df7b476e25)
0x77e7108224b0 is located 32 bytes inside of 40-byte region [0x77e710822490,0x77e7108224b8)
freed by thread T0 here:
    #0 0x7ba71291f79d in free /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:51
    #1 0x7ba7130c5cad in free (staff/override_malloc_free.so+0x6cad) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x7ba710b2631f in sprite_unref /home/animate.c:61
    #3 0x7ba710b27d05 in animate_destroy_placement /home/animate.c:282
    #4 0x7ba710b2fa35 in wrapped_destroy_placement staff/animate_wrap.c:199
    #5 0x7ba712f14ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7ffc0dcc2d5f  ([stack]+0x1dd5f)
previously allocated by thread T0 here:
    #0 0x7ba712920cb5 in malloc /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:67
    #1 0x7ba7130c5b85 in malloc (staff/override_malloc_free.so+0x6b85) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x7ba710b26967 in make_rect /home/animate.c:109
    #3 0x7ba710b2711d in animate_create_rectangle /home/animate.c:190
    #4 0x7ba710b2f81f in wrapped_create_rectangle staff/animate_wrap.c:147
    #5 0x7ba712f14ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7ffc0dcc2ccf  ([stack]+0x1dccf)
SUMMARY: AddressSanitizer: heap-use-after-free /home/animate.c:61 in sprite_unref
Shadow bytes around the buggy address:
  0x77e710822200: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x77e710822280: fa fa 00 00 00 00 00 fa fa fa 00 00 00 00 00 fa
  0x77e710822300: fa fa 00 00 00 00 00 fa fa fa 00 00 00 00 00 fa
  0x77e710822380: fa fa 00 00 00 00 00 02 fa fa 00 00 00 00 05 fa
  0x77e710822400: fa fa fd fd fd fd fd fa fa fa fd fd fd fd fd fd
=>0x77e710822480: fa fa fd fd fd fd[fd]fa fa fa fd fd fd fd fd fd
  0x77e710822500: fa fa fd fd fd fd fd fd fa fa fd fd fd fd fd fd
  0x77e710822580: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x77e710822600: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x77e710822680: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x77e710822700: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==71==ABORTINGProgram return code 42 (ASAN failure). Log appended.
Rerun your program through a sanitizer such as ASAN or valgrind.
BEGIN: Image bounds circle sanity; TopLeft
BEGIN: attempting to load functions load and save from shared library
END: loading successful
BEGIN: create_canvas w=10 h=10 c0x=00000000
END: create_canvas
BEGIN: create_circle r=5 c=0xff112233 filled:=True
END: create_circle
BEGIN: place_sprite x=-5 y=-5
END: place_sprite
BEGIN: generate frame f=0 fr=25
END: generate frame
BEGIN: check frame
END: check frame
BEGIN: destroy_placement
END: destroy_placement
BEGIN: destroy_canvas
END: destroy_canvas
BEGIN: destroy_sprite
=================================================================
==74==ERROR: AddressSanitizer: heap-use-after-free on address 0x71d49e6224b0 at pc 0x75949e9482c3 bp 0x7fff8c3f2c50 sp 0x7fff8c3f2c40
READ of size 4 at 0x71d49e6224b0 thread T0
    #0 0x75949e9482c2 in sprite_unref /home/animate.c:61
    #1 0x75949e949137 in animate_destroy_sprite /home/animate.c:195
    #2 0x75949e9518b3 in wrapped_destroy_sprite staff/animate_wrap.c:161
    #3 0x7594a0d11ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #4 0x7594a0d0e76a  (/usr/lib/libffi.so.8+0x476a) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #5 0x7594a0d1106d in ffi_call (/usr/lib/libffi.so.8+0x706d) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x75949e927c4f  (/usr/lib/python3.14/lib-dynload/_ctypes.cpython-314-x86_64-linux-gnu.so+0x13c4f) (BuildId: 4ecdae4bdfcdb64e8aebbddce9d8a5786a091a5d)
    #7 0x75949e919937  (/usr/lib/python3.14/lib-dynload/_ctypes.cpython-314-x86_64-linux-gnu.so+0x5937) (BuildId: 4ecdae4bdfcdb64e8aebbddce9d8a5786a091a5d)
    #8 0x75949ff7867b in _PyObject_MakeTpCall (/usr/lib/libpython3.14.so.1.0+0x17867b) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #9 0x75949ff91151 in _PyEval_EvalFrameDefault (/usr/lib/libpython3.14.so.1.0+0x191151) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #10 0x75949ff8b804  (/usr/lib/libpython3.14.so.1.0+0x18b804) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #11 0x7594a0076d2d in PyEval_EvalCode (/usr/lib/libpython3.14.so.1.0+0x276d2d) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #12 0x7594a00b70d0  (/usr/lib/libpython3.14.so.1.0+0x2b70d0) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #13 0x7594a00b624a  (/usr/lib/libpython3.14.so.1.0+0x2b624a) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #14 0x7594a00b5c06  (/usr/lib/libpython3.14.so.1.0+0x2b5c06) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #15 0x7594a00b59e6  (/usr/lib/libpython3.14.so.1.0+0x2b59e6) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #16 0x7594a006c081 in Py_RunMain (/usr/lib/libpython3.14.so.1.0+0x26c081) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #17 0x7594a006562a in Py_BytesMain (/usr/lib/libpython3.14.so.1.0+0x26562a) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #18 0x75949fa27634 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
    #19 0x75949fa276e8 in __libc_start_main_impl ../csu/libc-start.c:360
    #20 0x55e14a88b044 in _start (/usr/bin/python3.14+0x1044) (BuildId: ddf03afb83452757f6df249b7608c7df7b476e25)
0x71d49e6224b0 is located 32 bytes inside of 40-byte region [0x71d49e622490,0x71d49e6224b8)
freed by thread T0 here:
    #0 0x7594a071f79d in free /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:51
    #1 0x7594a0ee4cad in free (staff/override_malloc_free.so+0x6cad) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x75949e94831f in sprite_unref /home/animate.c:61
    #3 0x75949e949d05 in animate_destroy_placement /home/animate.c:282
    #4 0x75949e951a35 in wrapped_destroy_placement staff/animate_wrap.c:199
    #5 0x7594a0d11ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7fff8c3f2e6f  ([stack]+0x1de6f)
previously allocated by thread T0 here:
    #0 0x7594a0720cb5 in malloc /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:67
    #1 0x7594a0ee4b85 in malloc (staff/override_malloc_free.so+0x6b85) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x75949e948c20 in make_circle /home/animate.c:130
    #3 0x75949e9490ec in animate_create_circle /home/animate.c:185
    #4 0x75949e951872 in wrapped_create_circle staff/animate_wrap.c:154
    #5 0x7594a0d11ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7fff8c3f2e0f  ([stack]+0x1de0f)
SUMMARY: AddressSanitizer: heap-use-after-free /home/animate.c:61 in sprite_unref
Shadow bytes around the buggy address:
  0x71d49e622200: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x71d49e622280: fa fa 00 00 00 00 00 fa fa fa 00 00 00 00 00 fa
  0x71d49e622300: fa fa 00 00 00 00 00 fa fa fa 00 00 00 00 00 fa
  0x71d49e622380: fa fa 00 00 00 00 00 02 fa fa 00 00 00 00 05 fa
  0x71d49e622400: fa fa fd fd fd fd fd fa fa fa fd fd fd fd fd fd
=>0x71d49e622480: fa fa fd fd fd fd[fd]fa fa fa fd fd fd fd fd fd
  0x71d49e622500: fa fa fd fd fd fd fd fd fa fa fd fd fd fd fd fd
  0x71d49e622580: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x71d49e622600: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x71d49e622680: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x71d49e622700: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==74==ABORTINGProgram return code 42 (ASAN failure). Log appended.
Rerun your program through a sanitizer such as ASAN or valgrind.
BEGIN: Bitmap sanity crosshair.bmp
BEGIN: attempting to load functions load and save from shared library
END: loading successful
BEGIN: create_canvas w=100 h=100 c0x=00808080
END: create_canvas
BEGIN: create_sprite crosshair.bmp
=================================================================
==77==ERROR: AddressSanitizer: requested allocation size 0x19000000000 (0x19000000040 after adjustments for alignment, red zones etc.) exceeds maximum supported size of 0x10000000000 (thread T0)
    #0 0x711955320cb5 in malloc /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:67
    #1 0x711955aacb85 in malloc (staff/override_malloc_free.so+0x6b85) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x71195352e6ea in load_bmp /home/animate.c:93
    #3 0x71195352f0c5 in animate_create_sprite /home/animate.c:179
    #4 0x71195397d7c6 in wrapped_create_sprite staff/animate_wrap.c:140
    #5 0x711954195ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7fff4f72481f  ([stack]+0x1e81f)
==77==HINT: if you don't care about these errors you may set allocator_may_return_null=1
SUMMARY: AddressSanitizer: allocation-size-too-big (staff/override_malloc_free.so+0x6b85) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884) in malloc
==77==ABORTINGprerequisites needed: ['no restricted functions', 'simple rectangle sanity']↵prerequisites not met
prerequisites needed: ['no restricted functions', 'simple rectangle sanity']↵
prerequisites not met
prerequisites needed: ['no restricted functions', 'simple rectangle sanity']↵Program return code 42 (ASAN failure). Log appended.
Rerun your program through a sanitizer such as ASAN or valgrind.
BEGIN: Animation No motion
BEGIN: attempting to load functions load and save from shared library
END: loading successful
BEGIN: create_canvas w=10 h=20 c0x=00bacbac
END: create_canvas
BEGIN: create_rectangle w=4 h=3 c=0xff112233 filled:=True
END: create_rectangle
BEGIN: place_sprite x=1 y=2
END: place_sprite
BEGIN: create_rectangle w=2 h=3 c=0xff445566 filled:=True
END: create_rectangle
BEGIN: place_sprite x=2 y=3
END: place_sprite
BEGIN: set_animation_params vx=0 vy=0 ax=0 ay=0
END: set_animation_params
BEGIN: generate frame f=0 fr=1
END: generate frame
BEGIN: check frame 0
END: check frame 0
BEGIN: generate frame f=1 fr=1
END: generate frame
BEGIN: check frame 1
END: check frame 1
BEGIN: generate frame f=2 fr=1
END: generate frame
BEGIN: check frame 2
END: check frame 2
BEGIN: destroy_canvas
END: destroy_canvas
BEGIN: destroy_sprite
=================================================================
==80==ERROR: AddressSanitizer: heap-use-after-free on address 0x709ed86224b0 at pc 0x745ed876a2c3 bp 0x7ffd5a5d0e70 sp 0x7ffd5a5d0e60
READ of size 4 at 0x709ed86224b0 thread T0
    #0 0x745ed876a2c2 in sprite_unref /home/animate.c:61
    #1 0x745ed876b137 in animate_destroy_sprite /home/animate.c:195
    #2 0x745ed87738b3 in wrapped_destroy_sprite staff/animate_wrap.c:161
    #3 0x745edad14ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #4 0x745edad1176a  (/usr/lib/libffi.so.8+0x476a) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #5 0x745edad1406d in ffi_call (/usr/lib/libffi.so.8+0x706d) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x745ed8749c4f  (/usr/lib/python3.14/lib-dynload/_ctypes.cpython-314-x86_64-linux-gnu.so+0x13c4f) (BuildId: 4ecdae4bdfcdb64e8aebbddce9d8a5786a091a5d)
    #7 0x745ed873b937  (/usr/lib/python3.14/lib-dynload/_ctypes.cpython-314-x86_64-linux-gnu.so+0x5937) (BuildId: 4ecdae4bdfcdb64e8aebbddce9d8a5786a091a5d)
    #8 0x745ed9f7867b in _PyObject_MakeTpCall (/usr/lib/libpython3.14.so.1.0+0x17867b) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #9 0x745ed9f91151 in _PyEval_EvalFrameDefault (/usr/lib/libpython3.14.so.1.0+0x191151) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #10 0x745ed9f8b804  (/usr/lib/libpython3.14.so.1.0+0x18b804) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #11 0x745eda076d2d in PyEval_EvalCode (/usr/lib/libpython3.14.so.1.0+0x276d2d) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #12 0x745eda0b70d0  (/usr/lib/libpython3.14.so.1.0+0x2b70d0) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #13 0x745eda0b624a  (/usr/lib/libpython3.14.so.1.0+0x2b624a) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #14 0x745eda0b5c06  (/usr/lib/libpython3.14.so.1.0+0x2b5c06) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #15 0x745eda0b59e6  (/usr/lib/libpython3.14.so.1.0+0x2b59e6) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #16 0x745eda06c081 in Py_RunMain (/usr/lib/libpython3.14.so.1.0+0x26c081) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #17 0x745eda06562a in Py_BytesMain (/usr/lib/libpython3.14.so.1.0+0x26562a) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #18 0x745ed9a27634 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
    #19 0x745ed9a276e8 in __libc_start_main_impl ../csu/libc-start.c:360
    #20 0x628e51bc0044 in _start (/usr/bin/python3.14+0x1044) (BuildId: ddf03afb83452757f6df249b7608c7df7b476e25)
0x709ed86224b0 is located 32 bytes inside of 40-byte region [0x709ed8622490,0x709ed86224b8)
freed by thread T0 here:
    #0 0x745edad1f79d in free /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:51
    #1 0x745edadfbcad in free (staff/override_malloc_free.so+0x6cad) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x745ed876a31f in sprite_unref /home/animate.c:61
    #3 0x745ed876b07d in animate_destroy_canvas /home/animate.c:170
    #4 0x745ed877378c in wrapped_destroy_canvas staff/animate_wrap.c:134
    #5 0x745edad14ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7ffd5a5d108f  ([stack]+0x1e08f)
previously allocated by thread T0 here:
    #0 0x745eda720cb5 in malloc /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:67
    #1 0x745edae1bb85 in malloc (staff/override_malloc_free.so+0x6b85) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x745ed876a967 in make_rect /home/animate.c:109
    #3 0x745ed876b11d in animate_create_rectangle /home/animate.c:190
    #4 0x745ed877381f in wrapped_create_rectangle staff/animate_wrap.c:147
    #5 0x745edad14ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7ffd5a5d0fff  ([stack]+0x1dfff)
SUMMARY: AddressSanitizer: heap-use-after-free /home/animate.c:61 in sprite_unref
Shadow bytes around the buggy address:
  0x709ed8622200: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x709ed8622280: fa fa 00 00 00 00 00 fa fa fa 00 00 00 00 00 fa
  0x709ed8622300: fa fa 00 00 00 00 00 fa fa fa 00 00 00 00 00 fa
  0x709ed8622380: fa fa 00 00 00 00 00 02 fa fa 00 00 00 00 05 fa
  0x709ed8622400: fa fa fd fd fd fd fd fa fa fa fd fd fd fd fd fd
=>0x709ed8622480: fa fa fd fd fd fd[fd]fa fa fa fd fd fd fd fd fd
  0x709ed8622500: fa fa fd fd fd fd fd fd fa fa fd fd fd fd fd fd
  0x709ed8622580: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x709ed8622600: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x709ed8622680: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x709ed8622700: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==74==ABORTINGProgram return code 42 (ASAN failure). Log appended.
Rerun your program through a sanitizer such as ASAN or valgrind.
BEGIN: Bitmap sanity crosshair.bmp
BEGIN: attempting to load functions load and save from shared library
END: loading successful
BEGIN: create_canvas w=100 h=100 c0x=00808080
END: create_canvas
BEGIN: create_sprite crosshair.bmp
=================================================================
==77==ERROR: AddressSanitizer: requested allocation size 0x19000000000 (0x19000000040 after adjustments for alignment, red zones etc.) exceeds maximum supported size of 0x10000000000 (thread T0)
    #0 0x711955320cb5 in malloc /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:67
    #1 0x711955aacb85 in malloc (staff/override_malloc_free.so+0x6b85) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x71195352e6ea in load_bmp /home/animate.c:93
    #3 0x71195352f0c5 in animate_create_sprite /home/animate.c:179
    #4 0x71195397d7c6 in wrapped_create_sprite staff/animate_wrap.c:140
    #5 0x711954195ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7fff4f72481f  ([stack]+0x1e81f)
==77==HINT: if you don't care about these errors you may set allocator_may_return_null=1
SUMMARY: AddressSanitizer: allocation-size-too-big (staff/override_malloc_free.so+0x6b85) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884) in malloc
==77==ABORTINGprerequisites needed: ['no restricted functions', 'simple rectangle sanity']↵prerequisites not met
prerequisites needed: ['no restricted functions', 'simple rectangle sanity']↵
prerequisites not met
prerequisites needed: ['no restricted functions', 'simple rectangle sanity']↵Program return code 42 (ASAN failure). Log appended.
Rerun your program through a sanitizer such as ASAN or valgrind.
BEGIN: Animation No motion
BEGIN: attempting to load functions load and save from shared library
END: loading successful
BEGIN: create_canvas w=10 h=20 c0x=00bacbac
END: create_canvas
BEGIN: create_rectangle w=4 h=3 c=0xff112233 filled:=True
END: create_rectangle
BEGIN: place_sprite x=1 y=2
END: place_sprite
BEGIN: create_rectangle w=2 h=3 c=0xff445566 filled:=True
END: create_rectangle
BEGIN: place_sprite x=2 y=3
END: place_sprite
BEGIN: set_animation_params vx=0 vy=0 ax=0 ay=0
END: set_animation_params
BEGIN: generate frame f=0 fr=1
END: generate frame
BEGIN: check frame 0
END: check frame 0
BEGIN: generate frame f=1 fr=1
END: generate frame
BEGIN: check frame 1
END: check frame 1
BEGIN: generate frame f=2 fr=1
END: generate frame
BEGIN: check frame 2
END: check frame 2
BEGIN: destroy_canvas
END: destroy_canvas
BEGIN: destroy_sprite
=================================================================
==80==ERROR: AddressSanitizer: heap-use-after-free on address 0x709ed86224b0 at pc 0x745ed876a2c3 bp 0x7ffd5a5d0e70 sp 0x7ffd5a5d0e60
READ of size 4 at 0x709ed86224b0 thread T0
    #0 0x745ed876a2c2 in sprite_unref /home/animate.c:61
    #1 0x745ed876b137 in animate_destroy_sprite /home/animate.c:195
    #2 0x745ed87738b3 in wrapped_destroy_sprite staff/animate_wrap.c:161
    #3 0x745edad14ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #4 0x745edad1176a  (/usr/lib/libffi.so.8+0x476a) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #5 0x745edad1406d in ffi_call (/usr/lib/libffi.so.8+0x706d) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x745ed8749c4f  (/usr/lib/python3.14/lib-dynload/_ctypes.cpython-314-x86_64-linux-gnu.so+0x13c4f) (BuildId: 4ecdae4bdfcdb64e8aebbddce9d8a5786a091a5d)
    #7 0x745ed873b937  (/usr/lib/python3.14/lib-dynload/_ctypes.cpython-314-x86_64-linux-gnu.so+0x5937) (BuildId: 4ecdae4bdfcdb64e8aebbddce9d8a5786a091a5d)
    #8 0x745ed9f7867b in _PyObject_MakeTpCall (/usr/lib/libpython3.14.so.1.0+0x17867b) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #9 0x745ed9f91151 in _PyEval_EvalFrameDefault (/usr/lib/libpython3.14.so.1.0+0x191151) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #10 0x745ed9f8b804  (/usr/lib/libpython3.14.so.1.0+0x18b804) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #11 0x745eda076d2d in PyEval_EvalCode (/usr/lib/libpython3.14.so.1.0+0x276d2d) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #12 0x745eda0b70d0  (/usr/lib/libpython3.14.so.1.0+0x2b70d0) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #13 0x745eda0b624a  (/usr/lib/libpython3.14.so.1.0+0x2b624a) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #14 0x745eda0b5c06  (/usr/lib/libpython3.14.so.1.0+0x2b5c06) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #15 0x745eda0b59e6  (/usr/lib/libpython3.14.so.1.0+0x2b59e6) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #16 0x745eda06c081 in Py_RunMain (/usr/lib/libpython3.14.so.1.0+0x26c081) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #17 0x745eda06562a in Py_BytesMain (/usr/lib/libpython3.14.so.1.0+0x26562a) (BuildId: 52cae3244274805bf3e4640e0f90a8a50c9643c9)
    #18 0x745ed9a27634 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
    #19 0x745ed9a276e8 in __libc_start_main_impl ../csu/libc-start.c:360
    #20 0x628e51bc0044 in _start (/usr/bin/python3.14+0x1044) (BuildId: ddf03afb83452757f6df249b7608c7df7b476e25)
0x709ed86224b0 is located 32 bytes inside of 40-byte region [0x709ed8622490,0x709ed86224b8)
freed by thread T0 here:
    #0 0x745edad1f79d in free /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:51
    #1 0x745edadfbcad in free (staff/override_malloc_free.so+0x6cad) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x745ed876a31f in sprite_unref /home/animate.c:61
    #3 0x745ed876b07d in animate_destroy_canvas /home/animate.c:170
    #4 0x745ed877378c in wrapped_destroy_canvas staff/animate_wrap.c:134
    #5 0x745edad14ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7ffd5a5d108f  ([stack]+0x1e08f)
previously allocated by thread T0 here:
    #0 0x745eda720cb5 in malloc /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:67
    #1 0x745edae1bb85 in malloc (staff/override_malloc_free.so+0x6b85) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x745ed876a967 in make_rect /home/animate.c:109
    #3 0x745ed876b11d in animate_create_rectangle /home/animate.c:190
    #4 0x745ed877381f in wrapped_create_rectangle staff/animate_wrap.c:147
    #5 0x745edad14ac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7ffd5a5d0fff  ([stack]+0x1dfff)
SUMMARY: AddressSanitizer: heap-use-after-free /home/animate.c:61 in sprite_unref
Shadow bytes around the buggy address:
  0x709ed8622200: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x709ed8622280: fa fa 00 00 00 00 00 fa fa fa 00 00 00 00 00 fa
  0x709ed8622300: fa fa 00 00 00 00 00 fa fa fa 00 00 00 00 00 fa
  0x709ed8622380: fa fa 00 00 00 00 00 02 fa fa 00 00 00 00 05 fa
  0x709ed8622400: fa fa fd fd fd fd fd fa fa fa fd fd fd fd fd fd
=>0x709ed8622480: fa fa fd fd fd fd[fd]fa fa fa fd fd fd fd fd fd
  0x709ed8622500: fa fa fd fd fd fd fd fd fa fa fd fd fd fd fd fd
  0x709ed8622580: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x709ed8622600: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x709ed8622680: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x709ed8622700: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==80==ABORTINGBEGIN: Reuses sprite memory
BEGIN: attempting to load functions load and save from shared library
END: loading successful
BEGIN: create_canvas w=256 h=384 c0x=00bacbac
END: create_canvas
BEGIN: create_sprite rick.bmp
=================================================================
==83==ERROR: AddressSanitizer: requested allocation size 0x20000000000 (0x20000000040 after adjustments for alignment, red zones etc.) exceeds maximum supported size of 0x10000000000 (thread T0)
    #0 0x72d709920cb5 in malloc /usr/src/debug/gcc/gcc/libsanitizer/asan/asan_malloc_linux.cpp:67
    #1 0x72d709f51b85 in malloc (staff/override_malloc_free.so+0x6b85) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884)
    #2 0x6ed7055e26ea in load_bmp /home/animate.c:93
    #3 0x6ed7055e30c5 in animate_create_sprite /home/animate.c:179
    #4 0x6ed705af47c6 in wrapped_create_sprite staff/animate_wrap.c:140
    #5 0x72d70961aac5  (/usr/lib/libffi.so.8+0x7ac5) (BuildId: d5e3b0d8921923f35438adefa9f864745abc5e90)
    #6 0x7ffe388ddebf  ([stack]+0x1debf)
==83==HINT: if you don't care about these errors you may set allocator_may_return_null=1
SUMMARY: AddressSanitizer: allocation-size-too-big (staff/override_malloc_free.so+0x6b85) (BuildId: 0c30d8bc31dc779b6ca3d28c2473f911d00f2884) in malloc
==83==ABORTING

