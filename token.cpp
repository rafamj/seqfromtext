//token.cpp

#include "token.h"

static string type2string(Token::Type t) {
  switch(t) {
    case Token::UNKNOWN: return "UNKNOWN";
    case Token::IDENTIFIER: return "IDENTIFIER";
    case Token::NUMBER: return "NUMBER";
    case Token::END: return "END";
    case Token::SPECIAL: return "SPECIAL";
    case Token::STRING: return "STRING";
    case Token::OUT: return "OUT";
    default: return "UNKNOWN";
  }
}


void Token::print() {
  printf("%s %s ",type2string(type).c_str(),value.c_str());
  if(type==SPECIAL) {
    printf(" %d\n",value.c_str()[0]);
  } else {
    printf("\n");
  }
}
