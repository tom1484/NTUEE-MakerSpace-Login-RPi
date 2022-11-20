#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>

#include <iostream>
#include <string>
#include <ctime>

#define MAX_BUF_LEN 1024

void on_sigint(int signum);

void parse_header(
    char *buf, int *header_len,
    bool *to_unlock,
    bool *to_lock, int *lock_period);
void parse_frame_data(
    char *buf, int buf_len, int header_len,
    u_int8_t *ft_color, u_int8_t *bg_color,
    double *ft_height, int *thickness,
    char *text);

void parse_data(int buf_len);

int main_pipe[2];
char pipe_buf[MAX_BUF_LEN];

char logic_buf[MAX_BUF_LEN];
char clock_buf[MAX_BUF_LEN];

// void select_source();

int main(int argc, char *argv[])
{
    signal(SIGINT, on_sigint);

    main_pipe[0] = atoi(argv[1]);
    main_pipe[1] = atoi(argv[2]);

    fcntl(main_pipe[0], F_SETFL, O_NONBLOCK);

    int buf_len;
    while (1)
    {
        if ((buf_len = read(main_pipe[0], pipe_buf, MAX_BUF_LEN)) > 0)
            parse_data(buf_len);
    }

    return 0;
}

// void select_source()
// {
// }

void on_sigint(int signum)
{
    close(main_pipe[0]);
    close(main_pipe[1]);
}

void parse_data(int buf_len)
{
    int header_len;
    bool to_unlock;
    bool to_lock;
    int lock_period;
    parse_header(pipe_buf, &header_len, &to_unlock, &to_lock, &lock_period);

    u_int8_t ft_color[3];
    u_int8_t bg_color[3];
    double ft_height;
    int thickness;
    char msg[1024];
    parse_frame_data(pipe_buf, buf_len, header_len, ft_color, bg_color, &ft_height, &thickness, msg);

    printf("%s\n", msg);
}

void parse_header(
    char *buf, int *header_len,
    bool *to_unlock,
    bool *to_lock, int *lock_period)
{
    int sft = 0;

    memcpy(to_unlock, buf + sft, 1);
    sft += 1;

    memcpy(to_lock, buf + sft, 1);
    sft += 1;

    memcpy(lock_period, buf + sft, 4);
    sft += 4;

    *header_len = sft;
}

void parse_frame_data(
    char *buf, int buf_len, int header_len,
    u_int8_t *ft_color, u_int8_t *bg_color,
    double *ft_height, int *thickness,
    char *text)
{
    int sft = header_len;

    memcpy(ft_color, buf + sft, 3);
    sft += 3;
    memcpy(bg_color, buf + sft, 3);
    sft += 3;
    memcpy(ft_height, buf + sft, 8);
    sft += 8;
    memcpy(thickness, buf + sft, 4);
    sft += 4;

    memcpy(text, buf + sft, buf_len - sft);
}