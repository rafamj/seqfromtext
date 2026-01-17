//sysex.cpp

#include "sysex.h"

SysEx::SysEx(uint8_t *buffer, int size){
  this->buffer= new uint8_t[size];
  for(int i=0; i<size;i++) {
    this->buffer[i]=buffer[i];
  }
  this->size=size;
}

SysEx::~SysEx(){
  delete [] buffer;
}
