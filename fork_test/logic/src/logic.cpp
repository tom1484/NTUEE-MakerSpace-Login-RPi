#include <iostream>
#include <string>
#include <boost/filesystem.hpp>

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

namespace BFS = boost::filesystem;

#define CHILD_NUM 2

#define MAX_BUF_LEN 1024

enum Child
{
    Main = 0,
    Serial = 1,
};

void on_sigchld(int signum);
void on_sigint(int signum);

void respawn_children();
void child_routine(int child, int p_in, int p_out);

bool quitting = false;

pid_t child_pid[CHILD_NUM];
int child_pipe[CHILD_NUM][2];

void process_main();
void process_serial();

int main_pipe[2];
char pipe_buf[MAX_BUF_LEN];

BFS::path PATH;
// std::string PATH;

int main(int argc, char *argv[])
{
    signal(SIGCHLD, on_sigchld);
    signal(SIGINT, on_sigint);

    main_pipe[0] = atoi(argv[1]);
    main_pipe[1] = atoi(argv[2]);
    PATH = BFS::path(argv[3]);

    for (int i = 0; i < CHILD_NUM; i++)
        child_pid[i] = 0;

    while (1)
    {
        respawn_children();

        if (read(child_pipe[Child::Main][0], pipe_buf, MAX_BUF_LEN) > 0)
            process_main();
        if (read(child_pipe[Child::Serial][0], pipe_buf, MAX_BUF_LEN) > 0)
            process_serial();
    }

    return 0;
}

void process_main()
{
    // send frame data to frame via manager
}

void process_serial()
{
    if (child_pid[Child::Main] != 0)
        write(child_pipe[Child::Main][1], pipe_buf, MAX_BUF_LEN);
}

void respawn_children()
{
    int pipe_tmp[2][2];

    for (int i = 0; i < CHILD_NUM; i++)
    {
        if (child_pid[i] == 0)
        {
            pipe(pipe_tmp[0]);
            pipe(pipe_tmp[1]);

            child_pipe[i][0] = pipe_tmp[1][0];
            child_pipe[i][1] = pipe_tmp[0][1];

            fcntl(child_pipe[i][0], F_SETFL, O_NONBLOCK);

            if ((child_pid[i] = fork()) == 0)
            {
                close(pipe_tmp[0][1]);
                close(pipe_tmp[1][0]);
                child_routine(i, pipe_tmp[0][0], pipe_tmp[1][1]);
            }
            close(pipe_tmp[0][0]);
            close(pipe_tmp[1][1]);
        }
    }
}

void on_sigchld(int signum)
{
    pid_t pid;
    int status;

    if (!quitting)
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        {
            for (int i = 0; i < CHILD_NUM; i++)
                if (pid == child_pid[i])
                {
                    // printf("%d died\n", child_pid[i]);
                    close(child_pipe[i][0]);
                    close(child_pipe[i][1]);
                    child_pid[i] = 0;
                }
        }
}

void on_sigint(int signum)
{
    quitting = true;

    bool cleaned;
    while (1)
    {
        cleaned = true;
        for (int i = 0; i < CHILD_NUM; i++)
        {
            // printf("trying to kill (%d, %d)\n", i, child_pid[i]);
            if (child_pid[i] != 0)
            {
                if (kill(child_pid[i], SIGINT) < 0)
                {
                    cleaned = false;
                    // printf("killing child %d failed\n", child_pid[i]);
                }
                else
                {
                    // printf("child %d killed\n", child_pid[i]);
                    close(child_pipe[i][0]);
                    close(child_pipe[i][1]);
                    child_pid[i] = -1;
                }
            }
        }
        if (cleaned)
            break;
    }

    close(main_pipe[0]);
    close(main_pipe[1]);

    exit(0);
}

void child_routine(int child, int p_in, int p_out)
{
    std::string pipe_str[2];
    pipe_str[0] = std::to_string(p_in);
    pipe_str[1] = std::to_string(p_out);

    if (child == Child::Main)
    {
        BFS::path main_base_path = PATH;
        BFS::path main_bin = main_base_path / "build/main";
        const char *main_argv[] = {
            "main",
            pipe_str[0].c_str(),
            pipe_str[1].c_str(),
            main_base_path.c_str(),
            NULL,
        };
        execv(main_bin.c_str(), (char *const *)main_argv);
    }
    if (child == Child::Serial)
    {
        BFS::path serial_base_path = PATH;
        BFS::path serial_bin = serial_base_path / "build/serial";
        const char *serial_argv[] = {
            "serial",
            pipe_str[0].c_str(),
            pipe_str[1].c_str(),
            serial_base_path.c_str(),
            NULL,
        };
        execv(serial_bin.c_str(), (char *const *)serial_argv);
    }
}