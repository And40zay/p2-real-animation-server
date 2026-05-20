#include "rpc_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Helper function to parse arguments from command string
static int parse_args(const char* cmd, char* tokens[], int max_tokens) {
    char buffer[512];
    strncpy(buffer, cmd, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    int count = 0;
    char* token = strtok(buffer, " \t\n");
    
    while (token != NULL && count < max_tokens) {
        tokens[count++] = strdup(token);
        token = strtok(NULL, " \t\n");
    }
    
    return count;
}

// Clean up tokens
static void free_tokens(char* tokens[], int count) {
    for (int i = 0; i < count; i++) {
        free(tokens[i]);
    }
}

// Command handlers
static void cmd_create_rectangle(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    // Command: create_rectangle width height color filled
    // That's 5 tokens total (index 0-4)
    if (argc != 5) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    // Parse arguments
    size_t width = atoi(tokens[1]);
    size_t height = atoi(tokens[2]);
    unsigned long color_val = strtoul(tokens[3], NULL, 16);  // Parse hex color
    color_t color = (color_t)color_val;
    bool filled = (atoi(tokens[4]) != 0);
    
    printf("Creating rectangle: %ldx%ld color=0x%06lx filled=%d\n", 
           width, height, color_val, filled);
    
    struct sprite* sprite = animate_create_rectangle(width, height, color, filled);
    
    if (sprite == NULL) {
        snprintf(response, 256, "%d\n", RPC_INTERNAL_ERROR);
        return;
    }
    
    // Send back the sprite address as handle
    snprintf(response, 256, "%d %lu\n", RPC_SUCCESS, (unsigned long)sprite);
}

static void cmd_create_circle(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    if (argc != 3) {  // command + radius + color + filled
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    size_t radius = atoi(tokens[1]);
    color_t color = (color_t)atol(tokens[2]);
    bool filled = (argc > 3) ? (atoi(tokens[3]) != 0) : true;
    
    struct sprite* sprite = animate_create_circle(radius, color, filled);
    
    if (sprite == NULL) {
        snprintf(response, 256, "%d\n", RPC_INTERNAL_ERROR);
        return;
    }
    
    snprintf(response, 256, "%d %lu\n", RPC_SUCCESS, (unsigned long)sprite);
}

static void cmd_create_sprite(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    if (argc != 2) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    struct sprite* sprite = animate_create_sprite(tokens[1]);
    
    if (sprite == NULL) {
        snprintf(response, 256, "%d\n", RPC_INTERNAL_ERROR);
        return;
    }
    
    snprintf(response, 256, "%d %lu\n", RPC_SUCCESS, (unsigned long)sprite);
}

static void cmd_create_canvas(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    // Command: create_canvas height width background_color
    if (argc != 4) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    size_t height = atoi(tokens[1]);
    size_t width = atoi(tokens[2]);
    unsigned long bg_val = strtoul(tokens[3], NULL, 16);
    color_t bg_color = (color_t)bg_val;
    
    struct canvas* canvas = animate_create_canvas(height, width, bg_color);
    
    if (canvas == NULL) {
        snprintf(response, 256, "%d\n", RPC_INTERNAL_ERROR);
        return;
    }
    
    snprintf(response, 256, "%d %lu\n", RPC_SUCCESS, (unsigned long)canvas);
}

static void cmd_place_sprite(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    // Command: place_sprite canvas_handle sprite_handle x y
    if (argc != 5) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    struct canvas* canvas = (struct canvas*)strtoul(tokens[1], NULL, 10);
    struct sprite* sprite = (struct sprite*)strtoul(tokens[2], NULL, 10);
    ssize_t x = atol(tokens[3]);
    ssize_t y = atol(tokens[4]);
    
    struct sprite_placement* placement = animate_place_sprite(canvas, sprite, x, y);
    
    if (placement == NULL) {
        snprintf(response, 256, "%d\n", RPC_INTERNAL_ERROR);
        return;
    }
    
    snprintf(response, 256, "%d %lu\n", RPC_SUCCESS, (unsigned long)placement);
}

// Main RPC handler
void handle_rpc(client_session_t* client, const char* command, char* response, size_t response_size) {
    char* tokens[16];
    int argc = parse_args(command, tokens, 16);
    
    if (argc == 0) {
        snprintf(response, response_size, "%d\n", RPC_FAILED);
        return;
    }
    
    // Check if logged in for non-login commands
    if (strcmp(tokens[0], "Login") != 0 && strcmp(tokens[0], "Disconnect") != 0) {
        if (!client->logged_in) {
            snprintf(response, response_size, "Not logged in\n");
            free_tokens(tokens, argc);
            return;
        }
    }
    
    // Route to appropriate handler
    if (strcmp(tokens[0], "create_rectangle") == 0) {
        cmd_create_rectangle(client, tokens, argc, response);
    } else if (strcmp(tokens[0], "create_circle") == 0) {
        cmd_create_circle(client, tokens, argc, response);
    } else if (strcmp(tokens[0], "create_sprite") == 0) {
        cmd_create_sprite(client, tokens, argc, response);
    } else if (strcmp(tokens[0], "create_canvas") == 0) {
        cmd_create_canvas(client, tokens, argc, response);
    } else if (strcmp(tokens[0], "place_sprite") == 0) {
        cmd_place_sprite(client, tokens, argc, response);
    } else if (strcmp(tokens[0], "Login") == 0) {
        // Handle login (we'll implement this later)
        snprintf(response, response_size, "Login handler coming soon\n");
    } else {
        snprintf(response, response_size, "%d\n", RPC_FAILED);
    }
    
    free_tokens(tokens, argc);
}