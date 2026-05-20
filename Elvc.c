#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "animate.h"
#include "rpc_handler.h"
#include "threadpool.h"

#define MAX_CLIENTS 100
#define THREAD_POOL_SIZE 4

// Global server state
static threadpool_t* pool = NULL;
static client_session_t* clients = NULL;

int main(int argc, char** argv, char** envp) {
    // Initialize threadpool
    pool = threadpool_create(THREAD_POOL_SIZE);
    
    // Create FIFO for incoming connections (well-known name)
    // Or accept command-line argument for FIFO name
    
    printf("Animation server started\n");
    
    // Main loop: accept client connections
    // For each client, create new FIFOs and spawn handler thread
    
    pthread_exit(NULL);
}