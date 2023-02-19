#include "serial.h"

void Serial::scanner(
    int stream_id,
    char start_symbol, char stop_symbol,
    bool *to_terminate, bool *input_available,
    std::string *input)
{
    bool paused = true;

    char byteIn;
    std::string input_buf = "";

    while (!(*to_terminate))
    {
        if (serialDataAvail(stream_id) > 0)
        {
            byteIn = serialGetchar(stream_id);
            if (!paused)
            {
                if (byteIn == stop_symbol)
                {
                    paused = true;
                    *input = input_buf;
                    *input_available = true;
                }
                else
                {
                    input_buf += byteIn;
                }
            }
            else if (!(*input_available))
            {
                if (byteIn == start_symbol)
                {
                    input_buf = "";
                    paused = false;
                }
            }
        }
    }
    return;
}

void Serial::open(std::string serialDevice, int boudRate)
{
    stream_id = serialOpen(serialDevice.c_str(), boudRate);
}

void Serial::activate(char _startSymbol, char _stopSymbol)
{
    start_symbol = _startSymbol;
    stop_symbol = _stopSymbol;

    to_terminate = false;

    input_available = false;
    input = "";

    scanner_thread = std::thread(
        &Serial::scanner, this,
        stream_id, start_symbol, stop_symbol,
        &to_terminate, &input_available,
        &input);
}

void Serial::terminate()
{
    to_terminate = true;

    scanner_thread.join();
    serialClose(stream_id);
}

bool Serial::available()
{
    return input_available;
}

std::string Serial::read()
{
    if (input_available)
    {
        input_available = false;
        return input;
    }
    else
        return "";
}
