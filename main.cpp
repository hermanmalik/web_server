#include "src/server.h"

int main(int argc, char *argv[]) {
    char const * port = NULL;
    if (argc > 1) port = argv[1];
    try {
        start_webserver(port);
    } catch (...) {
        printf("server error");
        return 1;
    }
}