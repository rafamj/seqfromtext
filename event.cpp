//event.cpp

#include "event.h"

Event::Event(Note *n) {
  note=n;
  type=NOTE;
}

Event::Event(Silence *s) {
  silence=s;
  type=SILENCE;
}

Event::Event() {
  type=LOOP_START;
  note=0;
}

Event::Event(Type t) {
  type=t;
}

Event::Event(int n) {
  type=LOOP_END;
  repeats=n;
}
  
Event::Event(int control, int value){
  type=CONTROL_CHANGE;
  CC.control=control;
  CC.value=value;
}

Event::Event(SysEx *sx) {
  type=SYSEX;
  sysEx=sx;
}

Event::Event(Chord *c) {
  type=CHORD;
  chord=c;
}

Event::Event(Type t, int v) {
  type=t;
  value=v;
}

Event::Event(Channel *c, string l):Wait(c,l){
  type=WAIT;
}

Event::Event(Channel *c){
  channel=c;
  type=BREAK;
}

Event::Event(vector <int> v):values(v){
  type=SWEEP;
}

bool Event::isQueueEvent(){
  switch(type) {
    case NOTE: 
    case PROGRAM_CHANGE:
    case CONTROL_CHANGE:
    case CLOCK_TICK:
    case CHORD: 
    case START:
    case STOP:
    case SYSEX: return true;
    default: return false;
  }
}

bool Event::isFlowControlEvent(){
  switch(type) {
    case LABEL:
    case BREAK:
    case LOOP_START:
    case LOOP_END:
    case FIRST_ENDING: return true;
    default: return false;
  }
}
