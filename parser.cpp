
// #include <stdlib.h>
// #include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
// #include <cstring>
// #include <cstdlib>
// #include <sstream>
// #include <iostream>


struct http_request {
    std::string method;
    std::string URI;
    std::string version;
    std::vector<std::pair<std::string, std::string>> headers;
    std::string body;
};

struct http_response {
    std::string version;
    std::string status;
    std::vector<std::pair<std::string, std::string>> headers;
    std::string body;
};

// Parses the HTTP request and throws std::invalid_argument if malformed.
http_request parseRequest(std::string request, int len) {
    http_request out;
    
    // Split the request into lines
    size_t pos = 0;
    while (pos < request.size()) {
        size_t end = request.find("\r\n", pos);
        if (end == std::string::npos)
            break;
        std::string line = request.substr(pos, end - pos);
        // std::cout << line;
        pos = end + 2;  // Move past the "\r\n"

        // Check if it's the first line (request line)
        if (out.method.empty()) {
            // Parse the request line
            size_t methodEnd = line.find(' ');
            if (methodEnd == std::string::npos) {
                printf("method");
                throw std::invalid_argument("Invalid request line format");
            }
            out.method = line.substr(0, methodEnd);
            size_t pathEnd = line.find(' ', methodEnd + 1);
            if (pathEnd == std::string::npos) {
                printf("URI");
                throw std::invalid_argument("Invalid request line format");
            }
            out.URI = line.substr(methodEnd + 1, pathEnd - methodEnd - 1);
            out.version = line.substr(pathEnd + 1);
        } else {
            // Parse headers
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string headerName = line.substr(0, colonPos);
                std::string headerValue = line.substr(colonPos + 2);  // Skip ': ' after the colon
                out.headers.emplace_back(headerName, headerValue);
            } else {
                break; 
            }
        }
    }

    // Find the position of the double "\r\n" that separates headers from the body
    pos = request.find("\r\n\r\n");
    if (pos != std::string::npos) {
        // Extract the body if it exists
        out.body = request.substr(pos + 4);
    }

    return out;
}

// Turns the http_response into a string
std::string responseToString(http_response response) {
    std::string request_str;

    // Append version and status
    request_str += response.version + " " + response.status + "\r\n";

    // Append headers
    for (const auto& header : response.headers) {
        request_str += header.first + ": " + header.second + "\r\n";
    }

    // Append a blank line to indicate end of headers
    request_str += "\r\n";

    // Append body
    request_str += response.body;

    return request_str;
}

// Validates a http_request and returns 1 if it is valid.
// Currently only checks method and version
bool validateRequest(http_request request) {
    // std::string supported_methods[] = {"GET", "HEAD"};
    // std::string supported_versions[] = {"HTTP/0.9","HTTP/1.0","HTTP/1.1","HTTP/2", "HTTP/3"};
    if (request.method != "GET" && request.method != "HEAD") return 0;
    if (request.version != "HTTP/0.9" && request.version != "HTTP/1.0"
         && request.version != "HTTP/1.1" && request.version != "HTTP/2"
         && request.version != "HTTP/3") return 0;
    return 1;
}