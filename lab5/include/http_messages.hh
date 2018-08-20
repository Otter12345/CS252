#ifndef INCLUDE_HTTP_MESSAGES_HH_
#define INCLUDE_HTTP_MESSAGES_HH_

#include <map>
#include <string>
#include <iostream>
#include <sstream>

class HttpRequest {
 public:
    std::string method;
    std::string request_uri;
    std::string http_version;
    std::map<std::string, std::string> headers;
    std::string message_body;
    std::string query;  // implemented in Task 2.2

    void print() const;
};



class HttpResponse {
 public:
    std::string http_version;
    int status_code;
    std::string reason_phrase;
    std::map<std::string, std::string> headers;
    std::string message_body;

    std::string to_string() const;

    void print() const {
        std::cout << to_string() << std::endl;
    }
};

#endif  // INCLUDE_HTTP_MESSAGES_HH_
