//channelcc.cpp

#include "channelcc.h"

#define MAX_LENGTH 65535 //used as a flag

ChannelCC::ChannelCC(MidiChannel *mc, int cc):MidiChannel(mc->port,mc->midiChannel){
vc=mc; ////// 
cc_ev=new Event(cc,0);
v1=0;
v2=0;
length=0;
index=0;
cc_ev->CC.value=0;
v_index=0;
}

Event *ChannelCC::peekNextEvent(){
  if(vc->closed) { 
    closed=true; 
    return 0;
  } else if(!closed) {
    if(v_index) {
      return cc_ev;
    } else {
      return ::Channel::peekNextEvent();
    }
  } else {
    return 0;
  }
}

void ChannelCC::loadSection(){
  v1=v2;
  length=values[v_index++];
  v2=values[v_index++];
  index=0;
}

Event *ChannelCC::nextEvent(){
  //if(vc->closed) { closed=true; return 0;}
  if(v_index==0) {
    Event *event=::Channel::nextEvent();
    if(!event) return 0;
    if(event->type==Event::SWEEP){
      procesSweep(event);
      return (Event *)1;
    } else {
      return event;
    }
  } else { //SWEEP
      if(v2>v1) {
        cc_ev->CC.value=v1+(v2-v1)*index/length;
      } else {
        cc_ev->CC.value=v1-(v1-v2)*index/length;
      }
      //printf("cc_ev->CC.value %d\n",cc_ev->CC.value);
    if(index++>=length){
      if(v_index<values.size()) {
        loadSection();
      } else {
	v_index=0;
      }
    }
    return cc_ev; 
  }
}

void ChannelCC::procesSweep(Event *event) {
  values=event->values;
  v_index=0;
  v2=values[v_index++];
  loadSection();
  closed=false;
}

void ChannelCC::sendControlChange(Event *event,snd_seq_event_t *ev){
    //printf("send SWEEP controller channel %d control %d value %d tick %d\n",midiChannel, event->CC.control,event->CC.value,tick);
    snd_seq_ev_set_controller(ev,midiChannel, event->CC.control,event->CC.value);
    //printf("cc tick1 %u\n",tick);tick+=vc->incTick(1);printf("cc tick2 %u\n",tick);
    //printf("cc tick1 %u\n",tick);tick+=incTick(1);printf("cc tick2 %u\n",tick);
    tick+=incTick(1);
    //tick+=vc->incTick(1);
}
 
void ChannelCC::processEvent(Event *event,snd_seq_event_t *ev){
  //printf("processEvent %d\n",event->type);
  switch(event->type) {
    case Event::SWEEP:  procesSweep(event);break;
    case Event::CONTROL_CHANGE: sendControlChange(event,ev);break;
    default: MidiChannel::processEvent(event,ev);break;
  }
}


