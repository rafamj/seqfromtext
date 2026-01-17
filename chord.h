//chord.h
#ifndef CHORD_H
#define CHORD_H

#include <vector>
using namespace std;

class Chord {
  public:
  int note; //contains one note of a chord
  int duration;
  Chord(int note, int duration);
};
#endif
