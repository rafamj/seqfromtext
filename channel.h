//channel.h
#ifndef CHANNEL_H
#define CHANNEL_H

#include <alsa/asoundlib.h>
#include "sequence.h"

#define TICKS_PER_QUARTER 192

class Channel {
  public:
  int port;
  snd_seq_tick_time_t tick;
  bool closed;
  bool waiting;
  Sequence seq;
  Channel(int port);
  virtual void processEvent(Event *event, snd_seq_event_t *ev);
  virtual Event *nextEvent(); 
  virtual Event *peekNextEvent();
  virtual snd_seq_tick_time_t getTick(){return tick;}
  void setTick(snd_seq_tick_time_t t){tick=t;}
  void addWait(string label, Channel *channel);
  void wakeUp(snd_seq_tick_time_t t);
  void jump(string label);
  void addSequence(vector<Event *> *s){seq.addSequence(s);}
  virtual snd_seq_tick_time_t incTick(int duration){ return duration;}
};

#endif

