#include "rpc_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Check login against users.txt
int check_login(const char* username, int* balance) {
    FILE* fp = fopen("users.txt", "r");
    if (!fp) return 0;
    
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        char user[33];
        int bal;
        if (sscanf(line, "%32s %d", user, &bal) == 2) {
            if (strcmp(username, user) == 0) {
                *balance = bal;
                fclose(fp);
                return (bal > 0) ? 1 : 0;
            }
        }
    }
    
    fclose(fp);
    return 0;
}

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
static void cmd_login(client_session_t* client, char* tokens[], int argc, char* response) {
    if (argc != 2) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    int balance;
    if (check_login(tokens[1], &balance)) {
        client->logged_in = 1;
        strncpy(client->username, tokens[1], 32);
        client->username[32] = '\0';
        client->balance = balance;
        snprintf(response, 256, "0\n");
        printf("User %s logged in with balance %d\n", client->username, balance);
    } else {
        snprintf(response, 256, "-1\n");
        printf("Failed login attempt for user %s\n", tokens[1]);
    }
}

static void cmd_create_rectangle(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    if (argc != 5) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    size_t width = atoi(tokens[1]);
    size_t height = atoi(tokens[2]);
    unsigned long color_val = strtoul(tokens[3], NULL, 16);
    color_t color = (color_t)color_val;
    bool filled = (atoi(tokens[4]) != 0);
    
    struct sprite* sprite = animate_create_rectangle(width, height, color, filled);
    
    if (sprite == NULL) {
        snprintf(response, 256, "%d\n", RPC_INTERNAL_ERROR);
        return;
    }
    
    snprintf(response, 256, "%d %lu\n", RPC_SUCCESS, (unsigned long)sprite);
}

static void cmd_create_circle(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    if (argc != 3 && argc != 4) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    size_t radius = atoi(tokens[1]);
    unsigned long color_val = strtoul(tokens[2], NULL, 16);
    color_t color = (color_t)color_val;
    bool filled = (argc == 4) ? (atoi(tokens[3]) != 0) : true;
    
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

static void cmd_placement_up(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    if (argc != 2) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    struct sprite_placement* placement = (struct sprite_placement*)strtoul(tokens[1], NULL, 10);
    animate_placement_up(placement);
    snprintf(response, 256, "%d\n", RPC_SUCCESS);
}

static void cmd_placement_down(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    if (argc != 2) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    struct sprite_placement* placement = (struct sprite_placement*)strtoul(tokens[1], NULL, 10);
    animate_placement_down(placement);
    snprintf(response, 256, "%d\n", RPC_SUCCESS);
}

static void cmd_placement_top(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    if (argc != 2) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    struct sprite_placement* placement = (struct sprite_placement*)strtoul(tokens[1], NULL, 10);
    animate_placement_top(placement);
    snprintf(response, 256, "%d\n", RPC_SUCCESS);
}

static void cmd_placement_bottom(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    if (argc != 2) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    struct sprite_placement* placement = (struct sprite_placement*)strtoul(tokens[1], NULL, 10);
    animate_placement_bottom(placement);
    snprintf(response, 256, "%d\n", RPC_SUCCESS);
}

static void cmd_destroy_placement(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    if (argc != 2) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    struct sprite_placement* placement = (struct sprite_placement*)strtoul(tokens[1], NULL, 10);
    animate_destroy_placement(placement);
    snprintf(response, 256, "%d\n", RPC_SUCCESS);
}

static void cmd_destroy_sprite(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    if (argc != 2) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    struct sprite* sprite = (struct sprite*)strtoul(tokens[1], NULL, 10);
    bool result = animate_destroy_sprite(sprite);
    snprintf(response, 256, "%d %d\n", RPC_SUCCESS, result ? 1 : 0);
}

static void cmd_destroy_canvas(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    if (argc != 2) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    struct canvas* canvas = (struct canvas*)strtoul(tokens[1], NULL, 10);
    animate_destroy_canvas(canvas);
    snprintf(response, 256, "%d\n", RPC_SUCCESS);
}

static void cmd_set_animation_params(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    if (argc != 6) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    struct sprite_placement* placement = (struct sprite_placement*)strtoul(tokens[1], NULL, 10);
    ssize_t vx = atol(tokens[2]);
    ssize_t vy = atol(tokens[3]);
    ssize_t ax = atol(tokens[4]);
    ssize_t ay = atol(tokens[5]);
    
    animate_set_animation_params(placement, vx, vy, ax, ay);
    snprintf(response, 256, "%d\n", RPC_SUCCESS);
}
static void cmd_generate(client_session_t* client, char* tokens[], int argc, char* response) {
    if (!client->logged_in) {
        snprintf(response, 256, "Not logged in\n");
        return;
    }
    
    // generate <canvas> <filename> <start> <end> <frame_rate>
    if (argc != 6) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    struct canvas* canvas = (struct canvas*)strtoul(tokens[1], NULL, 10);
    const char* filename = tokens[2];
    int start_frame = atoi(tokens[3]);
    int end_frame = atoi(tokens[4]);
    int frame_rate = atoi(tokens[5]);
    
    if (canvas == NULL || start_frame < 0 || end_frame < start_frame || frame_rate <= 0) {
        snprintf(response, 256, "%d\n", RPC_VALUE_ERROR);
        return;
    }
    
    // Calculate number of frames
    int num_frames = end_frame - start_frame + 1;
    size_t frame_size = animate_frame_size_bytes(canvas);
    
    // Open .dat file
    char dat_file[512];
    snprintf(dat_file, sizeof(dat_file), "%s.dat", filename);
    FILE* dat_fp = fopen(dat_file, "wb");
    if (!dat_fp) {
        snprintf(response, 256, "0 -1\n");  // Data write failed
        return;
    }
    
    // Generate and write frames
    void* frame_buffer = malloc(frame_size);
    int data_write_success = 1;
    
    for (int frame = start_frame; frame <= end_frame; frame++) {
        animate_generate_frame(canvas, frame, frame_rate, frame_buffer);
        if (fwrite(frame_buffer, 1, frame_size, dat_fp) != frame_size) {
            data_write_success = 0;
            break;
        }
    }
    
    free(frame_buffer);
    fclose(dat_fp);
    
    if (!data_write_success) {
        snprintf(response, 256, "0 -1\n");  // Data write failed
        return;
    }
    
    // Use ffmpeg to create MP4
    char mp4_file[512];
    snprintf(mp4_file, sizeof(mp4_file), "%s.mp4", filename);
    char log_file[512];
    snprintf(log_file, sizeof(log_file), "%s.log", filename);
    
    char ffmpeg_cmd[1024];
    snprintf(ffmpeg_cmd, sizeof(ffmpeg_cmd),
             "ffmpeg -f rawvideo -pixel_format rgba -video_size %zux%zu "
             "-framerate %d -i %s -c:v libx264 -pix_fmt yuv420p %s 2> %s",
             canvas->width, canvas->height, frame_rate, dat_file, mp4_file, log_file);
    
    int ffmpeg_result = system(ffmpeg_cmd);
    
    if (ffmpeg_result != 0) {
        snprintf(response, 256, "0 0 -1\n");  // Movie write failed
        return;
    }
    
    snprintf(response, 256, "0 0 0\n");  // Success
    printf("Generated video: %s (%d frames)\n", mp4_file, num_frames);
}

static void cmd_disconnect(client_session_t* client, char* tokens[], int argc, char* response) {
    (void)tokens;
    (void)argc;
    client->logged_in = 0;
    snprintf(response, 256, "%d\n", RPC_SUCCESS);
    printf("User %s disconnected\n", client->username);
}

// Main RPC handler
void handle_rpc(client_session_t* client, const char* command, char* response, size_t response_size) {
    char* tokens[16];
    int argc = parse_args(command, tokens, 16);
    
    if (argc == 0) {
        snprintf(response, response_size, "%d\n", RPC_FAILED);
        return;
    }
    
    // Check if logged in for non-login, non-disconnect commands
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
    } else if (strcmp(tokens[0], "placement_up") == 0) {
        cmd_placement_up(client, tokens, argc, response);
    } else if (strcmp(tokens[0], "placement_down") == 0) {
        cmd_placement_down(client, tokens, argc, response);
    } else if (strcmp(tokens[0], "placement_top") == 0) {
        cmd_placement_top(client, tokens, argc, response);
    } else if (strcmp(tokens[0], "placement_bottom") == 0) {
        cmd_placement_bottom(client, tokens, argc, response);
    } else if (strcmp(tokens[0], "destroy_placement") == 0) {
        cmd_destroy_placement(client, tokens, argc, response);
    } else if (strcmp(tokens[0], "destroy_sprite") == 0) {
        cmd_destroy_sprite(client, tokens, argc, response);
    } else if (strcmp(tokens[0], "destroy_canvas") == 0) {
        cmd_destroy_canvas(client, tokens, argc, response);
    } else if (strcmp(tokens[0], "set_animation_params") == 0) {
        cmd_set_animation_params(client, tokens, argc, response);
    } else if (strcmp(tokens[0], "Disconnect") == 0) {
        cmd_disconnect(client, tokens, argc, response);
    } else if (strcmp(tokens[0], "Login") == 0) {
        cmd_login(client, tokens, argc, response);
    } else {
        snprintf(response, response_size, "%d\n", RPC_FAILED);
    }
    
    free_tokens(tokens, argc);
}
