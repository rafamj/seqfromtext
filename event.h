//event.h
#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <alsa/asoundlib.h>
#include "note.h"
#include "silence.h"
#include "sysex.h"
#include "chord.h"
#include "label.h"

    struct Wait_t{
      Channel *channel;
      string label;
      Wait_t(Channel *c, string l):channel(c),label(l){}
    };

 
class Event {
  public:
  enum Type {NOTE, LOOP_START, LOOP_END, FIRST_ENDING, PROGRAM_CHANGE, CONTROL_CHANGE, SYSEX, CHORD, SILENCE, VOLUME, TIME, GATE, CLOCKONOFF, CLOCK_TICK,
             START, STOP, LABEL, WAIT, WAKEUP, JUMP, SWEEP, BREAK};
  Type type;
  union {
    Note *note;
    Silence *silence;
    SysEx *sysEx;
    int programNumber;
    int repeats; //for loops
    int value; //for volume, 
    struct {
      int control;
      int value;
    } CC;
    struct {
      int numerator;
      int division;
    } TM;
    struct Wait_t Wait;
    Chord *chord;
    bool clockOn;
    Label *label;
    snd_seq_tick_time_t tick; //for WAKEUP
    vector<int> values; //for sweep
    Channel *channel; //for BREAK
  };
  Event(Note *n);
  Event(Silence *s);
  Event();
  Event(Type t);
  Event(int n); //LOOP_END
  Event(int control, int value); //CONTROL_CHANGE
  Event(SysEx *sx);
  Event(Chord *c);
  Event(Type t, int value);
  Event(bool clk):clockOn(clk){type=CLOCKONOFF;}
  Event(Label *l):label(l){type=LABEL;}
  Event(Channel *n, string l); //wait and jump
  Event(Channel *n); //BREAK
  Event(vector <int> v);
  bool isQueueEvent();
  bool isFlowControlEvent();
  //~Event(){}
};
#endif

