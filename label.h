//label.h
#ifndef LABEL_H
#define LABEL_H

#include <string>
#include <vector>
using namespace std;

class Channel;

class Label {
  public:
  string label;
  size_t position;
  vector<Channel *> waitingChannels;
  Label(string l):label(l){position=0;}
  Label(string l, size_t p):label(l){position=p;}
};
#endif
