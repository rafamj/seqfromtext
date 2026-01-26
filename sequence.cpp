//sequence.cpp
#include <stdio.h>  ///para printf
#include "sequence.h"
#include "channel.h"

Sequence::Sequence():events(){
  index=0;
  start_loop=0;
}

Sequence::Sequence(Event *ev):events(){
  index=0;
  start_loop=0;
  events.push_back(ev);
}

void Sequence::addSequence(vector<Event *> *s){
  size_t index=events.size();
  for(size_t i=0; i<s->size();i++) {
    Event *ev= (*s)[i];
    if(ev->type==Event::LABEL) {
      Label *l=searchLabel(ev->label->label);
      if(!l) {
        labels.push_back(new Label(ev->label->label,index));
      } else {
        l->position=index;
        //printf("Error label %s redefined\n",ev->label->label.c_str());
      }
    }
    events.push_back(ev);
    index++;
  }
  //events.insert(events.end(),s->begin(),s->end());
}

Event *Sequence::peekNextEvent(){
  if(index<events.size()) {
    return events[index];
  } else {
    return 0;
  }
}


Label *Sequence::searchLabel(string name) {
    for(size_t i=0; i<labels.size(); i++) {
      if(labels[i]->label.compare(name)==0) return labels[i];
    }
  return 0;
}

void Sequence::processLabel(string name, size_t pos){
  Label *l=searchLabel(name);
  if(!l) {
    labels.push_back(new Label(name,pos));
  } else {
    for(size_t i=0; i<l->waitingChannels.size();i++) {
      l->waitingChannels[i]->wakeUp(owner->getTick());
    }
  }
}

Event *Sequence::nextEvent() {
  if(index>=events.size()) return 0;
  Event *ev=events[index];
  while(ev->isFlowControlEvent()) {
    if(ev->type==Event::LOOP_START) {
      start_loop=index;
      num_repeats=0;
    } else if(ev->type==Event::LOOP_END) {
      if(num_repeats==0) {
        num_repeats=ev->repeats;
      } else if (num_repeats!=-1) {// -1 -> break the loop	
        num_repeats--;
      }
      if(num_repeats>1 || num_repeats==0) {
        index=start_loop;
      }
    } else if(num_repeats==2 && ev->type==Event::FIRST_ENDING) { //jump to end loop
      while(ev->type!=Event::LOOP_END) {
        index++;
        ev=events[index];
      }
    } else if(ev->type==Event::LABEL) {
      processLabel(ev->label->label,index);
      ev=events[index];
    } else if(ev->type==Event::BREAK) {
      ev->channel->seq.breakLoop();
    }
    index++;
    if(index>=events.size()) return 0; //
    ev=events[index]; 
  }
  //printf("nota %d  index %ld\n",ev->note->note,index);
  index++;
  return ev;
}

void Sequence::addWait(string label, Channel *channel){
  Label *l=searchLabel(label);
  if(!l) {
    l=new Label(label,0);
    labels.push_back(l); 
  } 
  l->waitingChannels.push_back(channel);
}

void Sequence::jump(string label){
  if(label.length()>0) {
    Label *l=searchLabel(label);
    if(l) {
      index=l->position;
    }  
  } else {    
    num_repeats=-1;
  }
}

void Sequence::breakLoop(){
  num_repeats=2;
}

size_t Sequence::length(string begin, string end){
    size_t result=0;
    Label *l0=searchLabel(begin);
    Label *l1=searchLabel(end);
    if(l0 && l1) {
      for(size_t i=l0->position; i<l1->position;i++) {
        switch(events[i]->type) { 
          case Event::NOTE: result+=events[i]->note->duration;break;
	  case Event::SILENCE: result+=events[i]->silence->duration;break;
	  default: break;
        }  
      }
    }
    return result;
}

size_t Sequence::position(string label){
    Label *l=searchLabel(label);
    if(l) {
      return l->position;
    } else {
      return 0;
    }
}

