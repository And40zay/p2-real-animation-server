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
    char* client_response_fifo = (char*)arg;
    
    // Open the well-known FIFO to read the command
    int server_fd = open(WELL_KNOWN_FIFO, O_RDONLY);
    if (server_fd == -1) {
        free(client_response_fifo);
        return;
    }
    
    char command[MAX_COMMAND];
    ssize_t bytes_read = read(server_fd, command, sizeof(command) - 1);
    close(server_fd);
    
    if (bytes_read > 0) {
        command[bytes_read] = '\0';
        
        // Remove newline
        char* newline = strchr(command, '\n');
        if (newline) *newline = '\0';
        
        printf("Received command: %s\n", command);
        
        // Create temporary client session
        client_session_t client = {
            .client_pid = getpid(),
            .logged_in = 0
        };
        
        char response[MAX_RESPONSE];
        handle_rpc(&client, command, response, MAX_RESPONSE);
        
        // Send response back through client's FIFO
        int resp_fd = open(client_response_fifo, O_WRONLY);
        if (resp_fd != -1) {
            write(resp_fd, response, strlen(response));
            close(resp_fd);
        }
    }
    
    free(client_response_fifo);
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
    // Main loop - accept connections
while (running) {
    // Wait for a client to create their response FIFO and send it via command line?
    // For now, we'll use a different approach - clients send their PID and we create response FIFO
    
    printf("Waiting for client...\n");
    
    // Open well-known FIFO for reading
    int server_fd = open(WELL_KNOWN_FIFO, O_RDONLY);
    if (server_fd == -1) {
        if (running) {
            perror("Failed to open well-known FIFO");
        }
        continue;
    }
    
    char command[MAX_COMMAND];
    ssize_t bytes_read = read(server_fd, command, sizeof(command) - 1);
    close(server_fd);
    
    if (bytes_read > 0) {
        command[bytes_read] = '\0';
        
        // Remove newline
        char* newline = strchr(command, '\n');
        if (newline) *newline = '\0';
        
        printf("Received command: %s\n", command);
        
        // For now, just send a simple response back through well-known FIFO
        // This is simplified - real implementation would use per-client FIFOs
        
        // Parse client PID from command (simplified - in reality, client would send PID first)
        char response_fifo[256];
        snprintf(response_fifo, sizeof(response_fifo), "%s/client_%d", FIFO_BASE, getpid());
        
        client_session_t client = {
            .client_pid = getpid(),
            .logged_in = 0
        };
        
        char response[MAX_RESPONSE];
        handle_rpc(&client, command, response, MAX_RESPONSE);
        
        // Try to send response
        int resp_fd = open(response_fifo, O_WRONLY);
        if (resp_fd != -1) {
            write(resp_fd, response, strlen(response));
            close(resp_fd);
        }
    }
}
    
    // Cleanup
    threadpool_destroy(pool);
    unlink(WELL_KNOWN_FIFO);
    
    printf("Server shutdown complete\n");
    return 0;
}