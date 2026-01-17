//main.cpp
#include <iostream>

#include <stdio.h>
//#include <readline/readline.h>
//#include <readline/history.h>

#include <signal.h>
#include "parser.h"
#include "sequencer.h"


bool inputAvailable()
{
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  return (FD_ISSET(0, &fds));
}

int main(int argc, char *argv[]) {
  if(argc!=2) {
    printf("error expected 1 argument\n");
    exit(0);
  }
  Sequencer seq;
  Parser p(&seq,argv[1]);
  seq.parser=&p;
  seq.open_seq("default","seq");
  seq.init_queue();
  p.parse();
  seq.start_queue();
  signal(SIGINT, seq.sigterm_exit);
  signal(SIGTERM, seq.sigterm_exit);
  seq.send_sequences();
  while(1) {
      seq.loop();
  }
  if(seq.continueFlag) {
  /*
    if(freopen("/dev/tty","r",stdin)) {
      cin.clear();
    }  
    */
    //stdin=fopen("/dev/tty","r");
    //cin.clear();
    while(1) {
      
      if(0 && inputAvailable()) {
      printf("p3\n");
        //char *line=readline (">");
	char *line=NULL;
	size_t size;
	 if(!getline(&line, &size, stdin)) {
	   printf("no get line\n");
	 }
      printf("p4\n");
        if(line) {
          printf("-%s\n",line);
          if(line[0]=='.') exit(0);
      printf("p5\n");
          p.parse(line);
      printf("p6\n");
          free(line);
      printf("p7\n");
          seq.send_sequences();
      printf("p8\n");
        }
      } else {
      seq.loop();
      }
      
    }
  }
  printf("end main\n");
}


