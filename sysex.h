//sysex.h
#ifndef SYSEX_H
#define SYSEX_H

#include <stdint.h>


class SysEx {
  public:
  uint8_t *buffer;
  int size;
  SysEx(uint8_t *buffer, int size);
  ~SysEx();
};
#endif
