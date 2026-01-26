//error.cpp
#include <cstdlib>
#include <stdarg.h>
#include <stdio.h>
#include "sequencer.h"
#include "error.h"

static void stop() {
extern Sequencer *seqPointer;
  if(!seqPointer->continueFlag) {
    seqPointer->stop();
  }
}


void printError (const char * fmt, ... ){
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    stop();
}

