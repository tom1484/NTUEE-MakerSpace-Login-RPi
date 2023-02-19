#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>

#include <iostream>
#include <string>
#include <ctime>
#include <csignal>
#include <thread>
#include <exception>

const u_int8_t COLOR_WHITE[3] = {255, 255, 255};
const u_int8_t COLOR_BLACK[3] = {0, 0, 0};
const u_int8_t COLOR_RED[3] = {255, 0, 0};
const u_int8_t COLOR_GREEN[3] = {0, 255, 0};
const u_int8_t COLOR_BLUE[3] = {0, 0, 255};

const double FT_HEIGHT = 75;
const int THICKNESS = -1;

void on_sigint(int signum);
void on_sigpipe(int signum);

int wrap_header(
    char *buf,
    bool to_unlock,
    bool to_lock, int lock_period);
int wrap_frame_data(
    char *buf, size_t header_len,
    const u_int8_t *ft_color, const u_int8_t *bg_color,
    double ft_height, int thickness,
    const char *text, size_t text_len);

void show_text(
    bool to_unlock,
    bool to_lock, int lock_period,
    const u_int8_t *ft_color, const u_int8_t *bg_color,
    double ft_height, int thickness,
    std::string text);

const char *pipe_path = "/tmp/clock_pipe";
int pipe_id;

bool update_curr_time(std::tm *curr_time);
std::string get_time_str(std::tm curr_time);

int main()
{
    signal(SIGINT, on_sigint);
    signal(SIGPIPE, on_sigpipe);
    // printf("I am %d process.\n", getpid());

    while((pipe_id = open(pipe_path, O_WRONLY | O_NONBLOCK)) < 0)
    {
    }
    std::cout << "Pipe " << pipe_path << " opened" << std::endl;

    std::tm curr_time;
    while (true)
    {
        if (update_curr_time(&curr_time))
        {
            std::string curr_time_str = get_time_str(curr_time);
            show_text(false, false, 0, COLOR_WHITE, COLOR_BLACK, FT_HEIGHT, THICKNESS, curr_time_str);
            std::cout << curr_time_str << std::endl;
        }
        usleep(1000);
    }

    close(pipe_id);

    return 0;
}

void on_sigint(int signum)
{
    std::cout << "Closing pipe..." << std::endl;
    close(pipe_id);

    char message[100];
    sprintf(message, "Program interrupted by signal %d", signum);
    std::cout << message << std::endl;
    exit(1);
}

void on_sigpipe(int signum)
{
    return;
}

bool update_curr_time(std::tm *curr_time)
{
    std::time_t t_new_time = std::time(0);
    t_new_time += 8 * (60 * 60);
    std::tm new_time = *std::localtime(&t_new_time);

    if (curr_time->tm_sec != new_time.tm_sec)
    {
        *curr_time = new_time;
        return true;
    }

    return false;
}

std::string get_time_str(std::tm curr_time)
{
    std::string year = std::to_string(curr_time.tm_year + 1900);
    std::string mon = std::to_string(curr_time.tm_mon + 1);
    std::string mday = std::to_string(curr_time.tm_mday);
    std::string hour = std::to_string(curr_time.tm_hour);
    std::string min = std::to_string(curr_time.tm_min);
    std::string sec = std::to_string(curr_time.tm_sec);

    mon = std::string(mon.length() == 1 ? "0" : "") + mon;
    mday = std::string(mday.length() == 1 ? "0" : "") + mday;
    hour = std::string(hour.length() == 1 ? "0" : "") + hour;
    min = std::string(min.length() == 1 ? "0" : "") + min;
    sec = std::string(sec.length() == 1 ? "0" : "") + sec;

    std::string result = "";
    // result += year + "-" + mon + "-" + mday + "\n";
    result += hour + ":" + min + ":" + sec;

    return result;
}

void show_text(
    bool to_unlock,
    bool to_lock, int lock_period,
    const u_int8_t *ft_color, const u_int8_t *bg_color,
    double ft_height, int thickness,
    std::string text)
{
    char frame_data_buf[1024];
    int header_len = wrap_header(frame_data_buf, to_unlock > 0, to_lock > 0, lock_period);
    int n = wrap_frame_data(frame_data_buf, header_len, ft_color, bg_color, ft_height, thickness, text.c_str(), text.length());

    write(pipe_id, frame_data_buf, n + 1);
}

int wrap_header(
    char *buf,
    bool to_unlock,
    bool to_lock, int lock_period)
{
    int sft = 0;

    memcpy(buf + sft, (void *)(&to_unlock), 1);
    sft += 1;
    memcpy(buf + sft, (void *)(&to_lock), 1);
    sft += 1;
    memcpy(buf + sft, (void *)(&lock_period), 4);
    sft += 4;

    return sft;
}

int wrap_frame_data(
    char *buf, size_t header_len,
    const u_int8_t *ft_color, const u_int8_t *bg_color,
    double ft_height, int thickness,
    const char *text, size_t text_len)
{
    size_t sft = header_len;

    memcpy(buf + sft, ft_color, 3);
    sft += 3;
    memcpy(buf + sft, bg_color, 3);
    sft += 3;
    memcpy(buf + sft, (void *)(&ft_height), 8);
    sft += 8;
    memcpy(buf + sft, (void *)(&thickness), 4);
    sft += 4;
    memcpy(buf + sft, text, text_len);
    sft += text_len;

    buf[sft] = '\0';

    return sft;
}