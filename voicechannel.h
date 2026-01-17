//voicechannel.h
#ifndef VOICECHANNEL_H
#define VOICECHANNEL_H

//#include <alsa/asoundlib.h>
#include "midichannel.h"

class VoiceChannel: public MidiChannel {
  snd_seq_event_t ev;
  int transpose;
  void sendNote(Event *event,snd_seq_event_t *ev);
  void sendChord(Event *event,snd_seq_event_t *ev);
  public:
  int octave, volume, gate;
  VoiceChannel(int port, int mchan);
  void processEvent(Event *event,snd_seq_event_t *ev); 
};

#endif

