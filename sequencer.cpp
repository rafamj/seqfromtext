/* sequencer.cpp */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <limits.h>
#include "error.h"
#include  "sequencer.h"
#include "voicechannel.h"
#include "timechannel.h"
#include "systemchannel.h"

#define MAX_EVENTS  8 


Sequencer::Sequencer() {
  this->bpm0=120;
  this->ticks_per_quarter=TICKS_PER_QUARTER;
  transpose=0;
}

void Sequencer::open_seq(const char * device, const  char *name) {
  if (snd_seq_open(&seq_handle, device, SND_SEQ_OPEN_DUPLEX, 0) < 0) {
    fprintf(stderr, "Error opening ALSA sequencer.\n");
    exit(1);
  }
  snd_seq_set_client_name(seq_handle, name);
}


int Sequencer::createOutPort(const char *name){
  char nameOutput[60];
  int out;

  sprintf(nameOutput,"%s output",name);
  if ((out = snd_seq_create_simple_port(seq_handle, nameOutput,
            SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
  }
  port_out_id.push_back(out);
  printf("port %d out %d\n",snd_seq_client_id(seq_handle),out);
  return port_out_id.size()-1;
}

int Sequencer::createInPort(const char *name){
  char nameInput[60];
  int in;

  sprintf(nameInput,"%s input",name);
  if ((in = snd_seq_create_simple_port(seq_handle, nameInput,
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }
  printf("port %d  in %d\n",snd_seq_client_id(seq_handle),in);
  port_in_id.push_back(in);
  return port_in_id.size()-1;
}

int Sequencer::searchClient(string cl) { //ponerlo en sequencer.cpp
   int status;
   snd_seq_client_info_t* info;

   snd_seq_client_info_alloca(&info);

   status = snd_seq_get_any_client_info(seq_handle, 0, info);

   while (status >= 0) {
      int id = snd_seq_client_info_get_client(info);
      char const* name = snd_seq_client_info_get_name(info);
      if(strcmp(name,cl.c_str())==0) {
        //printf("found %s %d\n",name,id);
	return id;
      }
      status = snd_seq_query_next_client(seq_handle, info);
   }
   printError("Client %s not found",cl.c_str());   
   return -1;
}

void Sequencer::connect(bool out, int local_port, int client, int port) {
    snd_seq_addr_t sender, dest;
    snd_seq_port_subscribe_t *subs;
   
    //printf("connect sender %d %d  dest %d %d\n",snd_seq_client_id(seq_handle),port_in_id,client,port);
    if(out) {
      dest.client = client;
      dest.port = port;
      sender.client = snd_seq_client_id(seq_handle);
      sender.port = port_out_id[local_port];
    } else {
      dest.client = snd_seq_client_id(seq_handle);
      dest.port =  port_in_id[local_port];
      sender.client = client;
      sender.port = port;
    }
    snd_seq_port_subscribe_alloca(&subs);
    snd_seq_port_subscribe_set_sender(subs, &sender);
    snd_seq_port_subscribe_set_dest(subs, &dest);
    snd_seq_port_subscribe_set_queue(subs, 1);
    snd_seq_port_subscribe_set_time_update(subs, 1);
    snd_seq_port_subscribe_set_time_real(subs, 1);
    if(snd_seq_subscribe_port(seq_handle, subs)) {
      if(out) {
        printError("Error connecting to %d:%d",client,port);
      } else {
        printError("Error connecting from %d:%d",client,port);
      }
    }

}


void Sequencer::set_tempo(int bpm) {

  snd_seq_queue_tempo_t *queue_tempo;
  snd_seq_queue_tempo_malloc(&queue_tempo);
  int tempo = (int)(6e7 / ((double)bpm * (double)ticks_per_quarter) * (double)ticks_per_quarter);
  snd_seq_queue_tempo_set_tempo(queue_tempo, tempo);
  snd_seq_queue_tempo_set_ppq(queue_tempo, ticks_per_quarter);
  snd_seq_set_queue_tempo(seq_handle, queue_id, queue_tempo);
  snd_seq_queue_tempo_free(queue_tempo);
}

snd_seq_tick_time_t Sequencer::get_tick() {

  snd_seq_queue_status_t *status;
  snd_seq_tick_time_t current_tick;
  
  snd_seq_queue_status_malloc(&status);
  snd_seq_get_queue_status(seq_handle, queue_id, status);
  current_tick = snd_seq_queue_status_get_tick_time(status);
  snd_seq_queue_status_free(status);
  return(current_tick);
}

void Sequencer::init_queue() {

  queue_id = snd_seq_alloc_queue(seq_handle);
  //size_t pool_size=max_sequence_length();
  size_t pool_size=MAX_EVENTS+1;
  snd_seq_set_client_pool_output(seq_handle, (pool_size<<1) + 4);
}

void Sequencer::clear_queue() {

  snd_seq_remove_events_t *remove_ev;

  snd_seq_remove_events_malloc(&remove_ev);
  snd_seq_remove_events_set_queue(remove_ev, queue_id);
  snd_seq_remove_events_set_condition(remove_ev, SND_SEQ_REMOVE_OUTPUT | SND_SEQ_REMOVE_IGNORE_OFF);
  snd_seq_remove_events(seq_handle, remove_ev);
  snd_seq_remove_events_free(remove_ev);
}

VoiceChannel *Sequencer::newVoiceChannel(int port, int midiChannel){
  VoiceChannel *c=new VoiceChannel(port, midiChannel);
  channel.push_back(c);
  return c;
}

TimeChannel *Sequencer::newTimeChannel(int port){
  TimeChannel *c=new TimeChannel(port);
  channel.push_back(c);
  return c;
}

SysChannel *Sequencer::newSysExChannel(int port){
  SysChannel *c=new SysChannel(port);
  channel.push_back(c);
  return c;
}

ChannelCC *Sequencer::newChannelCC(MidiChannel *ch, int cc){
  ChannelCC *c=new ChannelCC(ch,cc);
  channel.push_back(c);
  return c;
}

Channel *Sequencer::selectNextChannel() {
  Channel *channel=0;
  snd_seq_tick_time_t min_tick=UINT_MAX;
      for(size_t i=0; i<this->channel.size(); i++) {
        if(!this->channel[i]->closed && !this->channel[i]->waiting && this->channel[i]->seq.length()>0) {
          Event *ev=this->channel[i]->peekNextEvent();
          if(ev && (this->channel[i]->getTick()<min_tick || (this->channel[i]->getTick()==min_tick &&  (ev->type!=Event::NOTE || ev->type!=Event::CHORD))))  {
	    if(this->channel[i]->getTick()==0) { //real time event
	      this->channel[i]->setTick(get_tick());
	      return this->channel[i];
	    }
	    channel=this->channel[i];
	    min_tick=this->channel[i]->getTick();
	  } 
	}
      }
   return channel;
}

void Sequencer::send_events() {
  int n=0;
  //int result;
  snd_seq_event_t ev;
  static snd_seq_tick_time_t last_tick=0; 
  Event *event=0;
  int port=0;

     n=queueEventsCount();
     //printf("send_events n %d\n",n);
  if(n>= MAX_EVENTS-1) {
    usleep(10000);
    return;
  }
  while(n < MAX_EVENTS-1) {
      Channel *channel=selectNextChannel();
      if(channel==0) {
	    if(!continueFlag && last_tick < get_tick() && waitList.size()==0) {
	      printf("end\n");
	      stop();
	    } else {
              //usleep(100000);
	      return;
	    }
      } else {
        event=channel->nextEvent();

        if(!event) {  
	  //It is possible to get !event because the control flow events
	  //printf("ERROR\n");exit(0);
	  continue;
        } else if(event==(Event *)1) { //call next event
	    continue;
	}
        if(event->isQueueEvent()) {
          snd_seq_ev_clear(&ev);
          snd_seq_ev_schedule_tick(&ev, queue_id,  0, channel->getTick());
          snd_seq_ev_set_source(&ev, port_out_id[port]);
          snd_seq_ev_set_subs(&ev);
          channel->processEvent(event,&ev);
	  last_tick=max(last_tick,channel->getTick());
          //snd_seq_event_output(seq_handle, &ev);
	  //snd_seq_drain_output(seq_handle);
	  int result; 
	  if ((result = snd_seq_event_output_direct(seq_handle, &ev)) >= 0) {
            //printf("ALSA MIDI write ok:\n");
          } else {
            printf("ALSA MIDI write error: %s %d\n", snd_strerror(result),result);
          }
	  
	  n++;
	} else {
          channel->processEvent(event,&ev);
	}
     }
   }
  //printf("end send_events()\n\n\n");
}

void Sequencer::send_sequences() {
  //printf("send_sequences channels %ld channel 0 size %ld closed %d\n",this->channel.size(),this->channel[0]->seq.length(),this->channel[0]->closed);
  for(size_t i=0; i<this->channel.size(); i++) {
    if(this->channel[i]->seq.length()==0) {
      printf("channel %ld empty\n",i);
      this->channel[i]->closed=true;
    } else if (this->channel[i]->closed){
      this->channel[i]->closed=false;
      //this->channel[i]->setTick(get_tick()+1);
      this->channel[i]->setTick(0);
    }
  }
  //printf("send sequences\n");
  send_events();
}


void Sequencer::midi_action() {
  snd_seq_event_t *ev;

  do {
    snd_seq_event_input(seq_handle, &ev);

    if(!ev){ continue;}

    if(ev->type==SND_SEQ_EVENT_NOTEON) {
      //printf("note on received channel %d note %d tick %u\n",ev->data.note.channel, ev->data.note.note, ev->time.tick);
    } else if(ev->type==SND_SEQ_EVENT_KEYPRESS) {
      //printf("keypress received channel %d note %d value %d tick %u\n",ev->data.note.channel, ev->data.note.note, ev->data.note.velocity,ev->time.tick);
    } else if(ev->type==SND_SEQ_EVENT_NOTEOFF) {
      //printf("note off received channel %d %d tick %u\n",ev->data.note.channel,ev->data.note.note, ev->time.tick);
    } else if(ev->type==SND_SEQ_EVENT_PGMCHANGE) {
      //printf("program change received channel %d %d tick %u\n",ev->data.control.channel,ev->data.control.param, ev->time.tick);
    } else if(ev->type==SND_SEQ_EVENT_CONTROLLER) {
      //printf("control change received channel %d %d %d tick %u\n",ev->data.control.channel,ev->data.control.param , ev->data.control.value, ev->time.tick);
    } else if(ev->type==SND_SEQ_EVENT_SYSEX) {
        printf("SYS EX RECEIVED\n");
    
        printf("SYS EX RECEIVED len %u\n",ev->data.ext.len);
        if(waitList.size()>0) {
	  Function *f=waitList[0];
	  vector<Value *> parameters;
	  parameters.push_back(new Value((char *)ev->data.ext.ptr,ev->data.ext.len));
	  parameters.push_back(new Value(ev->data.ext.len));
	  parser->execFunction(f,parameters);
	  waitList.erase(waitList.begin());
          if(!continueFlag && waitList.size()==0) {
	    //printf("end wait list\n");
            stop();
          }
	}

    } else if(ev->type==SND_SEQ_EVENT_CLOCK) {
        //printf("CLOCK tick %u %d %d\n",ev->time.tick,ev->time.time.tv_sec,ev->time.time.tv_nsec);
    } else if(ev->type==SND_SEQ_EVENT_SENSING) {
      //printf("Sensing received\n");
    } else {
        printf("UNKNOWN EVENT RECEIVED %d\n",ev->type);
    }
    snd_seq_free_event(ev);
  } while (snd_seq_event_input_pending(seq_handle, 0) > 0);
}

void Sequencer::start_queue(){
  //printf("start_queue\n");
  snd_seq_start_queue(seq_handle, queue_id, NULL);
  snd_seq_drain_output(seq_handle);
  npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
  //printf("npfd %d\n",npfd);
  //pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
  pfd = (struct pollfd *)malloc(npfd * sizeof(struct pollfd));
  snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
}

int Sequencer::queueEventsCount() {
  snd_seq_queue_status_t *status;
  int events_count;

	snd_seq_queue_status_alloca(&status);
	snd_seq_get_queue_status(seq_handle, queue_id, status);
	events_count = snd_seq_queue_status_get_events(status);

	//printf("Eventos pendientes en la cola: %d\n", events_count);
  return events_count;
}

void Sequencer::loop() {
    if (poll(pfd, npfd, 0) > 0) {
	  midi_action(); 
    }
   //send_events();
}


void Sequencer::stop() {
  clear_queue();
  //sleep(2);
  snd_seq_stop_queue(seq_handle, queue_id, NULL);
  snd_seq_free_queue(seq_handle, queue_id);
  free(pfd);
  if(!continueFlag) {
     exit(0);
  }
}

                             
