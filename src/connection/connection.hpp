#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <sstream>

#include <jsoncpp/json/json.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

class Connection
{
public:
    void initialize(std::string _base_url);

    Json::Value send_update_request(std::string rfid, std::string timestamp);
    Json::Value send_register_request(std::string rfid, std::string student_id, std::string timestamp);

private:
    curlpp::Easy request;

    std::string base_url;
    std::stringstream result_stream;

    Json::Value parse_result();
};

#endif
