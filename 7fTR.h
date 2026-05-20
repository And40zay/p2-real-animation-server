#ifndef RPC_HANDLER_H
#define RPC_HANDLER_H

#include "animate.h"

typedef struct client_session {
    int client_pid;
    char username[33];
    int logged_in;
    int balance;
    FILE* fifo_c2s;
    FILE* fifo_s2c;
    struct client_session* next;
} client_session_t;

// RPC response codes
#define RPC_SUCCESS 0
#define RPC_FAILED -1
#define RPC_VALUE_ERROR -2
#define RPC_INTERNAL_ERROR -3

// Parse and execute RPC command
void handle_rpc(client_session_t* client, const char* command);

#endif