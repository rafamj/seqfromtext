//channel.cpp

#include "channel.h"

Channel::Channel(int p) {
  port=p;
  tick=0;
  closed=false;
  seq.owner=this;
}

void Channel::processEvent(Event *event, snd_seq_event_t *ev){
     printf("Error, evento %d desconocido\n",event->type);
}

void Channel::addWait(string label, Channel *channel){
    seq.addWait(label,channel);
}

void Channel::wakeUp(snd_seq_tick_time_t t){
  tick=t;
  closed=false;
}

void Channel::jump(string label){
  seq.jump(label);
}

Event *Channel::nextEvent() {
    Event *ev=seq.nextEvent();
    while(ev && (ev->type==Event::JUMP || ev->type==Event::WAIT)) {
      if(ev->type==Event::JUMP) {
        ev->Wait.channel->jump(ev->Wait.label);
      } else if(ev->type==Event::WAIT) {
        ev->Wait.channel->addWait(ev->Wait.label,this);
        closed=true;
	return (Event *)1;
      }
      ev=seq.nextEvent();
    }
    if(!ev) {
      closed=true;
    }
    return ev;
  }

