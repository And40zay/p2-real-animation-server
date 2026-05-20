#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT 256
#define MAX_RESPONSE 1024

int main(int argc, char** argv, char** envp) {
    char input[MAX_INPUT];
    char response[MAX_RESPONSE];
    
    printf("Animation Client Started\n");
    printf("Type commands (e.g., 'create_rectangle 10 20 0xFF0000 1')\n");
    printf("Type 'quit' to exit\n\n");
    
    while (1) {
        printf("> ");
        fflush(stdout);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "quit") == 0) {
            break;
        }
        
        // For now, just echo the command (will connect to server later)
        printf("Command received: %s\n", input);
        printf("(Client-server communication not yet implemented)\n\n");
    }
    
    printf("Client exiting\n");
    return 0;
}