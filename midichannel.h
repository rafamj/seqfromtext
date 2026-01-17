//midichannel.h
#ifndef MIDICHANNEL_H
#define MIDICHANNEL_H

//#include <alsa/asoundlib.h>
#include "channel.h"

class MidiChannel: public Channel {
  snd_seq_event_t ev;
  public:
  void sendProgramChange(Event *event,snd_seq_event_t *ev);
  void sendControlChange(Event *event,snd_seq_event_t *ev);
  int midiChannel;
  int numerator, division;
  MidiChannel(int port, int mchan);
  void processEvent(Event *event,snd_seq_event_t *ev); 
  snd_seq_tick_time_t incTick(int duration);
};

#endif

