//main.cpp
#include <iostream>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "parser.h"
#include "sequencer.h"
#include "error.h"


sem_t sem;
Sequencer seq;
bool exitFlag=false;

void *thread(void *arg) {
  char *ret;
  //printf("thread() entered with argument '%s'\n",(char *)arg);
  if ((ret = (char*) malloc(20)) == NULL) {
    perror("malloc() error");
    exit(2);
  }
  //strcpy(ret, "This is a test");
  while(!exitFlag) {
      sem_wait(&sem);
      seq.loop();
      sem_post(&sem);
      usleep(100);
      sem_wait(&sem);
      seq.send_events();
      sem_post(&sem);
  }
  
  pthread_exit(ret);
}



void sigterm_exit(int sig) {
  printf("sigterm_exit\n");
  seq.stop();
  exit(0);
}


int main(int argc, char *argv[]) {
  

if(argc!=2) {
    printf("error expected 1 argument\n");
    exit(0);
  }
  Parser p(&seq,argv[1]);
  seq.parser=&p;
  seq.open_seq("default","seq");
  seq.init_queue();
  seq.start_queue();
  p.parse();
  signal(SIGINT, sigterm_exit);
  signal(SIGTERM, sigterm_exit);
  
  if(seq.continueFlag) {
 pthread_t thid;
  void *ret;
  if (sem_init(&sem, 0, 1) == -1) printError("Error creating semaphore\n");
  if (pthread_create(&thid, NULL, thread, (void *)"thread 1") != 0) {
    perror("pthread_create() error");
    exit(1);
  }
    while(1) {
        char *line=readline (">");
        if(line) {
          if (line[0]) {
            if(line[0]=='.') {
              free(line);
	      break;
	    }
	    add_history(line);
	    sem_wait(&sem);
	    p.parse(line);
	    sem_post(&sem);
            usleep(100);
	    sem_wait(&sem);
            seq.send_sequences();
	    sem_post(&sem);
	  }
          free(line);
	}
    }
  seq.stop();
  exitFlag=true;
  if (pthread_join(thid, &ret) != 0) {
    perror("pthread_create() error");
    exit(3);
  }

  } else {
  while(1) {
      seq.loop();
      seq.send_events();
  }
 }
}


