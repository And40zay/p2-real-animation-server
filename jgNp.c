#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "rpc_handler.h"

#define MAX_RESPONSE 1024
#define FIFO_PATH "/tmp/animate_fifos"

int main(int argc, char** argv, char** envp) {
    printf("Animation Server Starting...\n");
    
    // Create FIFO directory if it doesn't exist
    mkdir(FIFO_PATH, 0755);
    
    // For now, just test that we can call animate functions
    struct canvas* test_canvas = animate_create_canvas(100, 100, 0);
    printf("Created test canvas: %p\n", test_canvas);
    
    struct sprite* rect = animate_create_rectangle(10, 20, 0xFF0000, 1);
    printf("Created test rectangle: %p\n", rect);
    
    struct sprite_placement* placement = animate_place_sprite(test_canvas, rect, 10, 10);
    printf("Created placement: %p\n", placement);
    
    // Test RPC handler
    client_session_t test_client = {
        .client_pid = getpid(),
        .logged_in = 1,
        .username = "testuser",
        .balance = 100
    };
    
    char response[MAX_RESPONSE];
    
    // Test a command
    handle_rpc(&test_client, "create_rectangle 10 20 0xFF0000 1", response, MAX_RESPONSE);
    printf("RPC Response: %s", response);
    
    // Clean up
    animate_destroy_placement(placement);
    animate_destroy_sprite(rect);
    animate_destroy_canvas(test_canvas);
    
    printf("Server shutdown complete\n");
    return 0;
}