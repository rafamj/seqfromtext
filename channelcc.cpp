//channelcc.cpp

#include "channelcc.h"

#define MAX_LENGTH 65535 //used as a flag

ChannelCC::ChannelCC(MidiChannel *mc, int cc):MidiChannel(mc->port,mc->midiChannel){
vc=mc; ////// 
cc_ev=new Event(cc,0);
v1=0;
v2=0;
shape=0;
length=0;
index=0;
cc_ev->CC.value=0;
}

Event *ChannelCC::nextEvent(){
  if(length==0) {
    Event *event=::Channel::nextEvent();
    if(!event) return 0;
    if(event->type==Event::SWEEP){
      procesSweep(event);
      return (Event *)1;
    } else if(event->type==Event::LFO){
      processLFO(event);
      return (Event *)1;
    } else {
      return event;
    }
  } else if (length==MAX_LENGTH) {
    if(shape==0) {
      if(v2<=v1/2) {
        cc_ev->CC.value=0;
        v2+=incTick(1);
      } else if(v2<=v1) {
        cc_ev->CC.value=127;
        v2+=incTick(1);
      } else {
        v2=0;
      }
    } else if(shape==1){
      if(v2<=v1) {
        cc_ev->CC.value= v2*127/v1;
        v2+=incTick(1);
      } else {
        v2=0;
      }
    } else if(shape==2){
      if(v2<=v1) {
        if(v2*255/v1<=127) {
          cc_ev->CC.value= v2*255/v1;
	} else {
          cc_ev->CC.value= 255-v2*255/v1;
	}
        v2+=incTick(1);
      } else {
        v2=0;
      }
    }

    return cc_ev; 
    } else { //SWEEP
      if(v2>v1) {
        cc_ev->CC.value=v1+(v2-v1)*index/(length-1);
      } else {
        cc_ev->CC.value=v1-(v1-v2)*index/(length-1);
      }
      //printf("cc_ev->CC.value %d\n",cc_ev->CC.value);
    if(index++>=length-1){
      length=0;
    }
  return cc_ev; 
  }
}

void ChannelCC::procesSweep(Event *event) {
  string l1=event->sweep.label1;
  string l2=event->sweep.label2;
  v1=event->sweep.value1;
  v2=event->sweep.value2;
  length=vc->seq.length(l1, l2)*vc->incTick(1)/incTick(1);
  vc->addWait(l1,this);
  closed=true;
  index=0;
}

void ChannelCC::processLFO(Event *event) {
  length=MAX_LENGTH;
  closed=false;
  v2=0;
  v1=event->Lfo.beats*TICKS_PER_QUARTER/event->Lfo.div;
  shape=event->Lfo.shape;
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
  if(vc->closed) { closed=true; return;}
  switch(event->type) {
    case Event::LFO: processLFO(event);break;
    case Event::SWEEP:  procesSweep(event);break;
    case Event::CONTROL_CHANGE: sendControlChange(event,ev);break;
    default: MidiChannel::processEvent(event,ev);break;
  }
}


