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
    struct client_session* next;
} client_session_t;

// Response codes
#define RPC_SUCCESS 0
#define RPC_FAILED -1
#define RPC_VALUE_ERROR -2
#define RPC_INTERNAL_ERROR -3

// Function to handle RPC command
void handle_rpc(client_session_t* client, const char* command, char* response, size_t response_size);

#endif