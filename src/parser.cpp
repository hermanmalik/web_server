
#define WIN32_LEAN_AND_MEAN

// #include <stdlib.h>
// #include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <algorithm>
// #include <cctype> // for tolower
// #include <cstring>
// #include <cstdlib>
// #include <sstream>
// #include <iostream>

#define ROOT_DIR "root/"
#define DEBUG 1

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
                if(DEBUG) printf("Method parsing error");
                throw std::invalid_argument("Invalid request line format");
            }
            out.method = line.substr(0, methodEnd);
            size_t pathEnd = line.find(' ', methodEnd + 1);
            if (pathEnd == std::string::npos) {
                if(DEBUG) printf("URI parsing error");
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
    if (request.method != "GET" && request.method != "HEAD" && request.method != "POST" 
        && request.method != "PUT" && request.method != "DELETE" && request.method != "CONNECT" 
        && request.method != "OPTIONS" && request.method != "TRACE" && request.method != "PATCH") return 0;
    if (request.version != "HTTP/0.9" && request.version != "HTTP/1.0"
         && request.version != "HTTP/1.1" && request.version != "HTTP/2"
         && request.version != "HTTP/3") return 0;
    return 1;
}

// Takes a URI and returns a filepath
std::string parseURI(std::string URI) {
    // we assume all URIs are filepaths
    std::string processedURL = URI;    

    // Find the first character which is not a letter, number, _, /, ., -, or space, and cut out anything after it
    size_t pos = processedURL.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_/.- ");
    if (pos != std::string::npos) {
        processedURL = processedURL.substr(0, pos);
    }

    // Strip out any double periods, periods before the last /, and double slashes
    size_t doublePeriod = processedURL.find("..");
    while (doublePeriod != std::string::npos) {
        processedURL.erase(doublePeriod, 1);
        doublePeriod = processedURL.find("..");
    }
    
    size_t doubleSlash = processedURL.find("//");
    while (doubleSlash != std::string::npos) {
        processedURL.replace(doubleSlash, 2, "/");
        doubleSlash = processedURL.find("//");
    }
    // alternative double slash removal - is it equivalent?
    // std::string::iterator new_end = std::unique(processedURL.begin(), processedURL.end(),
    //     [](char lhs, char rhs) { return (lhs == rhs) && (lhs == '/'); }
    // );
    // processedURL.erase(new_end, processedURL.end());

    if (processedURL.find_last_of("/") > 0) {
        size_t periodBeforeSlash = processedURL.find_last_of(".", processedURL.find_last_of("/") - 1);
        while (periodBeforeSlash != std::string::npos) {
            processedURL.erase(0, periodBeforeSlash);
            periodBeforeSlash = processedURL.find_last_of(".", processedURL.find_last_of("/") - 1);
        }
    }

    // Strip leading slash
    if (!processedURL.empty() && processedURL[0] == '/') {
        processedURL.erase(0, 1);
    }
    
    // Prepend root/
    processedURL = ROOT_DIR + processedURL;

    // Turn all '/'s into '\'s
    std::replace(processedURL.begin(), processedURL.end(), '/', '\\');

    // TODO DEAL WITH CASE SENSITIVITY 
    // std::transform(processedURL.begin(), processedURL.end(), processedURL.begin(), std::tolower);

    // if (DEBUG) std::cout << "stripped URL: " << processedURL << "\n";

    // If the path ends with a slash, try to find index.html
    if (processedURL.back() == '\\') {
        // if index.html exists serve that
        if (GetFileAttributesA((processedURL + "index.html").c_str()) != INVALID_FILE_ATTRIBUTES) {
            return processedURL + "index.html";
        }
        // else remove the trailing slash
        processedURL = processedURL.substr(0, processedURL.length() - 1);
    }
    
    // if the requested file exists, serve it
    if (GetFileAttributesA(processedURL.c_str()) != INVALID_FILE_ATTRIBUTES) {
        // NEED TO CHECK IF IT IS FOLDER AND RETURN 403 
    }

    // else, look in the folder above for a txt, html, or other allowed 
    else if (GetFileAttributesA((processedURL+".html").c_str()) != INVALID_FILE_ATTRIBUTES) {
        return processedURL+".html";
    }
    else if (GetFileAttributesA((processedURL+".txt").c_str()) != INVALID_FILE_ATTRIBUTES) {
        return processedURL+".txt";
    }

    // // File not found but 404 file exists
    // if (GetFileAttributesA("root\\404.html") != INVALID_FILE_ATTRIBUTES) {
    //     return "root\\404.html";
    // }

    // File not found
    else processedURL = "";

    // if (DEBUG) std::cout << "processed URL: " << processedURL << "\n";

    return processedURL;

}