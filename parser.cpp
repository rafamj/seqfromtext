//parser.cpp
#include "error.h"
#include "parser.h"
#include "sysfunction.h"
#include "sequencer.h"

extern "C" {
  extern void add_history (const char *);
}

static int note2midi(char note, int octave, int alt) {
  int n;

  if(note>'B') octave--;
  n=(note-'A')*2;
  if(note>='C') n--;
  if(note>='F') n--;
  return 9 + n + alt + octave * 12;
}

static uint8_t hexString2int(string v) {
  if(v.length()>2) {
    printError("Number too long, %s",v.c_str());
  }
  return stoi(v,0,16);
}


Parser::Parser(Sequencer *s):lex(),table() {
  seq=s;
  init();
}

Parser::Parser(Sequencer *s, char *fname):lex(fname),table() {
  seq=s;
  init();
}

void Parser::init(){
  octave=4;
  seq->continueFlag=false;
  table.insert("openOutPort", new Value(new SysFunction(SysFunction::OPENOUT, vector<Value::Type>({Value::STRING}))));
  table.insert("openInPort", new Value(new SysFunction(SysFunction::OPENIN, vector<Value::Type>({Value::STRING}))));
  table.insert("connect", new Value(new SysFunction(SysFunction::CONNECT, vector<Value::Type>({Value::UNDEF, Value::UNDEF, Value::INTEGER}))));
  //table.insert("connectOut", new Value(new SysFunction(SysFunction::CONNECTOUT, vector<Value::Type>({Value::OUTPORT, Value::INTEGER, Value::INTEGER})))); ///
  //table.insert("connectIn", new Value(new SysFunction(SysFunction::CONNECTIN, vector<Value::Type>({Value::INPORT, Value::INTEGER, Value::INTEGER}))));////
  table.insert("tempo",new Value(new SysFunction(SysFunction::TEMPO,vector<Value::Type>({Value::INTEGER}))));
  table.insert("note",new Value(new SysFunction(SysFunction::NOTE, vector<Value::Type>({Value::INTEGER, Value::INTEGER}))));
  table.insert("pad",new Value(new SysFunction(SysFunction::PAD, vector<Value::Type>({Value::STRING, Value::INTEGER}))));
  table.insert("channel",new Value(new SysFunction(SysFunction::MIDI_CHANNEL, vector<Value::Type>({Value::OUTPORT, Value::INTEGER}))));
  table.insert("timeChannel",new Value(new SysFunction(SysFunction::TIME_CHANNEL, vector<Value::Type>({Value::OUTPORT}))));
  table.insert("sysExChannel",new Value(new SysFunction(SysFunction::SYSEX_CHANNEL, vector<Value::Type>({Value::OUTPORT}))));
  table.insert("channelCC",new Value(new SysFunction(SysFunction::CHANNELCC, vector<Value::Type>({Value::CHANNEL, Value::INTEGER}))));
  table.insert("wait",new Value(new SysFunction(SysFunction::WAIT, vector<Value::Type>({Value::INTEGER}))));
  table.insert("start",new Value(new SysFunction(SysFunction::START, vector<Value::Type>())));
  table.insert("stop",new Value(new SysFunction(SysFunction::STOP, vector<Value::Type>())));
  table.insert("printf",new Value(new SysFunction(SysFunction::PRINTF, vector<Value::Type>())));
  table.insert("waitSysEx",new Value(new SysFunction(SysFunction::WAITSYSEX, vector<Value::Type>({Value::INPORT, Value::FUNCTION}))));
  table.insert("add_history",new Value(new SysFunction(SysFunction::ADDHISTORY, vector<Value::Type>({Value::STRING}))));
  table.insert("_Dict",new Value(new Table()));
}


void Parser::jumpBlanks() {
  char c=lex.getChar();
  while(c == ' ') {
    c=lex.getChar();
  }
  lex.unGet();
}

int Parser::readDuration() {
    int duration=1;
    while(lex.getChar()=='_') {
       duration++;
    }
    lex.unGet();
return duration;
}

void Parser::checkLimits(int n, int v0, int v1)  {
  if(n<v0 || n>v1) {
   //   printf("Error in line %d. ",lex.lineNum()); ///////////
   // ::printError("Number %d out of limits. Must be between %d and %d\n",n,v0,v1);
   printError("Number %d out of limits. Must be between %d and %d",n,v0,v1);
  }
}

int Parser::parseNote(int note) {
  int alt=0;

  char c=lex.getChar();
  if(c=='v' && octave>0) {
    octave--;
    c=lex.getChar();
  } else if(c=='^' && octave <9) {
    octave++;
    c=lex.getChar();
  }
  if(c=='+') {
    alt=1;
  } else if(c=='-') {
    alt=-1;
  } else {
    lex.unGet();
  }
  return note2midi(note,octave,alt);
}

vector<Value *> Parser::parseIntegeParameters(unsigned int n) {
  lex.expect("(");
  vector<Value *> v=readListOfValues(')');
  if(v.size()!=n) {
    if(n!=1) {
      printError("%d parameters expected",n);
    } else {
      printError("One parameter expected");
    }
  } else {
    for(size_t i=0; i<v.size();i++) {
      if(v[i]->type!=Value::INTEGER) {
        printError("Number expected in parameter %d",i);
      }
    }
  }
  return v;
}

vector<uint8_t> Parser::parseSysEx(int base) {
      vector<uint8_t> buffer;

      lex.expect("(");
      Token t=lex.nextToken();
      while(!t.is(')')) {
        if(t.is('<')) {
          vector<Value *> v=readListOfValues('>');
          for(size_t i=0; i<v.size();i++) {
	    if(v[i]->type==Value::INTEGER) {
	      buffer.push_back(v[i]->integer);
	    } else if(v[i]->type==Value::STRING) {
	      buffer.insert(buffer.end(),v[i]->str.begin(),v[i]->str.end());
	    } else {
	      printError("Integer expected");
	    }
          }
	} else if(base==16 && (t.is(Token::IDENTIFIER) || t.is(Token::NUMBER))) {
	  if(t.is(Token::NUMBER)) {
	    char c=lex.getChar();
	    if(isxdigit(c)) {
	      t.value += c;
	    } else {
	      lex.unGet();
	    }
	  }
	  uint8_t byte=hexString2int(t.value);
	  //printf("HEX t.value %s %d %x\n",t.value.c_str(),byte, byte);
	  buffer.push_back(byte);
	} else if(base==10 && t.is(Token::NUMBER)) {
	  if(t.value[0]=='0' && toupper(t.value[1])=='X') {
	    uint8_t byte=hexString2int(t.value.substr(2));
	    //printf("HEX t.value %s %d %x\n",t.value.c_str(),byte, byte);
	    buffer.push_back(byte);
	  } else {
	    uint8_t byte=atoi(t.value.c_str());
	    buffer.push_back(byte);
	  }
	} else {
	  printError("Hex number expected");
	}
        t=lex.nextToken();
      }
      return buffer;
}

vector<int> Parser::parseChord() {
  vector<int> s;
  jumpBlanks();
  char c=lex.getChar();
  while(c!=']'){
    int note=toupper(c);
    if(note>='A' && note<='G') {
      int midiNote=parseNote(note);
      s.push_back(midiNote);
    } else {
      printError("Note expected");
    }
    jumpBlanks();
    c=lex.getChar();
  }
  return s;
}

vector<Event *> *Parser::parseEventValue() {
  vector<Value *> v=readListOfValues('>');
  vector<Event *>  *s=new vector<Event *>;

  for(size_t i=0;i<v.size();i++) {
    if(v[i]->type==Value::EVENTS) {
      s->insert(s->end(),v[i]->events->begin(),v[i]->events->end());
    } else if(v[i]->type==Value::STRING) {
      lex.newStream(v[i]->str);
      Token t=lex.peekToken(); 
      if(t.is('/')) {
	Value *val=readExpression();
        lex.closeStream();
	if(val->type==Value::EVENTS) {
          s->insert(s->end(),val->events->begin(),val->events->end());
	}
      } else {
        lex.closeStream();
        vector<Event *>  *ev=readPattern(v[i]->str);
        s->insert(s->end(),ev->begin(),ev->end());
      }
    } else {
      printError("Value not a sequence");
    }
  }
  return s; 
}

vector<Event *> *Parser::readPattern(string p) {
  vector<Event *> *result=new vector<Event *>; 
  Value *dv=table.search("_Dict");
  Table *dict=dv->dictionary;

  for(size_t i=0;i<p.size();i++) {
    if(p[i]!=' ' && p[i]!='\t'){
      Value *v=dict->search(string(1,p[i]));
      if(!v) {
        if(p[i]=='x') {
	  result->push_back(new Event(new Note(60, 1)));
	} else if(p[i]=='.') {
          result->push_back(new Event(new Silence(1)));
	} else if(p[i]=='_') {
	  if(result->size()>0 && result->back()->type==Event::NOTE) {
            result->back()->note->duration++;
	  }
	} else {
          printError("'%c' not found in the Dictionary",p[i]);
	}
      } else {	
        if(v->type==Value::EVENTS) {
          result->insert(result->end(),v->events->begin(),v->events->end());
	} else {
          printError("Events expected");
	}
      }
    }
  }
  return result;
}

vector<Event *>  *Parser::parse_sequence() {
  vector<Event *> *result=new vector<Event *>; 
  int note;
  jumpBlanks();
  char c=lex.getChar();
  
  Table *dict=table.search("_Dict")->dictionary;
  dict->levelUp();
  while(c!='/') {
    note=toupper(c);
    if(note>='A' && note<='G') {
        char c1=lex.getChar();
        if(note=='B' && toupper(c1)=='K') {  //break loop
          lex.expect("(");
          vector<Value *> v=readListOfValues(')');
          if(v.size()!=1 || v[0]->type!=Value::CHANNEL) {
	    printError("Channel expected");
	  }
	  result->push_back(new Event(v[0]->channel));
        } else if(note=='C' && toupper(c1)=='C') {  //Control Change
	  vector<Value *> v=parseIntegeParameters(2);
	  checkLimits(v[0]->integer,0,127);
	  checkLimits(v[1]->integer,0,127);
	  result->push_back(new Event(v[0]->integer,v[1]->integer));
	} else if(note=='D' && toupper(c1)=='D') {   // SysEx Decimal
          vector<uint8_t> buffer=parseSysEx(10);
          result->push_back(new Event(new SysEx(&buffer[0], buffer.size())));
	} else if(note=='D' && toupper(c1)=='I') {   // SysEx Decimal
	  char c1=lex.getChar();
	  char c2=lex.getChar();
	  if(toupper(c1)=='C' && toupper(c2)=='T') {
            lex.expect("{");
            readDict();
	  }
	} else if(note=='G' && toupper(c1)=='T') {   // Gate Time 
	  vector<Value *> v=parseIntegeParameters(1);
	  result->push_back(new Event(Event::GATE, v[0]->integer));
        } else {               //Note
	  lex.unGet();
	  int midiNote=parseNote(note);
	  result->push_back(new Event(new Note(midiNote, readDuration())));
        }
    } else if(note=='Z') { //silence
	result->push_back(new Event(new Silence(readDuration())));
    } else if(note=='J') { //Jump
      lex.expect("(");
      vector<Value *> v=readListOfValues(')');
      if(v.size()!=1 || v[0]->type!=Value::FARLABEL) {
        v[0]->print();
        printError("Error in J parameter");
      }
        Event *ev= new Event(v[0]->FL.channel,v[0]->FL.label);
	ev->type=Event::JUMP;
        result->push_back(ev);
    } else if(note=='K') { //CLOCK /////
      char c1=lex.getChar();
      result->push_back(new Event(c1=='1'));
    } else if(note=='M') { //midi note
        vector<Value *> v=parseIntegeParameters(1);
        checkLimits(v[0]->integer,0,127);
	result->push_back(new Event(new Note(v[0]->integer, 1)));
    } else if(note=='O') { //Octave
        vector<Value *> v=parseIntegeParameters(1);
	octave=v[0]->integer;
    } else if(note=='P') { //program change
        vector<Value *> v=parseIntegeParameters(1);
	Event *ev=new Event(Event::PROGRAM_CHANGE);
	checkLimits(v[0]->integer,1,128);
	ev->programNumber=v[0]->integer-1;
	result->push_back(ev);
    } else if(note=='S') { 
      lex.expect("(");
      vector<Value *> v=readListOfValues(')');
      vector<int> values;
      if(v.size()<3 || v.size()%2==0) {
        printError("Error in size of list of values of S() (must be odd and > 3)");
      }
      for(size_t i=0; i<v.size();i++) {
        if(i%2==0) {
	  checkLimits(v[i]->integer,0,127);
	}
        values.push_back(v[i]->integer);
      }
      result->push_back(new Event(values));
    } else if(note=='T') { 
      vector<Value *> v=parseIntegeParameters(2);
      Event *ev=new Event(Event::TIME);
      ev->TM.numerator=v[0]->integer;
      ev->TM.division=v[1]->integer;
      if(ev->TM.division==0) printError("Division by zero");
      result->push_back(ev);
    } else if(note=='V') { 
      vector<Value *> v=parseIntegeParameters(1);
      checkLimits(v[0]->integer,0,127);
      result->push_back(new Event(Event::VOLUME, v[0]->integer));
    } else if(note=='W') { 
      lex.expect("(");
      vector<Value *> v=readListOfValues(')');
      if(v.size()!=1 || v[0]->type!=Value::FARLABEL) {
        v[0]->print();
        printError("Error in W parameter");
      }
      result->push_back(new Event(v[0]->FL.channel,v[0]->FL.label));
    } else if(note=='X') { // SysEx Hex
        vector<uint8_t> buffer=parseSysEx(16);
	result->push_back(new Event(new SysEx(&buffer[0], buffer.size())));
    } else if(c=='{') { 
      readDict();
    } else if(c=='[') { //chord
      vector<int> s=parseChord();
      int duration=readDuration();
      if(s.size()>0) {
        for(size_t i=0;i<s.size()-1;i++) {
          result->push_back(new Event(new Chord(s[i],duration))); // a chord note don't increase the tick count
        }
        result->push_back(new Event(new Note(s[s.size()-1], duration))); // the last note increases the tick count
      }
    } else if(c=='<') { //variable
      vector<Event *> *evs=parseEventValue();
      result->insert(result->end(),evs->begin(),evs->end());
    } else if(c=='"') { //pattern
      string p=lex.readString('"');
      vector<Event *> *evs=readPattern(p);
      result->insert(result->end(),evs->begin(),evs->end());
    } else if(c=='|') { 
        char c1=lex.getChar();
        if(c1==':') {  //start loop
          result->push_back(new Event());
	} else if (c1=='|') {          // first ending
	  lex.unGet();
          result->push_back(new Event(Event::FIRST_ENDING));
	}
    } else if (c==':') { // end loop
      char c1=lex.getChar();
      if(c1=='|') {
        int num_repeats=0;
        jumpBlanks();
	c=lex.getChar();
        while(isdigit(c)) {
         num_repeats=num_repeats *10 + c-'0'; 
         c=lex.getChar();
        }
	lex.unGet();
        result->push_back(new Event(num_repeats));
      } else if(isalpha(c1)) {
        lex.unGet();
	Token t=lex.nextToken();
	if(t.is(Token::IDENTIFIER)) {
          result->push_back(new Event(new Label(t.value)));
	} else {
	  printError("Identifier expected");
	}
      }
    } else {
      printError("error reading note %c %d",c,c);
    }
    jumpBlanks();
    c=lex.getChar();
  }
  dict->levelDown();
  return result;
}

vector<Value *>Parser::readListOfValues(char terminator) {
  vector<Value *> result;
  Token t=lex.peekToken();
  char separator[3];
  if(t.is(terminator)) { //0 values
    lex.consumeToken();
    return result;
  }

  separator[0]=',';
  separator[1]=terminator;
  separator[2]=0;

  while(!t.is(terminator)) {
    Value *v=readExpression();
    result.push_back(v);
    t=lex.expect(separator);
  }
  return result;
}

Value *Parser::mixNotes(Value *v1, Value *v2) {
  size_t i=0, j=0;
  while(i<v1->events->size() && j<v2->events->size()) {
    while(i<v1->events->size() && (*v1->events)[i]->type!=Event::NOTE && (*v1->events)[i]->type!=Event::CHORD) i++;
    while(j<v2->events->size() && (*v2->events)[j]->type!=Event::NOTE && (*v2->events)[j]->type!=Event::CHORD) j++;
    if(i>=v1->events->size() || j>=v2->events->size()) return v1;
    if((*v1->events)[i]->type==(*v2->events)[j]->type)  {
      (*v1->events)[i]=new  Event(new Note((*v2->events)[j]->note->note,(*v2->events)[j]->note->duration));
      i++;j++;
    } else if((*v1->events)[i]->type==Event::NOTE && (*v2->events)[j]->type==Event::CHORD)  {
      while((*v2->events)[j]->type==Event::CHORD) {
       v1->events->insert(v1->events->begin()+i,new  Event(new Chord((*v2->events)[j]->chord->note, (*v2->events)[j]->chord->duration)));
       i++;j++;
      }
      (*v1->events)[i]=new  Event(new Note((*v2->events)[j]->note->note,(*v2->events)[j]->note->duration));
      i++;j++;
    } else if((*v1->events)[i]->type==Event::CHORD && (*v2->events)[j]->type==Event::NOTE)  {
      i++;
    }
  }
  return v1;
}

Value *Parser::multValues(Value *v1, Value *v2) {
  if(v1->type==Value::EVENTS && v2->type==Value::EVENTS){
    return mixNotes(v1,v2);
  } else if(v2->type!=Value::INTEGER) {
    printError("The second value of a product must be an integer");
    return v1;
  }
  int n=v2->integer;
  switch(v1->type) {
    case Value::INTEGER: return new Value(v1->integer*v2->integer); break;
    case Value::STRING: {string s;
      while(n-->0){ s=s+v1->str; }
      return new Value(s);} break;
    case Value::EVENTS: { vector<Event *> *events=new vector<Event *>;
                          while(n-->0){ events->insert(events->end(),v1->events->begin(),v1->events->end()); }
                          return new Value(events);} break;
    default: printError("Product not defined for these types");return v1; break;
  }
}

Value *Parser::divValues(Value *v1, Value *v2) {
  if(v1->type!=v2->type) {
    printError("Division of values of different type");
    printf("%d %d\n",v1->integer,v2->integer);
    return v1;
  }
  switch(v1->type) {
    case Value::INTEGER: if(v2->integer!=0) return new Value(v1->integer/v2->integer); else return 0;break;
    default: printError("Division not defined for these types");return v1; break;
  }
}

Value *Parser::readProduct() {
  Value *v=readValue();
  Token t=lex.peekToken();
  while(t.is('*') || t.is('/')) {
    lex.consumeToken();
    if(t.is('*')) {
      v=multValues(v,readValue());
    } else {
      v=divValues(v,readValue());
    }
    t=lex.peekToken();
  }
  return v;
}

Value *Parser::subValues(Value *v1, Value *v2) {
  if(v1->type!=v2->type) {
    printError("Substraction of values of different type");
    return v1;
  }
  switch(v1->type) {
    case Value::INTEGER: return new Value(v1->integer-v2->integer); break;
    default: printError("Substraction not defined for these types");return v1; break;
  }
}


Value *Parser::sumValues(Value *v1, Value *v2) {
  if(v1->type!=v2->type) {
    printError("sum of values of different type");
    printf("%d %d\n",v1->integer,v2->integer);
    return v1;
  }
  switch(v1->type) {
    case Value::INTEGER: return new Value(v1->integer+v2->integer); break;
    case Value::STRING: return new Value(v1->str + v2->str);break;
    case Value::EVENTS: v1->events->insert(v1->events->end(),v2->events->begin(),v2->events->end());return new Value(v1->events);break;
    default: printError("Sum not defined for these types");return v1; break;
  }
}

Value *Parser::readSum() {
  Value *v=readProduct();
  Token t=lex.peekToken();
  while(t.is('+') || t.is('-')) {
    lex.consumeToken();
    if(t.is('+')) {
      v=sumValues(v,readProduct());
    } else {
      v=subValues(v,readProduct());
    }
    t=lex.peekToken();
  }
  return v;
}


Value *Parser::readExpression() {
  Value *v=readSum();
  Token t=lex.peekToken();
  if(t.is('=') && v->type==Value::IDENTIFIER) {
        lex.consumeToken();
        Value *v2=readExpression();
	if(v2) {
	  table.insert(v->str,v2);
	} else {
	  printf("pp4\n");
	}
  } 
  return v;
}

void Parser::execPrintf(vector<Value *> v) {
  const char *format=v[0]->str.c_str();
  int index=1;

  while(*format) {
    if(*format=='%') {
      string f("%");
      format++;
      if(string("#0- +'I").find(*format)!=string::npos) {
        f +=*format;
	format++;
      }
      if(isdigit(*format)){
        f +=*format;
	format++;
      }
      if(string("hl").find(*format)!=string::npos) {
        f +=*format;
	format++;
      }
      if(string("hl").find(*format)!=string::npos && *format==*(format-1)) {
        f +=*format;
	format++;
      }
      switch(*format) {
        case 'd': 
        case 'x': 
        case 'X': 
        case 'c': f +=*format;printf(f.c_str(),v[index++]->integer);break; 
	
        case 's': 
	  if(v[index]->type==Value::ARRAY) {
            for(unsigned int i=0; i<v[index]->array.len; i++){
              printf("%02hhX ",v[index]->array.data[i]);
            }
	    index++;
	  } else {
	   f +=*format;printf(f.c_str(),v[index++]->str.c_str());
	  };
	  break;
	case '%': f +=*format;printf(f.c_str());break;
	default: printError("Error in printf format");
      }
    } else if(*format=='\\') {
      char c;
      format++;
      switch(*format)
      {
        case 'n': c='\n';break;
        case 'r': c='\r';break;
	case 't': c='\t';break;
	default: c=*format;break;
      }
      printf("%c",c);
    } else {
      printf("%c",*format);
    }
    format++;
  }
}

Value *Parser::execSysFunction(SysFunction *sf) {
  vector<Value *> v;
  Token t=lex.peekToken();
  if(t.is('(')){
    lex.consumeToken();
    v=readListOfValues(')'); 
   }
  if(sf->type==SysFunction::PRINTF) {
    execPrintf(v);
    return new Value(string("error"),Value::UNDEF); 
  }
  if(v.size()!=sf->parameters.size()) {
    printError("Error in number of parameters");
  }
  for(size_t i=0; i<v.size(); i++) {
    if(v[i]->type!=sf->parameters[i] && sf->parameters[i]!=Value::UNDEF) {
      printError("Error in type of parameter %ld",i);
    }
  }
  switch(sf->type) {
    case SysFunction::OPENOUT: return new Value(seq->createOutPort(v[0]->str.c_str()), Value::OUTPORT);
    case SysFunction::OPENIN: return new Value(seq->createInPort(v[0]->str.c_str()), Value::INPORT);
    case SysFunction::CONNECT: {
                                 int client=0;
				 if(v[1]->type==Value::INTEGER) client=v[1]->integer;
				 else if(v[1]->type==Value::STRING) client=seq->searchClient(v[1]->str);
				 else printError("Integer or string types expected");
				 switch(v[0]->type){
                                 case Value::OUTPORT: seq->connect(true, v[0]->integer,client,v[2]->integer);break;
                                 case Value::INPORT: seq->connect(false, v[0]->integer,client,v[2]->integer);break;
				 default: printError("Port expected as first parameter");
				 }
			       };break;
    //case SysFunction::CONNECTOUT: seq->connect(true, v[0]->integer,v[1]->integer,v[2]->integer);break;
    //case SysFunction::CONNECTIN: seq->connect(false, v[0]->integer,v[1]->integer,v[2]->integer);break;
    case SysFunction::TEMPO: seq->set_tempo(v[0]->integer);break;
    case SysFunction::NOTE: return new Value(new vector<Event *>(1,new Event(new Note(v[0]->integer,v[1]->integer))));
    case SysFunction::PAD:  { unsigned int l=v[1]->integer;
	    if        (v[0]->str.size()==l) { return new Value(v[0]->str);
	    } else if (v[0]->str.size()<l) { return new Value(v[0]->str.append(v[1]->integer - v[0]->str.size(), ' ')); 
	    } else { return new Value(v[0]->str.substr(0,v[1]->integer));
	    }}; 
    case SysFunction::MIDI_CHANNEL:{
    checkLimits(v[1]->integer,1,16);VoiceChannel *channel=seq->newVoiceChannel(v[0]->integer, v[1]->integer-1); 
    return new Value(channel);
    }
    case SysFunction::TIME_CHANNEL:  { Channel *channel=seq->newTimeChannel(v[0]->integer); return new Value(channel); }
    case SysFunction::SYSEX_CHANNEL:  { Channel *channel=seq->newSysExChannel(v[0]->integer); return new Value(channel); }
    case SysFunction::CHANNELCC:  {checkLimits(v[1]->integer,0,127);
      Channel *channel=seq->newChannelCC(v[0]->voicechannel, v[1]->integer); 
      return new Value(channel); }
    case SysFunction::WAIT: return new Value(new vector<Event *>(1,new Event(new Silence(v[0]->integer*TICKS_PER_QUARTER))));
    case SysFunction::START: return new Value(new vector<Event *>(1,new Event(Event::START)));
    case SysFunction::STOP: return new Value(new vector<Event *>(1,new Event(Event::STOP)));
    case SysFunction::WAITSYSEX:seq->waitList.push_back(v[1]->function);break;
    case SysFunction::ADDHISTORY:add_history(v[0]->str.c_str());break;
    default: return new Value(string("error"),Value::UNDEF);
  }
  return new Value(string("error"),Value::UNDEF);
}

Value *Parser::execFunction(Function *f,vector<Value *> v) {
  table.levelUp();
  for(size_t i=0;i<v.size();i++) {
    //printf("%s = ",f->parameters[i].c_str()); v[i]->print();
    table.insert(f->parameters[i],v[i]);
  }
  lex.newStream(f->body);
  Value *val=0;
  Token t=lex.peekToken();
  while(!t.is(Token::END)) {
    val=readExpression();
    t=lex.peekToken();
    if(t.is(';') || t.is('\n')) {
      lex.consumeToken();
    }
  }  
  lex.closeStream();
  table.levelDown();
  return val;
}

Value *Parser::readValue() {
  Token t=lex.nextToken();
  if(t.is(Token::STRING)) {
    return new Value(t.value); 
  } else if(t.is(':')) {
    t=lex.expect(Token::IDENTIFIER);
    return new Value(t.value,Value::LABEL);
  } else if(t.is('/')) {
    return new Value(parse_sequence());
  } else if(t.is('(')) {
    Value *v=readExpression();
    lex.expect(")");
    return v;
  } else if(t.is(Token::NUMBER)) {
    if(t.value[0]=='0' && t.value[1]=='x') {
      return new Value(stoi(t.value,0,16)); 
    } else {
      return new Value(stoi(t.value,0,10)); 
    }
  } else if(t.is(Token::IDENTIFIER)) {
    Token op=lex.peekToken();
    if(op.is('=')) { //left part of assign
      return new Value(t.value,Value::IDENTIFIER);
    } else if(op.is(':')) { //far label, used in W(...)
      lex.expect(":");
      Value *ch=table.search(t.value);
      //if(ch->type==Value::CHANNEL || ch->type==Value::VOICECHANNEL) { //
      if(ch->type==Value::CHANNEL) { //
        t=lex.expect(Token::IDENTIFIER);
        return new Value(ch->channel,t.value);
      } else {
        printError("Channel expected");
      }
    } else {
      Value *v=table.search(t.value);
      if(v) {
        if(v->type==Value::SYS_FUNCTION) {
	  return execSysFunction(v->sysfunction);
	} else if(v->type==Value::FUNCTION) {  
	   Token t=lex.peekToken();
	   if(t.is('(')){
	     vector<Value *> parameters;
	     lex.consumeToken();
	     parameters=readListOfValues(')');
	     return execFunction(v->function,parameters);
	   } else {
	     return v;
	   }
	} else if((v->type==Value::STRING || v->type==Value::ARRAY) && op.is('[')) {  
          lex.expect("[");
	  Value *index1=readExpression();
          Token op=lex.expect("]:");
	  if(op.is(']')) {
	    if(v->type==Value::STRING) {
	      return new Value(v->str[index1->integer]);
	    } else if(v->type==Value::ARRAY) {
	      return new Value(v->array.data[index1->integer]);
	    } else {
	      printError("Type not indexable");
	    }
	  } else  { // ':'
	    Value *index2=readExpression();
	    lex.expect("]");
	    string result;
            for(int i=index1->integer;i<=index2->integer;i++) {
	      result += v->str[i];
	    }
	    return new Value(result);
	  }
	} else {
          return v;
	}
      } else {
        ::printError("Error in line %d. Unknown identifier %s",lex.lineNum(), t.value.c_str());
        return new Value(t.value,Value::UNDEF);
      //printf("no en la tabla %s\n",t.value.c_str());
      }
    }  
  }
  return new Value(t.value,Value::ERROR); ////
}

vector<string> Parser::readParameters() {
  vector<string> result;

  Token t=lex.peekToken();
  if(t.is(')')) { //empty list
    lex.consumeToken();
  }
  while(!t.is(')')) {
    t=lex.nextToken();
    result.push_back(t.value);
    t=lex.expect(",)");
  }
  return result;
}

void Parser::readDef() {
  Token t=lex.expect(Token::IDENTIFIER); //name
  string name=t.value;
  vector<string> parameters;
  t=lex.peekToken();
  if(t.is('(')) {
    lex.consumeToken();
    parameters=readParameters();
  } else {
    printError("Parameters expected");
  }
  t=lex.expect("={");
  if(t.is('{')) {
    string body;
    t=lex.nextToken();
    while(!t.is('}')) {
      if(t.is(Token::STRING)) {
        body += '"';
        body += t.value;
        body += '"';
      } else {
        body += t.value;
      }
      t=lex.nextToken();
    }
    body += '\n';
    Value *v=new Value(new Function(parameters,body));
    table.insert(name,v);
  } else {
    t=lex.nextToken();
    if(t.is(Token::STRING)) {
      string body=t.value+'\n';
      Value *v=new Value(new Function(parameters,body));
      table.insert(name,v);
    } else if(t.is(Token::IDENTIFIER)) {
      Value *v=table.search(t.value);  
      if(v->type==Value::FUNCTION || v->type==Value::SYS_FUNCTION) {
        table.insert(name,v);
      } else {
        printError("Error1 in def");
      }
    } else {
      printError("Error in def");
    }
  }
}

void Parser::readDict() {
  Value *dict=table.search("_Dict");

  Token t=lex.peekToken();
  while(!t.is('}')) {
    t=lex.nextToken();
    string var=t.value;
    lex.expect("=");
    Value *v=readExpression();
    if(v->type==Value::EVENTS) {
      dict->dictionary->insert(var,new Value(v->events));
    } else if(v->type==Value::STRING) { 
      dict->dictionary->insert(var,new Value(v->str));
    }
    t=lex.expect(";}");
  }
}

void Parser::parse(char *str) {
  lex.newStream(str);
  parse();
  lex.closeStream();
}

void Parser::parse() {
  Token t=lex.peekToken();
  
  while(!t.is(Token::END)) {
    if (t.is(Token::IDENTIFIER) && t.value.compare("def")==0){
      lex.consumeToken();
      readDef();
    } else if (t.is(Token::IDENTIFIER) && t.value.compare("DICT")==0){
      lex.consumeToken();
      lex.expect("{");
      readDict();
    } else if (t.is('{')){
      lex.consumeToken();
      readDict();
    } else if (t.is(Token::IDENTIFIER) && t.value.compare("continue")==0){
      seq->continueFlag=true;
      lex.consumeToken();
    } else if (!t.is('\n')){
      Value *v1=readExpression();
      t=lex.peekToken();
      if(t.is(Token::OUT)) {
        //if(v1->type==Value::CHANNEL || v1->type==Value::VOICECHANNEL) {
        if(v1->type==Value::CHANNEL) {
          lex.consumeToken();
          Value *v2=readExpression();
          if(v2->type==Value::EVENTS) {
            v1->channel->addSequence(v2->events);
          } else {
	    printf("Events expected\n");
	    v2->print();
          }
        } else {
	  printError("Integer or channel expected");
        }
      }
    }
    lex.expect("\n;");
    t=lex.peekToken();
  }
}

void Parser::printError ( const char * fmt, ... ){
      if(!seq->continueFlag) {
        printf("Error in line %d. ",lex.lineNum());
      }
      va_list args;
      va_start(args, fmt);
      vprintf(fmt, args);
      va_end(args);
      if(!seq->continueFlag) {
        seq->stop();
      }
}

Parser::~Parser() {
}

