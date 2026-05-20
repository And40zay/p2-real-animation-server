#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include "rpc_handler.h"

#define FIFO_BASE "/tmp/animate_fifos"
#define WELL_KNOWN_FIFO FIFO_BASE "/server_fifo"
#define MAX_CMD 1024
#define MAX_RESP 1024

static volatile int running = 1;

void handle_sigterm(int sig) {
    if (sig == SIGINT || sig == SIGTERM) running = 0;
}

void handle_sigusr1(int sig, siginfo_t *info, void *context) {
    (void)sig;
    (void)context;
    // CRITICAL: Reply with SIGUSR2 to the sender
    if (info->si_pid > 0) {
        kill(info->si_pid, SIGUSR2);
    }
}

int main() {
    // Print server PID (required)
    printf("Server PID: %d\n", getpid());
    fflush(stdout);
    
    // Set up SIGUSR1 handler
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_sigusr1;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    
    signal(SIGINT, handle_sigterm);
    signal(SIGTERM, handle_sigterm);
    
    mkdir(FIFO_BASE, 0755);
    unlink(WELL_KNOWN_FIFO);
    mkfifo(WELL_KNOWN_FIFO, 0666);
    
    while (running) {
        int fd = open(WELL_KNOWN_FIFO, O_RDONLY);
        if (fd == -1) continue;
        
        char buf[1024];
        int n = read(fd, buf, sizeof(buf)-1);
        close(fd);
        if (n <= 0) continue;
        buf[n] = '\0';
        
        char *pid_str = strtok(buf, ":");
        char *fifo_name = strtok(NULL, ":");
        char *cmd = strtok(NULL, "\n");
        if (!pid_str || !fifo_name || !cmd) continue;
        
        client_session_t client = {0};
        client.client_pid = atoi(pid_str);
        client.logged_in = 0;
        
        char response[MAX_RESP];
        handle_rpc(&client, cmd, response, sizeof(response));
        
        int resp_fd = open(fifo_name, O_WRONLY);
        if (resp_fd != -1) {
            write(resp_fd, response, strlen(response));
            close(resp_fd);
        }
    }
    
    unlink(WELL_KNOWN_FIFO);
    return 0;
}