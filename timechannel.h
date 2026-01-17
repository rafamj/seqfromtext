//timechannel.h
#ifndef TIMECHANNEL_H
#define TIMECHANNEL_H

//#include <alsa/asoundlib.h>
#include "channel.h"

class TimeChannel: public Channel {
  bool clockOn;
  Event *tick_ev;
  snd_seq_tick_time_t clockTick;
  void sendTick(snd_seq_event_t *ev);
  public:
  TimeChannel(int port);
  void processEvent(Event *event,snd_seq_event_t *ev);
  Event *nextEvent();
  snd_seq_tick_time_t getTick();
};

#endif

