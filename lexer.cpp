//lexer.cpp
#include <iostream>
#include <fstream>
#include <sstream>
//#include <stdio.h>
//#include <readline/readline.h>
//#include <readline/history.h>
#include "error.h"
#include "lexer.h"

static bool isSpace(char c) {
  return string(" \t").find(c)!=string::npos;
}

static bool isSpecial(char c) {
  return string(":=#()/<>\"\n,+-*|[]^;{}").find(c)!=string::npos;
}

Lexer::Lexer() {
  stream=&cin;
  line=1;
  charNum=1;
  firstToken.type=Token::NO;
}

Lexer::Lexer(char *fname) {
  static ifstream fs; ////
  fs.open(fname);
  stream=&fs;
  line=1;
  charNum=1;
  firstToken.type=Token::NO;
}

Lexer::Lexer(string s) {
  stringstream *st = new stringstream(s);
  stream=st;
  line=1;
  charNum=1;
  firstToken.type=Token::NO;
}

void Lexer::newStream(string s) {
  stringstream *st = new stringstream(s+'\n');
  streamStack.push_back(stream);
  stream=st;
  firstToken.type=Token::NO;
}

void Lexer::closeStream(){
  if(streamStack.size()>0) {
     stream=streamStack.back();
     streamStack.pop_back();
     firstToken.type=Token::NO;
  }
}

void Lexer::jumpBlanks() {
  char c=getChar();

  while(c && c!=EOF && isSpace(c)) c=getChar();
  unGet();
}

char Lexer::getChar() {
  char c;
  stream->get(c);
  if(stream->eof()) {
      return EOF;
  }  
  if(c=='\n') {
    line++;
    charNum=0;
  }  else { 
    charNum++;
  }
  //printf("getchar %c %d\n",c,c);
  return c;
}

void Lexer::unGet() {
  charNum--;
  if(charNum<1) line--;
  stream->unget();
}

string Lexer::readIdentifier(char c) {
  string result;

  do {
    result +=c;
    c=getChar();
  } while(isalnum(c) || c=='_');
  unGet();
  return result;
}

string Lexer::readNumber(char c) {
  string result;
  result +=c;
  c=getChar();
  if(result[0]=='0' && toupper(c)=='X') {
    result +=c;
    c=getChar();
    while(isxdigit(c)) {
      result +=c;
      c=getChar();
    }
  } else {
    while(isdigit(c)) {
      result +=c;
      c=getChar();
    }
  }
  unGet();
  return result;
}

string Lexer::readString(char term) {
  char c=getChar();
  string s;
  while(c!=term) {
    if(c=='\\') {
      char c1=getChar();
      switch(c1) {
        case 'n': c='\n';break;
        case 'r': c='\r';break;
        case 't': c='\t';break;
	default: c=c1;
      }
    }
    s = s + c;
    c=getChar();
  }
  return s;
}

Token Lexer::readLongComment() {
  Token t=nextToken();
  while(!t.is(Token::END) && !t.is(Token::END_COMMENT)) {
    t=nextToken();
  }
  t=nextToken(); 
  return t;
}

void Lexer::readComment() {
  char c=getChar();
  while(c!=EOF && c!='\n') {
    c=getChar();
  }
  unGet();
}

Token Lexer::nextToken() {
  if(firstToken.type!=Token::NO) {
    Token result=firstToken;
    consumeToken();
    return result;
  }
  char c=getChar();
  while(c!=EOF) {
    if(isSpace(c)) {
      jumpBlanks();
    } else if(isalpha(c)){
      return  Token(Token::IDENTIFIER,readIdentifier(c));
    } else if(isdigit(c)){
      return Token(Token::NUMBER,readNumber(c));
    } else if(c=='*'){
      c=getChar();
      if(c=='/'){ 
        return Token(Token::END_COMMENT);
      } else {
        unGet();
        return Token(Token::SPECIAL,string(1,'*'));
      }
    } else if(c=='/'){
      c=getChar();
      if(c=='/'){
         readComment();
      } else if (c=='*') {
        return readLongComment();
      } else {
        unGet();
        return Token(Token::SPECIAL,string(1,'/'));
      }
    } else if(c=='"'){
      return Token(Token::STRING,readString('"'));
    } else if(c==0 || c=='.'){
      return Token(Token::END);
    } else if(c=='<'){
      c=getChar();
      if(c=='<'){
        return Token(Token::OUT);
      } else {
        unGet();
        return  Token(Token::SPECIAL,string(1,'<'));
      }
    } else if(isSpecial(c)){
      return Token(Token::SPECIAL,string(1,c));
    } else {
      printf("line %d token UNKNOWN %c %d\n",line,c,c);
      return Token(Token::UNKNOWN);
    }
    c=getChar();
  }
  return Token(Token::END,"");
}

Token Lexer::peekToken() {
  if(firstToken.type==Token::NO) {
    firstToken=nextToken();
  }
  return firstToken;
}

void Lexer::consumeToken() {
  firstToken.type=Token::NO;
}

Token Lexer::expect(const char *c) {
  Token t=nextToken();

  for(int i=0; c[i]; i++) {
    if(t.is(c[i])) {
      return t;
    }
  }
  printf("Line %d. One of '%s' expected\n",line,c);
      t.print();
  return t;
}

Token Lexer::expect(Token::Type ty){
  Token t=nextToken();

  if(t.type!=ty) {
      printError("Error in line %d, %d expected\n",line,ty);
  }
  return t;

}

Lexer::~Lexer() {
}

