
// Mostly adapted from the Winsock guide
// https://learn.microsoft.com/en-us/windows/win32/winsock/

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


// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "80"

int __cdecl main(int argc, char *argv[])
{
    WSADATA wsaData; // contains socket's implementation info
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET; // for the server to listen for client connections
    SOCKET ClientSocket = INVALID_SOCKET; // for handoff

    // To instantiate SOCKET object, we need address info (and port info)
    struct addrinfo *result = NULL, *ptr = NULL, hints; 

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Call WSAStartup to initialize Winsock and return its value as an integer and check for errors.
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData); // initiates use of WS2_32.dll
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    // if (argc < 2) iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    // else iResult = getaddrinfo(NULL, argv[1], &hints, &result);
    char const * port = DEFAULT_PORT;
    if (argc > 1) port = argv[1];
    iResult = getaddrinfo(NULL, port, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    
    // Call the socket function and return its value to the ListenSocket variable
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Now to accept client connections, the socket must be bound to a network address within the system
    // Setup the TCP listening socket
    printf("Binding to port %s...\n", port);
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    // printf("address: %d\n", getsockname(ListenSocket, result->ai_addr, (int *)&result->ai_addrlen));

    // No longer need address info
    freeaddrinfo(result);

    // Now we listen on this IP and port for connection requests (SOMAXCONN is backlog size)
    printf("Listening...\n");
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf( "Listen failed with error: %ld\n", WSAGetLastError() );
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // If we get a request, we need to accept it. 
    // We can only accept a single connection with this structure. For more, one technique is to loop the listen function and accept on other threads. 
    printf("Waiting for connection...\n");
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    // (here is where you would pass the accepted client socket to a worker thread)
    
    // Now we need to send and receive data on the socket.
    // Receive until the peer shuts down the connection
    printf("Connected.\n");
    do {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);

            // Echo the buffer back to the sender
            std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> hi </h1><p> hi losers :) </p></body></html>";
            std::ostringstream ss;
            ss << "HTTP/1.0 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n" << htmlFile;
            std::string htmlOutput = ss.str();
            int bytesSent;
            long totalBytesSent = 0;

            std::cout << htmlOutput;
            // printf("%s, %d\n",htmlOutput.c_str(), htmlOutput.size());

            // while (totalBytesSent < htmlOutput.size())
            // {
            //     bytesSent = send(ClientSocket, htmlOutput.c_str(), htmlOutput.size(), 0);
            //     if (bytesSent < 0)
            //     {
            //         break;
            //     }
            //     totalBytesSent += bytesSent;
            // }

            iSendResult = send(ClientSocket, htmlOutput.c_str(), htmlOutput.size(), 0);
            

            // iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        } else if (iResult == 0)
            printf("Connection closing...\n");
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    // Once done, we shutdown the send half of the connection since no more data will be sent
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
    // When the client application is done receiving data, we close the socket and free resources.
    closesocket(ClientSocket);
    WSACleanup();

    // need to implement continuing to work after client leaves

    return 0;
}