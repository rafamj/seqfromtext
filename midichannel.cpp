//midichannel.cpp

#include "midichannel.h"


MidiChannel::MidiChannel(int p, int mchan):Channel(p) {
  midiChannel=mchan;
  numerator=4;
  division=16;
}

void MidiChannel::sendProgramChange(Event *event,snd_seq_event_t *ev){
  //printf("send program change channel %d program %d tick %d\n",midiChannel, event->programNumber,tick);
  snd_seq_ev_set_pgmchange(ev, midiChannel, event->programNumber);
}

void MidiChannel::sendControlChange(Event *event,snd_seq_event_t *ev){
    //printf("send controller channel %d control %d value %d\n",this->channel[channel]->midiChannel, event->CC.control,event->CC.value);
    snd_seq_ev_set_controller(ev, midiChannel, event->CC.control,event->CC.value);
}


snd_seq_tick_time_t MidiChannel::incTick(int duration){ 
  return duration*TICKS_PER_QUARTER*numerator/division;
}

void MidiChannel::processEvent(Event *event,snd_seq_event_t *ev){
  switch(event->type) {
    case Event::SILENCE: tick += event->silence->duration*TICKS_PER_QUARTER*numerator/division;break;
    case Event::TIME: numerator=event->TM.numerator;division=event->TM.division;break;
    case Event::PROGRAM_CHANGE: sendProgramChange(event,ev);break;
    case Event::CONTROL_CHANGE: sendControlChange(event,ev);break;
    default: Channel::processEvent(event,ev);break;
  }
}

