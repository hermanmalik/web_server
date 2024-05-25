
#include <iostream>
#include <string>
#include <fstream>
#include "parser.h"

// this should take in an http_request and return the response
http_response handle_request(http_request request) {
    std::string status;
    std::string body;
    std::string version = "HTTP/1.0";

    
    // if invalid request
    if (validateRequest(request) == 0) {
        status = "400 Bad Request";
        body = "<!DOCTYPE html><html lang=\"en\"><body><h1> 400 Bad Request </h1></body></html>";
    }

    // GET support
    else if (request.method == "GET") {  
        std::string processedURL = parseURI(request.URI);
        // if empty, return 404
        if (processedURL.empty()) {
            status = "404 Not Found";
            // todo check if 404 file exists
            // if (GetFileAttributesA("root\\404.html") != INVALID_FILE_ATTRIBUTES) {
            //     return "root\\404.html";
            // }
            body = "<!DOCTYPE html><html lang=\"en\"><body><h1> 404 Not Found </h1></body></html>";
        }
        // else if forbidden, return 403
        else if (false) {
            status = "403 Forbidden";
            body = "<!DOCTYPE html><html lang=\"en\"><body><h1> 403 Forbidden </h1></body></html>";
        }
        // else return 200 and serve content
        else {
            status = "200 OK";
            std::ifstream ifs(processedURL);
            body.assign( (std::istreambuf_iterator<char>(ifs) ),
                (std::istreambuf_iterator<char>()    ) );;
        }
    }

    // HEAD support
    else if (request.method == "HEAD") {    
        status = "200 OK";
        body = "";
    }

    else {
        status = "500 Internal Server Error";
        body = "<!DOCTYPE html><html lang=\"en\"><body><h1> Error Handling Request </h1></body></html>";
    }

    std::vector<std::pair<std::string, std::string>> headers = {
        {"Connection", "keep-alive"},
        {"Content-Type", "text/html"},
        {"Content-Length", std::to_string(body.length())}
    };
    http_response response = {version, status, headers, body};
    return response;
}

