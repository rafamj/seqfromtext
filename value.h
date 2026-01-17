//value.h
#ifndef VALUE_H
#define VALUE_H

#include <string>
#include "event.h"
#include "function.h"

using namespace std;


class SysFunction;
class Table;
class Channel;
class VoiceChannel;

class Value {
    public:
  enum Type {UNDEF, STRING, EVENTS, INTEGER, IDENTIFIER, SYS_FUNCTION,ERROR,FUNCTION, CHANNEL, PORT, DICT, LABEL, FARLABEL, ARRAY};
    Type type;
    union {
      string str;
      int integer;
      vector<Event *> *events;
      Function *function;
      SysFunction *sysfunction;
      Table *dictionary;
      Channel *channel;
      VoiceChannel *voicechannel;
      Wait_t FL;
      struct {
       unsigned int len;
       char *data;
      }array;
    };
    Value(){type=UNDEF;}
    Value(string s):str(s) {type=STRING;}
    Value(string s, Type t):str(s) {type=t;}
    Value(vector<Event *> *e):events(e) {type=EVENTS;}
    Value(int n):integer(n) {type=INTEGER;}
    Value(Function *f):function(f) {type=FUNCTION;}
    Value(SysFunction *f):sysfunction(f) {type=SYS_FUNCTION;}
    Value(Channel *c):channel(c) {type=CHANNEL;}
    Value(VoiceChannel *c):voicechannel(c) {type=CHANNEL;}
    Value(Table *t):dictionary(t){type=DICT;}
    Value(int v, Type t):integer(v){type=t;}
    Value(Channel *c, string l): FL(c,l) {type=FARLABEL;}
    Value(char *c, unsigned int l){array.data=c;array.len=l;type=ARRAY;}
    void print();
};    
#endif
