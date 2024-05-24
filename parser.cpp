
// #include <stdlib.h>
// #include <stdio.h>
#include <string>
// #include <cstring>
// #include <cstdlib>
// #include <sstream>
// #include <iostream>


struct html_request {
    std::string method;
    std::string URI;
    html_header** headers;
    std::string data;
};

struct html_header {
    std::string name;
    std::string content;
};

html_request parseRequest(char request[], int len) {
    
}