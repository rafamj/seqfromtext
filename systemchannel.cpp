//systemchannel.cpp

#include "systemchannel.h"

SysChannel::SysChannel(int p): Channel(p) {
}

void SysChannel::processEvent(Event *event,snd_seq_event_t *ev){
  if(event->type==Event::SYSEX) {
    //printf("send sysex size %d \n",event->sysEx->size);
    snd_seq_ev_set_sysex(ev, event->sysEx->size, event->sysEx->buffer);
  } else {
    Channel::processEvent(event,ev);
  }
}

