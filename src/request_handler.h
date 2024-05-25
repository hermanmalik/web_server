#ifndef HANDLER_H
#define HANDLER_H

#include <iostream>
#include <string>
#include "parser.h"

// this should take in an http_request and return the response
http_response handle_request(http_request request);

#endif