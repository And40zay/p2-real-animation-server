#ifndef RPC_HANDLER_H
#define RPC_HANDLER_H

#include <stdint.h>
#include <stddef.h>
#include "animate.h"

// Client session structure
typedef struct client_session {
    int client_pid;
    char username[33];
    int logged_in;
    int balance;
    char fifo_c2s[256];  // Client to server FIFO name
    char fifo_s2c[256];  // Server to client FIFO name
    struct client_session* next;
} client_session_t;

// Response codes
#define RPC_SUCCESS 0
#define RPC_FAILED -1
#define RPC_VALUE_ERROR -2
#define RPC_INTERNAL_ERROR -3

// Function to handle RPC command
void handle_rpc(client_session_t* client, const char* command, char* response, size_t response_size);

// Function to check login
int check_login(const char* username, int* balance);

#endif