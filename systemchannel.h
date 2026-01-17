//systemchannel.h
#ifndef SYSTEMCHANNEL_H
#define SYSTEMCHANNEL_H

//#include <alsa/asoundlib.h>
#include "channel.h"

class SysChannel: public Channel {
  public:
  SysChannel(int port);
  void processEvent(Event *event,snd_seq_event_t *ev);
};

#endif

