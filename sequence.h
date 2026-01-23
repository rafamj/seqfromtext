//sequence.h
#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <vector>
#include "event.h"

using namespace std;

class Sequence{
  vector<Event *> events;
  vector<Label*> labels;
  size_t index;
  size_t start_loop;
  int num_repeats;
  Label *searchLabel(string name);
  void processLabel(string name, size_t pos);
  public:
  Sequence();
  Sequence(Event *ev);
  Channel *owner;
  void addSequence(vector<Event *> *s);
  Event *peekNextEvent();
  Event *nextEvent();
  void addWait(string label, Channel *channel);
  void jump(string label);
  size_t length(){return events.size();}
  size_t length(string begin, string end);
  size_t position(string label);
};
#endif
