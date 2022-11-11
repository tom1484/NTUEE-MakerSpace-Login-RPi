#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <string>
#include <string.h>

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <string>
#include <ctime>
#include <functional>
#include <csignal>

#ifdef OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/freetype.hpp>
#endif

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
void show_text(
    char *text,
    u_int8_t *ft_color, u_int8_t *bg_color,
    double ft_height, int thickness);

void timer(std::function<void()> task, int id, int peroid);

#ifdef OPENCV
std::ofstream ofs;
int width, height;
cv::Ptr<cv::freetype::FreeType2> ft2;
#endif

bool frame_locked;
int timer_id;
std::thread timer_thread;

#ifdef OPENCV
void initialize(int _fb, int _width, int _height)
{

    // set frame dimension
    width = _width;
    height = _height;

    // set framebuffer stream
    std::string fbDevice = "/dev/fb";
    fbDevice += std::to_string(_fb);
    ofs.open(fbDevice.c_str());

    ft2 = cv::freetype::createFreeType2();
    ft2->loadFontData("./FiraCode.ttf", 0);

    timer_id = 0;
    frame_locked = false;
}
#endif

std::thread clock_listener_thread;
std::thread message_listener_thread;

int clock_pipe_id;
int message_pipe_id;

bool clock_listener_thread_end;
bool message_listener_thread_end;

void pipe_listener(const char *pipe_path, bool *thread_end);

int main()
{
    signal(SIGINT, on_sigint);
#ifdef OPENCV
    initialize(0, 1920, 1080);
#endif
    clock_listener_thread_end = false;
    clock_listener_thread = std::thread(
        pipe_listener, "/tmp/clock_pipe", &clock_listener_thread_end);
    usleep(1000);

    message_listener_thread_end = false;
    message_listener_thread = std::thread(
        pipe_listener, "/tmp/message_pipe", &message_listener_thread_end);
    usleep(1000);

    while (true)
    {
    }

    return 0;
}

void on_sigint(int signum)
{
    std::cout << "Program interrupted by signal " << signum << std::endl;
    std::cout << "Closing listeners..." << std::endl;

    clock_listener_thread_end = true;
    clock_listener_thread.join();

    message_listener_thread_end = true;
    message_listener_thread.join();

    exit(1);
}

void pipe_listener(const char *pipe_path, bool *thread_end)
{
    int pipe_id = -1;
    char buf[1024];
    int buf_len;

    while (mkfifo(pipe_path, 0666) < 0 && errno != EEXIST)
    {
    }
    std::cout << "Pipe " << pipe_path << " created" << std::endl;

    while (!(*thread_end) && (pipe_id = open(pipe_path, O_RDONLY | O_NONBLOCK)) < 0)
    {
    }
    if (pipe_id > 0)
    {
        std::cout << "Pipe " << pipe_path << " opened" << std::endl;

        while (!(*thread_end))
        {
            if ((buf_len = read(pipe_id, buf, 1024)) > 0)
            {
                int header_len;
                bool to_unlock;
                bool to_lock;
                int lock_period;
                parse_header(buf, &header_len, &to_unlock, &to_lock, &lock_period);

                if (to_unlock || !frame_locked)
                {
                    u_int8_t ft_color[3];
                    u_int8_t bg_color[3];
                    double ft_height;
                    int thickness;
                    char msg[1024];
                    parse_frame_data(buf, buf_len, header_len, ft_color, bg_color, &ft_height, &thickness, msg);
                    show_text(msg, ft_color, bg_color, ft_height, thickness);

                    if (to_unlock && frame_locked)
                    {
                        ++timer_id;
                        frame_locked = false;
                    }

                    if (to_lock)
                    {
                        timer_thread = std::thread(
                            timer,
                            []()
                            {
                                frame_locked = false;
                            },
                            ++timer_id, lock_period);
                        timer_thread.detach();
                        frame_locked = true;
                    }
                }
            }
        }
    }

    close(pipe_id);
    std::cout << "Pipe " << pipe_path << " interrupted" << std::endl;

    return;
}

void timer(std::function<void()> task, int id, int period)
{
    auto startClock = std::chrono::high_resolution_clock::now();
    while (true)
    {
        auto elapsedClock = std::chrono::high_resolution_clock::now() - startClock;
        auto elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedClock).count();
        if (elapsedMillis >= period)
        {
            if (id == timer_id)
                task();
            return;
        }
    }
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

void show_text(char *text, u_int8_t *ft_color, u_int8_t *bg_color, double ft_height, int thickness)
{
#ifndef OPENCV
    printf("%s\n", text);
#endif
#ifdef OPENCV
    std::string text_str = std::string(text);
    cv::Scalar ft_color_cv = cv::Scalar(ft_color[0], ft_color[1], ft_color[2]);
    cv::Scalar bg_color_cv = cv::Scalar(bg_color[0], bg_color[1], bg_color[2]);

    cv::Mat image = cv::Mat::zeros(cv::Size(width, height), CV_8UC3);
    cv::Mat framebuffer_compat;

    // set background
    image.setTo(bg_color_cv);

    std::vector<std::string> lines(0);
    for (int i = 0, p = 0; i <= text_str.length(); i++)
    {
        if (i == text_str.length() || text_str[i] == '\n')
        {
            lines.push_back(text_str.substr(p, i - p));
            p = i + 1;
        }
    }

    for (int i = 0; i < lines.size(); i++)
    {
        std::string line = lines[i];
        int baseline = 0;

        // get text size
        cv::Size text_size = ft2->getTextSize(line, ft_height, thickness, &baseline);

        cv::Point origin;
        origin.x = image.cols / 2 - text_size.width / 2;
        origin.y = image.rows / 2 + text_size.height / 2 + int((text_size.height + 40) * (i - (lines.size() - 1) / 2.0));

        ft2->putText(image, line, origin, ft_height, ft_color_cv, thickness, 8, true);
    }

    cv::cvtColor(image, framebuffer_compat, cv::COLOR_RGB2BGR565);
    for (int y = 0; y < height; y++)
    {
        ofs.seekp(y * width * 2);
        ofs.write(reinterpret_cast<char *>(framebuffer_compat.ptr(y)), width * 2);
    }
#endif
}
