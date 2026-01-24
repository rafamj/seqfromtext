//error.cpp
#include <cstdlib>
#include <stdarg.h>
#include <stdio.h>
#include "error.h"

void printError (const char * fmt, ... ){
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    exit(0);
}

