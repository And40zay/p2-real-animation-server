cd /home/jake/Desktop/Systems\ Programming/P2/p2-scaffold

cat > test_rpc.c << 'EOF'
#include <stdio.h>
#include <string.h>
#include "rpc_handler.h"

int main() {
    client_session_t client = {0};
    client.logged_in = 0;
    
    char response[1024];
    
    printf("Testing RPC handler directly (no FIFO)\n\n");
    
    // Test login
    printf("Test 1: Login alice\n");
    handle_rpc(&client, "Login alice", response, sizeof(response));
    printf("Response: %s\n", response);
    
    // Test create rectangle
    printf("\nTest 2: create_rectangle 10 20 0xFF0000 1\n");
    handle_rpc(&client, "create_rectangle 10 20 0xFF0000 1", response, sizeof(response));
    printf("Response: %s\n", response);
    
    // Test create canvas
    printf("\nTest 3: create_canvas 100 100 0\n");
    handle_rpc(&client, "create_canvas 100 100 0", response, sizeof(response));
    printf("Response: %s\n", response);
    
    return 0;
}
EOF