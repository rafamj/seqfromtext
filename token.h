//token.h

#include <string>

using namespace std;


class Token {
  public:
  enum Type {NO,UNKNOWN,IDENTIFIER, NUMBER, SPECIAL, END, STRING, OUT, END_COMMENT};
  Type type;
  string value;
  Token():type(UNKNOWN),value(){};
  Token(Type ty):type(ty),value(){};
  Token(Type ty, string s):type(ty),value(s){};
  bool is(char c) {return type==SPECIAL && value[0]==c;}
  bool is(Type ty) {return type==ty;}
  void print();
};
