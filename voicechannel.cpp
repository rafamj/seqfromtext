//voicechannel.cpp

#include "voicechannel.h"


VoiceChannel::VoiceChannel(int p, int mchan):MidiChannel(p,mchan) {
  octave=4;
  volume=100;
  gate=100;
  transpose=0;
}


void VoiceChannel::sendNote(Event *event,snd_seq_event_t *ev){
    int next=event->note->duration*TICKS_PER_QUARTER*numerator/division;
    int duration=next*gate/100;
    //printf("send  channel %d note %d velocity %d duration %d tick %u\n",midiChannel-1, event->note->note + transpose, volume, duration,tick);
    snd_seq_ev_set_note(ev, midiChannel, event->note->note + transpose, volume, duration);
    tick += next;
}

void VoiceChannel::sendChord(Event *event,snd_seq_event_t *ev){
    int next=event->chord->duration*TICKS_PER_QUARTER*numerator/division;
    int duration=next*gate/100;
    //printf("send  channel %d note %d velocity %d duration %d tick %u\n",midiChannel-1, event->chord->note + transpose, volume, duration,tick);
    snd_seq_ev_set_note(ev, midiChannel, event->chord->note + transpose, volume, duration);
}


void VoiceChannel::processEvent(Event *event,snd_seq_event_t *ev){
  switch(event->type) {
    case Event::VOLUME:  volume=event->value;break;
    case Event::GATE: gate=event->value;break;
    case Event::NOTE: sendNote(event,ev);break;
    case Event::CHORD: sendChord(event,ev);break;
    default: MidiChannel::processEvent(event,ev);break;
  }
}

