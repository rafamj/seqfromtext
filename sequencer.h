/* sequencer.h */
#ifndef SEQUENCER_H
#define SEQUENCER_H
#include <alsa/asoundlib.h>
#include "sequence.h"
#include "voicechannel.h"
#include "midichannel.h"
#include "timechannel.h"
#include "systemchannel.h"
#include "channelcc.h"
#include "function.h"
#include "value.h"
#include "parser.h"

//class Parser;

class Sequencer {
   snd_seq_t *seq_handle;
   int queue_id;
   vector<Channel *> channel;
   int npfd;
   int bpm0;
   int ticks_per_quarter;
   int transpose;
   vector<int> port_in_id;
   vector<int> port_out_id;
   struct pollfd *pfd;
   Channel *selectNextChannel();
   int queueEventsCount();
  public:
  Parser *parser;
  vector<Function *> waitList;
  bool continueFlag;
  Sequencer();
   void open_seq(const char * device, const  char *name);
   int createOutPort(const char *name);
   int createInPort(const char *name);
   int searchClient(string cl);
   void connect(bool out, int port_out, int client, int port);
   void set_tempo(int bpm);
   snd_seq_tick_time_t get_tick();
   void init_queue();
   void clear_queue(); 
   VoiceChannel *newVoiceChannel(int port, int midiChannel);
   TimeChannel *newTimeChannel(int port);
   SysChannel *newSysExChannel(int port);
   ChannelCC *newChannelCC(MidiChannel *ch, int cc);
   void send_events();
   void send_sequences();
   void midi_action();
   void start_queue();
   void loop();
   void stop();
};
#endif


