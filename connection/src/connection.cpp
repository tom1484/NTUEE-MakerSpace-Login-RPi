#include "connection.h"

void Connection::initialize(std::string _base_url)
{

    curlpp::initialize();

    // set base url string
    base_url = _base_url.back() != '/' ? _base_url + '/' : _base_url;

    // set HTTP header
    std::list<std::string> header = {
        "Content-Type: application/json",
        "accept: application/json"};
    request.setOpt(new curlpp::options::HttpHeader(header));

    // set request result data stream
    curlpp::options::WriteStream ws(&result_stream);
    request.setOpt(ws);
}

Json::Value Connection::send_update_request(std::string rfid, std::string timestamp)
{

    // set url route
    std::string register_url = base_url + "update";
    request.setOpt(curlpp::Options::Url(register_url));

    // set JSON data
    char query[100];
    const char format[100] =
        "{"
        "\"RFID\": \"%s\", "
        "\"timestamp\": \"%s\""
        "}";
    std::sprintf(query, format, rfid.c_str(), timestamp.c_str());
    request.setOpt(new curlpp::options::PostFields(std::string(query)));

    // send request and decode result
    request.perform();

    return parse_result();
}

Json::Value Connection::send_register_request(std::string rfid, std::string student_id, std::string timestamp)
{

    // set url route
    std::string register_url = base_url + "register";
    request.setOpt(curlpp::Options::Url(register_url));

    // set JSON data
    char query[100];
    const char format[100] =
        "{"
        "\"RFID\": \"%s\", "
        "\"studentID\": \"%s\", "
        "\"timestamp\": \"%s\""
        "}";
    std::sprintf(query, format, rfid.c_str(), student_id.c_str(), timestamp.c_str());
    request.setOpt(new curlpp::options::PostFields(std::string(query)));

    // send request and decode result
    request.perform();

    return parse_result();
}

Json::Value Connection::parse_result()
{

    std::string json_raw;

    Json::Value result_json;
    Json::CharReaderBuilder builder;

    builder["collectComments"] = true;
    JSONCPP_STRING errs;

    if (!parseFromStream(builder, result_stream, &result_json, &errs))
    {
        std::cout << errs << std::endl;
    }

    return result_json;
}
