#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define FIFO_PATH "/tmp/animate_fifos/server_fifo"
#define MAX_CMD 512
#define MAX_RESP 1024

int main() {
    char input[MAX_CMD];
    char response[MAX_RESP];
    
    printf("Animation Client\n");
    printf("Commands: Login <user>, create_rectangle <w> <h> <color> <filled>\n");
    printf("Type 'quit' to exit\n\n");
    
    while (1) {
        printf("> ");
        fflush(stdout);
        
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "quit") == 0) break;
        
        // Open FIFO
        int fd = open(FIFO_PATH, O_RDWR);
        if (fd == -1) {
            printf("Cannot connect to server. Is it running?\n");
            continue;
        }
        
        // Send command
        write(fd, input, strlen(input));
        write(fd, "\n", 1);
        
        // Read response
        int n = read(fd, response, sizeof(response) - 1);
        if (n > 0) {
            response[n] = '\0';
            printf("%s", response);
        }
        
        close(fd);
    }
    
    return 0;
}