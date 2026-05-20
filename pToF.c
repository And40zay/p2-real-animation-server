#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include "rpc_handler.h"
#include "threadpool.h"

#define MAX_RESPONSE 1024
#define MAX_COMMAND 512
#define FIFO_BASE "/tmp/animate_fifos"
#define WELL_KNOWN_FIFO FIFO_BASE "/server_fifo"

static volatile int running = 1;
static threadpool_t* pool = NULL;

void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("\nShutting down server...\n");
        running = 0;
    }
}

void handle_client_request(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);
    
    char command[MAX_COMMAND];
    ssize_t bytes_read = read(client_fd, command, sizeof(command) - 1);
    
    if (bytes_read > 0) {
        command[bytes_read] = '\0';
        
        // Remove newline
        char* newline = strchr(command, '\n');
        if (newline) *newline = '\0';
        
        printf("Received command: %s\n", command);
        
        // For now, create a temporary client session
        client_session_t client = {
            .client_pid = getpid(),
            .logged_in = 0
        };
        
        char response[MAX_RESPONSE];
        handle_rpc(&client, command, response, MAX_RESPONSE);
        
        write(client_fd, response, strlen(response));
    }
    
    close(client_fd);
}

int main(int argc, char** argv, char** envp) {
    printf("Animation Server Starting...\n");
    
    // Setup signal handling
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    // Create FIFO directory
    mkdir(FIFO_BASE, 0755);
    
    // Create well-known FIFO for new connections
    unlink(WELL_KNOWN_FIFO);
    if (mkfifo(WELL_KNOWN_FIFO, 0666) == -1) {
        perror("Failed to create well-known FIFO");
        return 1;
    }
    
    // Create threadpool
    pool = threadpool_create(4);
    if (!pool) {
        fprintf(stderr, "Failed to create threadpool\n");
        return 1;
    }
    
    printf("Server listening on %s\n", WELL_KNOWN_FIFO);
    
    // Main loop - accept connections
    while (running) {
        int* client_fd = malloc(sizeof(int));
        *client_fd = open(WELL_KNOWN_FIFO, O_RDWR);
        
        if (*client_fd == -1) {
            if (running) {
                perror("Failed to open well-known FIFO");
            }
            free(client_fd);
            continue;
        }
        
        // Add to threadpool
        threadpool_add_task(pool, handle_client_request, client_fd);
    }
    
    // Cleanup
    threadpool_destroy(pool);
    unlink(WELL_KNOWN_FIFO);
    
    printf("Server shutdown complete\n");
    return 0;
}