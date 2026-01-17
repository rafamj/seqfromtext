//sysfunction.h
#ifndef SYSFUNCTION_H
#define SYSFUNCTION_H
#include "value.h"


class SysFunction {
  public:
  enum Type {OPEN, CONNECTOUT, CONNECTIN, TEMPO, NOTE, PAD, MIDI_CHANNEL, TIME_CHANNEL, SYSEX_CHANNEL, CHANNELCC, WAIT, START, STOP, PRINTF, WAITSYSEX};
  Type type; //number of function
  vector<Value::Type> parameters;
  SysFunction(Type t, vector<Value::Type> p):type(t),parameters(p){}
};
#endif
