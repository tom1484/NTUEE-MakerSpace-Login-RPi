#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

void on_sigint(int signum);

pid_t pid;

int main_pipe[2];

int main(int argc, char *argv[])
{
    signal(SIGINT, on_sigint);

    pid = getpid();
    // printf("serial opened: %d\n", pid);

    main_pipe[0] = atoi(argv[1]);
    main_pipe[1] = atoi(argv[2]);

    while (1)
    {
        // read serial data and send to main
    }

    return 0;
}

void on_sigint(int signum)
{
    close(main_pipe[0]);
    close(main_pipe[1]);
    exit(0);
}