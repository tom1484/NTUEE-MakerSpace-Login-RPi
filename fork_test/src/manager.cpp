#include <iostream>
#include <string>
#include <boost/filesystem.hpp>

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

namespace BFS = boost::filesystem;

#define CHILD_NUM 3

#define MAX_BUF_LEN 1024

enum Child
{
    Frame = 0,
    Logic = 1,
    Clock = 2,
};

void on_sigchld(int signum);
void on_sigint(int signum);

void respawn_children();
void child_routine(int child, int p_in, int p_out);

bool quitting = false;

pid_t child_pid[CHILD_NUM];
int child_pipe[CHILD_NUM][2];

void wrap_header(uint8_t indicator);
void process_logic();
void process_clock();

char pipe_buf[MAX_BUF_LEN];

BFS::path PATH = "/home/tom1484/Git/NTUEE-MakerSpace-Login-RPi/fork_test/";

int main()
{
    signal(SIGCHLD, on_sigchld);
    signal(SIGINT, on_sigint);

    for (int i = 0; i < CHILD_NUM; i++)
        child_pid[i] = 0;

    while (1)
    {
        respawn_children();

        if (read(child_pipe[Child::Logic][0], pipe_buf, MAX_BUF_LEN) > 0)
            process_logic();
        if (read(child_pipe[Child::Clock][0], pipe_buf, MAX_BUF_LEN) > 0)
            process_clock();
    }

    return 0;
}

void wrap_header(uint8_t indicator)
{
    memmove(pipe_buf + 1, pipe_buf, MAX_BUF_LEN - 1);
    memcpy(pipe_buf, (void *)(&indicator), 1);
}

void process_logic()
{
    // send frame data to frame
}

void process_clock()
{
    // wrap_header(1);
    if (child_pid[Child::Frame] != 0)
        write(child_pipe[Child::Frame][1], pipe_buf, MAX_BUF_LEN);
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

    exit(0);
}

void child_routine(int child, int p_in, int p_out)
{
    std::string pipe_str[2];
    pipe_str[0] = std::to_string(p_in);
    pipe_str[1] = std::to_string(p_out);

    if (child == Child::Frame)
    {
        BFS::path frame_base_path = PATH / "frame";
        BFS::path frame_bin = frame_base_path / "build/frame";
        const char *frame_argv[] = {
            "frame",
            pipe_str[0].c_str(),
            pipe_str[1].c_str(),
            frame_base_path.c_str(),
            NULL,
        };
        execv(frame_bin.c_str(), (char *const *)frame_argv);
    }
    if (child == Child::Logic)
    {
        BFS::path logic_base_path = PATH / "logic";
        BFS::path logic_bin = logic_base_path / "build/logic";
        const char *logic_argv[] = {
            "logic",
            pipe_str[0].c_str(),
            pipe_str[1].c_str(),
            logic_base_path.c_str(),
            NULL,
        };
        execv(logic_bin.c_str(), (char *const *)logic_argv);
    }
    if (child == Child::Clock)
    {
        BFS::path clock_base_path = PATH / "clock";
        BFS::path clock_bin = clock_base_path / "build/clock";
        const char *clock_argv[] = {
            "clock",
            pipe_str[0].c_str(),
            pipe_str[1].c_str(),
            clock_base_path.c_str(),
            NULL,
        };
        execv(clock_bin.c_str(), (char *const *)clock_argv);
    }
}