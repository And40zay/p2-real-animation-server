#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include "rpc_handler.h"

#define FIFO_PATH "/tmp/animate_fifos/server_fifo"
#define MAX_CMD 512
#define MAX_RESP 1024

static volatile int running = 1;

void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        running = 0;
    }
}

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);
    
    char command[MAX_CMD];
    int n = read(client_fd, command, sizeof(command) - 1);
    
    if (n > 0) {
        command[n] = '\0';
        // Remove newline
        char* nl = strchr(command, '\n');
        if (nl) *nl = '\0';
        
        printf("Received: %s\n", command);
        
        client_session_t client = {0};
        client.logged_in = 0;
        
        char response[MAX_RESP];
        handle_rpc(&client, command, response, sizeof(response));
        
        write(client_fd, response, strlen(response));
    }
    
    close(client_fd);
    return NULL;
}

int main() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    
    mkdir("/tmp/animate_fifos", 0755);
    unlink(FIFO_PATH);
    
    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("mkfifo");
        return 1;
    }
    
    printf("Server running on %s\n", FIFO_PATH);
    
    while (running) {
        printf("Waiting for client...\n");
        
        int client_fd = open(FIFO_PATH, O_RDWR);
        if (client_fd == -1) {
            if (running) perror("open");
            continue;
        }
        
        int* fd_ptr = malloc(sizeof(int));
        *fd_ptr = client_fd;
        
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, fd_ptr);
        pthread_detach(thread);
    }
    
    unlink(FIFO_PATH);
    printf("Server stopped\n");
    return 0;
}