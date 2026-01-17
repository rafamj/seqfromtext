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


int Sequencer::createPort(const char *name){
  char nameInput[60];
  char nameOutput[60];
  int out;
  int in;

  sprintf(nameOutput,"%s output",name);
  if ((out = snd_seq_create_simple_port(seq_handle, nameOutput,
            SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
  }
  port_out_id.push_back(out);
  sprintf(nameInput,"%s input",name);
  if ((in = snd_seq_create_simple_port(seq_handle, nameInput,
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
    fprintf(stderr, "Error creating sequencer port.\n");
    exit(1);
  }
  port_in_id.push_back(in);
  return port_out_id.size()-1;
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
        printError("Error connecting to %d:%d\n",client,port);
      } else {
        printError("Error connecting from %d:%d\n",client,port);
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

Channel *Sequencer::selectNextChannel(snd_seq_tick_time_t &min_tick) {
  Channel *channel=0;
  min_tick=UINT_MAX;
      //printf("selectNextChannel\n");
      for(size_t i=0; i<this->channel.size(); i++) {
        Event *ev=this->channel[i]->seq.peekNextEvent();
        if(!this->channel[i]->closed && (this->channel[i]->getTick()<min_tick || (this->channel[i]->getTick()==min_tick && ev && (ev->type!=Event::NOTE || ev->type!=Event::CHORD))))  {
	  channel=this->channel[i];
 //  printf("selected %ld\n",i);
	  min_tick=channel->getTick();
	}
      }
   return channel;
}

void Sequencer::send_events() {
  int n=0;
  int result;
  snd_seq_event_t ev;
  snd_seq_tick_time_t next_tick=UINT_MAX; 
  //snd_seq_tick_time_t echo_tick=UINT_MAX; 
  Event *event=0;
  int port=0;

     n=queueEventsCount();
     //printf(" \n\nsend_events n %d\n",n);
  if(n>=MAX_EVENTS-1) selectNextChannel(next_tick);
  while(n < MAX_EVENTS-1) {
      Channel *channel=selectNextChannel(next_tick);
      //echo_tick=min(echo_tick,next_tick);
      //printf("n %d channel %p\n",n,channel);
      if(channel==0) {
        //printf("error channel NULL\n");exit(0);
	return;
        break;
      } else {
        event=channel->nextEvent();

        //printf("channel %p  event %p port %d dest %d\n",channel,event,channel->port,port_in_id[port]);
        if(!event) {
          //printf("END channel %p remainingChannels %lu\n",channel,remainingChannels);
	  channel->closed=true;
	  remainingChannels--;
	  //next_tick=this->channel[channel]->tick;
	  if(remainingChannels>0) {
	    continue;
	  } else {
            break;
	  }  
        } else if(event==(Event *)1) { //call next event
	    continue;
	}
        if(event->isQueueEvent()) {
          snd_seq_ev_clear(&ev);
          snd_seq_ev_schedule_tick(&ev, queue_id,  0, channel->getTick());
          snd_seq_ev_set_source(&ev, port_out_id[port]);
          snd_seq_ev_set_subs(&ev);
          channel->processEvent(event,&ev);
	  if ((result = snd_seq_event_output_direct(seq_handle, &ev)) >= 0) {
            //printf("ALSA MIDI write ok:\n");
          } else {
            //printf("ALSA MIDI write error: %s %d\n", snd_strerror(result),result);
          }
	  n++;
	} else {
          channel->processEvent(event,&ev);
	}
     }
   }

   snd_seq_ev_clear(&ev);
          //snd_seq_ev_set_source(&ev, port_out_id[port]);
          //snd_seq_ev_set_subs(&ev);
   ev.type = SND_SEQ_EVENT_ECHO; 
   snd_seq_ev_schedule_tick(&ev, queue_id,  0, next_tick );
   snd_seq_ev_set_dest(&ev, snd_seq_client_id(seq_handle), port_in_id[port]); ///////
   //printf("ECHO sent port %d dest %d tick %u\n",port_out_id[port],port_in_id[port],next_tick);
   if ((result = snd_seq_event_output_direct(seq_handle, &ev)) >= 0) {
          //snd_seq_drain_output(seq_handle);
          //printf("ALSA MIDI write ok:\n");
   } else {
          //printf("ALSA MIDI write error: %s %d\n", snd_strerror(result),result);
   }
   
  //printf("end send_events()\n\n\n");
}

void Sequencer::send_sequences() {
  //printf("send_sequences channels %ld\n",this->channel.size());
  remainingChannels=0;
  for(size_t i=0; i<this->channel.size(); i++) {
    if(this->channel[i]->seq.length()==0) {
      //printf("channel %ld empty\n",i);
      this->channel[i]->closed=true;
      //this->channel[i]->seq.transmited=true;
    } else {
      remainingChannels++;
    }
  }
  //printf("remainingChannels %lu\n",remainingChannels);
  send_events();
}


void Sequencer::midi_action() {
  snd_seq_event_t *ev;

  //if(!snd_seq_event_input_pending(seq_handle, 0)) return;
  //printf("midi action\n");
  do {
    snd_seq_event_input(seq_handle, &ev);
    if(ev->type==SND_SEQ_EVENT_ECHO) {
        //printf("SND_SEQ_EVENT_ECHO\n");
     if(remainingChannels==0) {
       if(!continueFlag && waitList.size()==0) {
         //printf("exit\n");
         exit(0);
       } else {
         //printf("continue\n");
       }
     } else {
       send_events();
     }
    } else if(ev->type==SND_SEQ_EVENT_NOTEON) {
      printf("note on received channel %d %d tick %u\n",ev->data.note.channel, ev->data.note.note, ev->time.tick);
    } else if(ev->type==SND_SEQ_EVENT_NOTEOFF) {
      printf("note off received channel %d %d tick %u\n",ev->data.note.channel,ev->data.note.note, ev->time.tick);
    } else if(ev->type==SND_SEQ_EVENT_PGMCHANGE) {
      printf("program change received channel %d %d tick %u\n",ev->data.control.channel,ev->data.control.param, ev->time.tick);
    } else if(ev->type==SND_SEQ_EVENT_CONTROLLER) {
      printf("control change received channel %d %d %d tick %u\n",ev->data.control.channel,ev->data.control.param , ev->data.control.value, ev->time.tick);
    } else if(ev->type==SND_SEQ_EVENT_SYSEX) {
    /*
        printf("SYS EX RECEIVED len %u\n",ev->data.ext.len);
	for(unsigned int i=0; i<ev->data.ext.len; i++){
	  printf("%02hhX ",*((char *)ev->data.ext.ptr + i));
	}
	printf("\n");
	*/
        if(waitList.size()>0) {
	  Function *f=waitList[0];
	  vector<Value *> parameters;
	  parameters.push_back(new Value((char *)ev->data.ext.ptr,ev->data.ext.len));
	  parameters.push_back(new Value(ev->data.ext.len));
	  parser->execFunction(f,parameters);
	  waitList.erase(waitList.begin());
          if(!continueFlag && waitList.size()==0) {
            exit(0);
          }
	}
    } else {
        //printf("UNKNOWN EVENT RECEIVED %d tick %u\n",ev->type,ev->time.tick);
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
  pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
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
  //while (1) {
    if (poll(pfd, npfd, 100000) > 0) {
    //if (poll(pfd, npfd, 0) > 0) {
      for (int i = 0; i< npfd; i++) {
        if (pfd[i].revents > 0) midi_action(); 
        //if (pfd[i].revents | POLLIN) midi_action(); 
      }
    }  
  //}
}

void Sequencer::sigterm_exit(int sig) {

  //Sequencer::clear_queue();
  //sleep(2);
  //snd_seq_stop_queue(seq_handle, queue_id, NULL);
  //snd_seq_free_queue(seq_handle, queue_id);
  exit(0);
}

                             
