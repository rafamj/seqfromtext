//note.h
#ifndef NOTE_H
#define NOTE_H
class Note {
  public:
  int note;
  int duration;
  Note(int note, int duration);
  //Note(Note &n){note=n.note;duration=n.duration;}
};
#endif
