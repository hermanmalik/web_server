#include "src/server.h"

int main(int argc, char *argv[]) {
    char const * port = NULL;
    if (argc > 2) {
        printf("Invalid syntax. This program takes at most one argument, the port number as a positive integer. If no port number is given, the server runs on port 80.");
        return 1;
    }
    if (argc > 1) port = argv[1];
    try {
        start_webserver(port);
    } catch (...) {
        printf("server error");
        return 1;
    }
}