#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <string>
#include <vector>

// method, URI, version, headers, body
struct http_request {
    std::string method;
    std::string URI;
    std::string version;
    std::vector<std::pair<std::string, std::string>> headers;
    std::string body;
};

// version, status+reason, headers, body
struct http_response {
    std::string version;
    std::string status; // status and reason together
    std::vector<std::pair<std::string, std::string>> headers;
    std::string body;
};

// Parses the HTTP request and throws std::invalid_argument if malformed.
http_request parseRequest(std::string request, int len);

// Turns the http_response into a string
std::string responseToString(http_response response);

// Validates a http_request and returns 1 if it is valid.
bool validateRequest(http_request request);

#endif