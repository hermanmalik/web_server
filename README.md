# Web Server with HTTP Parser

### About
This project is a basic implementation of an HTTP webserver in C++. It uses the Winsock2 library to open a TCP socket, bind to a port given as a command-line argument, and listen for incoming requests. 

If it receives an HTTP request, it parses it with the parser according to the RFC standard, and responds accordingly.

### How to Run
Requirements:
- C++
- Windows
- G++ compiler or equivalent

The libraries used are system dependent; you will need to modify the C++ includes if you want to run this on a Unix system.

Otherwise, just compile with the -lws2_32 flag and you should be good to go.  To start the webserver just run the compiled executable with the port as the command-line argument.
