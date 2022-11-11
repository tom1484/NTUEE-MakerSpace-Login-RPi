#ifndef SERIAL_HPP_
#define SERIAL_HPP_

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringSerial.h>

#include <iostream>
#include <string>
#include <thread>

class Serial
{
public:
    void open(std::string serialDevice, int boudRate);
    void activate(char _startSymbol, char _stopSymbol);
    void terminate();
    bool available();
    std::string read();

private:
    char start_symbol;
    char stop_symbol;

    int stream_id;
    std::thread scanner_thread;

    bool to_terminate;

    bool input_available;
    std::string input;

    void scanner(
        int stream_id,
        char start_symbol, char stop_symbol,
        bool *to_terminate, bool *input_available,
        std::string *input);
};

#endif