//lexer.h
#include <stdio.h>
#include <vector>
#include "token.h"

#define MAX_BUFFER 80

class Lexer{
  vector<istream *> streamStack;
  istream *stream;
  int line;
  int charNum; 
  Token firstToken;
  void jumpBlanks();
  string readIdentifier(char c);
  string readNumber(char c);
  void readComment();
  Token readLongComment();
  int reopenTty();
  public:
  Lexer();
  Lexer(char *fname);
  Lexer(string s);
  void newStream(string s);
  void closeStream();
  int lineNum() {return line;}
  char getChar();
  string readString(char term);
  void unGet();
  Token nextToken();
  Token peekToken();
  void consumeToken();
  Token expect(const char *c);
  Token expect(Token::Type t);
  ~Lexer();
};
