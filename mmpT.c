#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_INPUT 512
#define MAX_RESPONSE 1024
#define FIFO_BASE "/tmp/animate_fifos"
#define WELL_KNOWN_FIFO FIFO_BASE "/server_fifo"

int main(int argc, char** argv, char** envp) {
    char input[MAX_INPUT];
    char response[MAX_RESPONSE];
    int logged_in = 0;
    
    printf("Animation Client Started\n");
    printf("First, login with: Login <username>\n");
    printf("Then try commands like:\n");
    printf("  create_rectangle 10 20 0xFF0000 1\n");
    printf("  create_canvas 100 100 0\n");
    printf("  place_sprite <canvas_handle> <sprite_handle> 10 10\n");
    printf("Type 'quit' to exit\n\n");
    
    while (1) {
        printf("%s> ", logged_in ? "" : "(not logged in) ");
        fflush(stdout);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "quit") == 0) {
            break;
        }
        
        // Check if logged in for non-login commands
        if (strncmp(input, "Login", 5) != 0 && !logged_in) {
            printf("Not logged in\n");
            continue;
        }
        
        // Connect to server
        int server_fd = open(WELL_KNOWN_FIFO, O_RDWR);
        if (server_fd == -1) {
            printf("Failed to connect to server. Is it running?\n");
            continue;
        }
        
        // Send command
        write(server_fd, input, strlen(input));
        write(server_fd, "\n", 1);
        
        // Read response
        ssize_t bytes_read = read(server_fd, response, sizeof(response) - 1);
        if (bytes_read > 0) {
            response[bytes_read] = '\0';
            printf("%s", response);
            
            // Check if login was successful
            if (strncmp(input, "Login", 5) == 0 && strncmp(response, "0", 1) == 0) {
                logged_in = 1;
            }
        }
        
        close(server_fd);
    }
    
    printf("Client exiting\n");
    return 0;
}