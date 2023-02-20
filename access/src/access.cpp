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

#include "serial.h"
#include "connection.h"

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

const char *SERIAL_DEV = "/dev/ttyUSB0";
const int BAUD_RATE = 9600;
const char START_SYMBOL = '\t';
const char STOP_SYMBOL = '\n';

std::string get_timestamp_str();
bool parse_input(std::string input, int *type, std::string *rfid, std::string *student_id);

const std::string BASE_URL = "140.112.194.49:1484/mks_access";

const char *pipe_path = "/tmp/message_pipe";
int pipe_id;

Serial serial;
Connection connection;

int main()
{
    signal(SIGINT, on_sigint);
    signal(SIGPIPE, on_sigpipe);

    while ((pipe_id = open(pipe_path, O_WRONLY | O_NONBLOCK)) < 0)
    {
    }
    std::cout << "Pipe " << pipe_path << " opened" << std::endl;

    serial.open(SERIAL_DEV, BAUD_RATE);
    serial.activate(START_SYMBOL, STOP_SYMBOL);

    connection.initialize(BASE_URL);

    int type;
    std::string rfid;
    std::string student_id;

    bool registering = false;
    while (true)
    {
        if (serial.available())
        {
            std::string serial_input = serial.read();
            if (parse_input(serial_input, &type, &rfid, &student_id))
            {
                std::string timestamp = get_timestamp_str();
                std::cout << serial_input << std::endl;

                if (type == 0)
                {
                    Json::Value result = connection.send_update_request(rfid, timestamp);
                    bool flag = result["flag"].asBool();
                    if (flag)
                    {
                        std::string display_name = result["personalInfo"]["displayName"].asString();
                        std::string welcome_message = display_name;
                        std::cout << welcome_message << std::endl;

                        registering = false;
                        show_text(true, true, 3000, COLOR_BLACK, COLOR_GREEN, FT_HEIGHT, THICKNESS, welcome_message);
                    }
                    else
                    {
                        std::string require_register_message = "Please\nregister!";
                        std::cout << require_register_message << std::endl;

                        registering = true;
                        show_text(true, true, 5000, COLOR_BLACK, COLOR_RED, FT_HEIGHT, THICKNESS, require_register_message);
                    }
                }
                else
                {
                    if (registering)
                    {
                        Json::Value result = connection.send_register_request(rfid, student_id, timestamp);
                        bool flag = result["flag"].asBool();
                        if (flag)
                        {
                            std::string display_name = result["personalInfo"]["displayName"].asString();
                            std::string welcome_message = display_name;
                            std::cout << welcome_message << std::endl;

                            show_text(true, true, 3000, COLOR_BLACK, COLOR_GREEN, FT_HEIGHT, THICKNESS, welcome_message);
                        }
                        else
                        {
                            std::string register_failure_message = "Registeration\nfailed!";
                            std::cout << register_failure_message << std::endl;

                            show_text(true, true, 3000, COLOR_BLACK, COLOR_GREEN, FT_HEIGHT, THICKNESS, register_failure_message);
                        }
                    }
                }
            }
        }
    }

    close(pipe_id);

    return 0;
}

void on_sigint(int signum)
{
    std::cout << "Closing pipe..." << std::endl;
    close(pipe_id);

    std::cout << "Closing serial..." << std::endl;
    serial.terminate();

    char message[100];
    sprintf(message, "Program interrupted by signal %d", signum);
    std::cout << message << std::endl;
    exit(1);
}

void on_sigpipe(int signum)
{
    return;
}

bool parse_input(std::string input, int *type, std::string *rfid, std::string *student_id)
{
    int _type;
    if (input.length() >= 3 && input[0] == '0')
    {
        _type = 0;
        std::string _rfid = input.substr(2);
        if (_rfid.length() == 8)
        {
            *type = _type;
            *rfid = _rfid;
            return true;
        }
    }
    else if (input.length() >= 3 && input[0] == '1')
    {
        _type = 1;
        std::string _student_id = input.substr(2);
        if (_student_id.length() == 9)
        {
            *type = _type;
            *student_id = _student_id;
            return true;
        }
    }
    return false;
}

std::string get_timestamp_str()
{
    std::time_t t_curr_time = std::time(0);
    t_curr_time += 8 * (60 * 60);
    std::tm curr_time = *std::localtime(&t_curr_time);

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
    result += year + "-" + mon + "-" + mday + " ";
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

    if (write(pipe_id, frame_data_buf, n + 1) < 0)
    {
        close(pipe_id);
        perror("Write failed ");
        exit(1);
    }
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
