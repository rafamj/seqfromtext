//timechannel.cpp

#include "timechannel.h"

TimeChannel::TimeChannel(int p): Channel(p) {
  clockTick=0;
  clockOn=false;
  tick_ev=new Event(Event::CLOCK_TICK);
}

void TimeChannel::sendTick(snd_seq_event_t *ev){
  //printf("sendTick tick %u\n",clockTick);
  ev->type = SND_SEQ_EVENT_CLOCK;
  clockTick += TICKS_PER_QUARTER/24;
}

Event *TimeChannel::nextEvent(){
  //printf("time nextEvent clockOnOff %d seq:length %ld tick %u clockTick %u\n",clockOn,seq.length(),tick,clockTick);
  if(clockOn && clockTick<=tick) {
      return tick_ev; 
  } else if(seq.length()>0) {
    return Channel::nextEvent();
  }
  return 0;
}

void TimeChannel::processEvent(Event *event,snd_seq_event_t *ev){
  switch(event->type) {
    case Event::START: ev->type = SND_SEQ_EVENT_START;break;
    case Event::STOP:  ev->type = SND_SEQ_EVENT_STOP;break;
    case Event::SILENCE: tick += event->silence->duration;break;
    case Event::CLOCKONOFF: clockOn=event->clockOn;break;
    case Event::CLOCK_TICK: sendTick(ev);break; 
    default: Channel::processEvent(event,ev);break;
  }
}

snd_seq_tick_time_t TimeChannel::getTick(){
  if(clockOn) {
    return min(clockTick,tick);
  }  else {
    return tick;
  }
}

