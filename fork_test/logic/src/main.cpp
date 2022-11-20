#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define MAX_BUF_LEN 1024

void on_sigint(int signum);

pid_t pid;

int main_pipe[2];
char pipe_buf[MAX_BUF_LEN];

int main(int argc, char *argv[])
{
    signal(SIGINT, on_sigint);

    pid = getpid();
    // printf("main opened: %d\n", pid);

    main_pipe[0] = atoi(argv[1]);
    main_pipe[1] = atoi(argv[2]);

    while (1)
    {
        if (read(main_pipe[0], pipe_buf, MAX_BUF_LEN) > 0)
        {
            // handle serial input
            // printf("%s\n", pipe_buf);
        }
    }

    return 0;
}

void on_sigint(int signum)
{
    close(main_pipe[0]);
    close(main_pipe[1]);
    exit(0);
}