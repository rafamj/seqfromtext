//parser.h
#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "table.h"
#include "lexer.h"
//#include "sequencer.h"
class Sequencer;

class Parser{
  Lexer lex;
  Sequencer *seq;
  Table table;
  int octave;
  bool continueFlag;
  void checkLimits(int n, int v0, int v1);
  vector<Event *> *readPattern(string p);
  vector<Event *> *parse_sequence(); 
  Value *mixNotes(Value *v1, Value *v2);
  Value *multValues(Value *v1, Value *v2);
  Value *divValues(Value *v1, Value *v2);
  Value *sumValues(Value *v1, Value *v2);
  Value *subValues(Value *v1, Value *v2);
  Value *readProduct(); 
  Value *readSum(); 
  Value *readExpression(); 
  void execPrintf(vector<Value *> v);
  Value *execSysFunction(SysFunction *f);
  Value *readValue(); 
  vector<string> readParameters();
  void readDef();
  void readDict();
  //void outNotes(int channel, Value *v);
  //void outSysEx(Value *v);
  void init();
  int parseNote(int note);
  vector<Value *> parseIntegeParameters(unsigned int n);
  vector<uint8_t> parseSysEx(int base);
  vector<int> parseChord();
  vector<Event *> *parseEventValue(); 
  void jumpBlanks();
  int readDuration();
  vector<Value *>readListOfValues(char terminator);
  void printError(const char * fmt, ... );
  public:
  Value *execFunction(Function *f, vector<Value *> parameters);
  Parser(Sequencer *s);
  Parser(Sequencer *s, char *fname);
  void parse();
  void parse(char *str);
  ~Parser();
};
#endif

