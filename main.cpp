#include "server.h"

int main(int argc, char *argv[]) {
    char const * port = NULL;
    if (argc > 1) port = argv[1];
    try {
        start_webserver(port);
    } catch (...) {
        return 1;
    }
}