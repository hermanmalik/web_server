
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
#include "request_handler.h"
#include "parser.h"


// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "80"
#define BACKLOG 10 // how many pending connections to hold

// Starts a http server on the given port
void __cdecl start_webserver(char const * port)
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
        throw std::exception();
    }

    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    // if (argc < 2) iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    // else iResult = getaddrinfo(NULL, argv[1], &hints, &result);
    if (port == NULL) port = DEFAULT_PORT;
    iResult = getaddrinfo(NULL, port, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed on port %s: %d\n", port, iResult);
        WSACleanup();
        throw std::exception();
    }
    
    // Call the socket function and return its value to the ListenSocket variable
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        throw std::exception();
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
        throw std::exception();
    }
    // printf("address: %d\n", getsockname(ListenSocket, result->ai_addr, (int *)&result->ai_addrlen));

    // No longer need address info
    freeaddrinfo(result);

    // Now we listen on this IP and port for connection requests (SOMAXCONN is backlog size)
    printf("Listening...\n");
    if (listen(ListenSocket, BACKLOG) == SOCKET_ERROR) {
        printf( "Listen failed with error: %ld\n", WSAGetLastError() );
        closesocket(ListenSocket);
        WSACleanup();
        throw std::exception();
    }
    
    // If we get a request, we need to accept it. 
    // We can only accept a single connection with this structure. For more, one technique is to loop the listen function and accept on other threads. 
    printf("Waiting for connection...\n");
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        throw std::exception();
    }
    // (here is where you would pass the accepted client socket to a worker thread)
    // Windows is CreateThread
    
    // Now we need to send and receive data on the socket.
    // Receive until the peer shuts down the connection
    printf("Connected.\n");
    do {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            // Print what was received
            if(DEBUG) printf("\nBytes received: %d\n", iResult);
            if(DEBUG) printf("%s\n", recvbuf);
            http_request request = parseRequest(recvbuf, recvbuflen);
            http_response response = handle_request(request);
            std::string output = responseToString(response);            

            // Send back our output 
            iSendResult = send(ClientSocket, output.c_str(), output.size(), 0);
            if(DEBUG) printf("Bytes sent: %d\n", iSendResult);
            if(DEBUG) std::cout << output << "\n";

            // Check for send error
            if (iSendResult == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                throw std::exception();
            }
        } else if (iResult == 0) // receiving empty message means to close?
            printf("Connection closing...\n");
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            throw std::exception();
        }

    } while (iResult > 0);

    // Once done, we shutdown the send half of the connection since no more data will be sent
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        throw std::exception();
    }
    // When the client application is done receiving data, we close the socket and free resources.
    closesocket(ClientSocket);
    WSACleanup();

    // need to implement continuing to work after client leaves

}

// implement thread function
DWORD WINAPI ThreadFunc(LPVOID lpParam) {
    return 0;
}

// move this to main code, this is how you create a new thread
HANDLE CreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    SIZE_T dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
);

// lpThreadAttributes: Optional security attributes for the new thread. NULL
// dwStackSize: The initial size of the stack, in bytes. Use 0 for default size. 0
// lpStartAddress: Pointer to the thread function. ThreadFunc
// lpParameter: Pointer to a variable to pass to the thread function. Can be NULL if no parameter is needed. (void *)&(as_threads[i_location].new_fd
// dwCreationFlags: Flags that control the creation of the thread (e.g., priority). 0
// lpThreadId: Pointer to a variable that receives the thread identifier. Can be NULL if you don't need it. &dw_thread_id

// CreateThread returns a handle to the newly created thread. You can use this handle to manipulate the thread (e.g., wait for its completion, terminate it, etc.). Make sure to close the handle when its no longer needed to avoid resource leaks:

HANDLE hThread;
DWORD dwThreadId;

hThread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, &dwThreadId);
if (hThread == NULL) {
    // Handle error
}

// Close the thread handle when it's no longer needed
CloseHandle(hThread);