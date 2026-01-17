//systemchannel.cpp

#include "systemchannel.h"

SysChannel::SysChannel(int p): Channel(p) {
}

void SysChannel::processEvent(Event *event,snd_seq_event_t *ev){
  if(event->type==Event::SYSEX) {
    //printf("send sysex size %d tick %d\n",event->sysEx->size, this->channel[channel]->tick);
    snd_seq_ev_set_sysex(ev, event->sysEx->size, event->sysEx->buffer);
  } else {
    Channel::processEvent(event,ev);
  }
}

