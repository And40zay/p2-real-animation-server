
#include <animate/animate.h>

int main(int argc, char** argv, char** envp) {

    struct canvas* canvas = animate_create_canvas(100,100,0);
    animate_destroy_canvas(canvas);
    
    return 0;
}
