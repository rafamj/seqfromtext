//channelcc.h
#ifndef CHANNELCC_H
#define CHANNELCC_H

#include "midichannel.h"

class ChannelCC: public MidiChannel {
  Channel* vc; ////
  Event *cc_ev;
  size_t length;
  int v1,v2;
  vector<int> values;
  size_t index=0;
  size_t v_index;
  void sendControlChange(Event *event, snd_seq_event_t *ev);
  void procesSweep(Event *event);
  void loadSection();
  public:
  ChannelCC(MidiChannel *mc, int cc);
  void processEvent(Event *event,snd_seq_event_t *ev);
  Event *peekNextEvent();
  Event *nextEvent();
  //snd_seq_tick_time_t getTick();
};

#endif

