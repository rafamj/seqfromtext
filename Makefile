CC = g++
CFLAGS =  -O2 -Wall -std=c++11
OBJECTS = sequencer.o parser.o token.o main.o lexer.o value.o table.o sequence.o note.o event.o chord.o silence.o error.o channel.o voicechannel.o systemchannel.o timechannel.o sysex.o label.o channelcc.o midichannel.o
#LIBS = -lasound -lreadline
LIBS = -lasound 

all: seqfromtext

seqfromtext: $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o seqfromtext $(LIBS) 

.cpp.o:
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm $(OBJECTS) seqfromtext
