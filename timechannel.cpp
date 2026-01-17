//timechannel.cpp

#include "timechannel.h"

TimeChannel::TimeChannel(int p): Channel(p) {
  clockTick=0;
  clockOn=false;
  tick_ev=new Event(Event::CLOCK_TICK);
  printf("creado TimeChannel\n");
}

void TimeChannel::sendTick(snd_seq_event_t *ev){
  printf("sendTick tick %u\n",clockTick);
  ev->type = SND_SEQ_EVENT_CLOCK;
  clockTick += TICKS_PER_QUARTER/24;
}

Event *TimeChannel::nextEvent(){
  printf("time nextEvent clockOnOff %d seq:length %ld tick %u clockTick %u\n",clockOn,seq.length(),tick,clockTick);
  if(clockOn && clockTick<=tick) {
      return tick_ev; 
  } else if(seq.length()>0) {
    printf("seq.nextEvent\n");
    return seq.nextEvent();
  }
  return 0;
}

void TimeChannel::processEvent(Event *event,snd_seq_event_t *ev){
  printf("processEvent\n");
  switch(event->type) {
    case Event::START: printf("start\n");ev->type = SND_SEQ_EVENT_START;break;
    case Event::STOP:  printf("stop\n");ev->type = SND_SEQ_EVENT_STOP;break;
    case Event::SILENCE: printf("wait tick %d\n",tick + event->silence->duration); tick += event->silence->duration;break;
    case Event::CLOCKONOFF: printf("clock on off %d\n",event->clockOn);clockOn=event->clockOn;break;
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

