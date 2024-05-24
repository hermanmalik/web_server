#ifndef WEBSERVER_H
#define WEBSERVER_H

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include "parser.h"

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "80"

// Starts a http server on the given port 
void __cdecl start_webserver(char const * port);

#endif