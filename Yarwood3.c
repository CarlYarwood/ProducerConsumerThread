#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include "q.h"
//Project 3
//@author Carl Yarwood
//last edited 4/30/2018
Q *queue;
pthread_mutex_t* queueLock;
pthread_cond_t* notEmpty;
int isVowel(char c);
void* Producer(void *);
void* Consumer(void *);
struct inProducer{
  char* fileName;
};
int main(int argc, char **argv){
  //checking argumments
  if(argc != 2){
    printf("invalid input, please try again,\nnext time includeing the file name you would like read \n");
    return 1;
  }
  //declarations and initalizations
  queue = (Q*)malloc(sizeof(Q));
  notEmpty = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
  queueLock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
  pthread_t *producer = (pthread_t*)malloc(sizeof(pthread_t));
  pthread_t *consumer = (pthread_t*)malloc(sizeof(pthread_t));
  char **inPro = (char**)malloc(sizeof(char*));
  *inPro = argv[1];
  pthread_mutex_init(queueLock,NULL);
  pthread_cond_init(notEmpty,NULL);
  init_q(queue);
  //createing threads
  pthread_create(producer,NULL,Producer,(void*)inPro);
  pthread_create(consumer,NULL,Consumer,NULL);
  //joining threads
  pthread_join(*producer,NULL);
  pthread_join(*consumer,NULL);
  return 0;
}
//fucntion run by the producer thread, reads in file
//and passes contents to the consumer through a shared buffer
void* Producer(void *inpro){
  char *fileName = *(char **)inpro;
  //note I chose two newlines as my end flag as with how the
  //program is written, it is very unlikely if not impossible
  //(I'm sure someone cleverer than me could figure it out)
  //for my buffer to read in two newlines from the same line
  char *end = "\n\n";
  FILE *fp = fopen(fileName, "r");
  //determin if file can be found
  if(fp == NULL){
    printf("Error could not find file %s\n", fileName);
    pthread_mutex_lock(queueLock);
    enqueue(queue,(void *)end);
    pthread_mutex_unlock(queueLock);
    pthread_cond_broadcast(notEmpty);
    return NULL;
  }
  if( fseek(fp,0,SEEK_END)){
    printf("Error, seek on file failed\n");
    pthread_mutex_lock(queueLock);
    enqueue(queue,(void *)end);
    pthread_mutex_unlock(queueLock);
    pthread_cond_broadcast(notEmpty);
    return NULL;
  }
  //determines if a file is empty
  if( !ftell(fp)){
    fclose(fp);
    printf("ftell failed");
    pthread_mutex_lock(queueLock);
    enqueue(queue,(void *)end);
    pthread_mutex_unlock(queueLock);
    pthread_cond_broadcast(notEmpty);
    return NULL;
  }
  // resets file
  fseek(fp,0,SEEK_SET);
  //start producing
  while(!feof(fp)){
    //reading line from file
    size_t bufferSize = 256;
    char *line = (char*)malloc(sizeof(char)*bufferSize);
    getline(&line, &bufferSize, fp);
    //removing newline at end of line
    line[strlen(line)-1] = 0;
    //putting line in queue and waking consumer
    pthread_mutex_lock(queueLock);
    enqueue(queue, (void *)line);
    pthread_mutex_unlock(queueLock);
    pthread_cond_broadcast(notEmpty);
  }
  //putting end flag in queue
  enqueue(queue, (void*)end);
  return NULL;
}
//function run by consumer thread
//reads in contents from shared buffer with
//producer and proccesses what it reads in
void* Consumer(void *incon){
  char *end = "\n\n";
  //printing initial statemnt
  printf("vowe cons non- word: theline\n---- ---- ---- ----\n");
  //initalizing in value
  char *in = NULL;
  //processing queue note I decide to read and give output to empty lines
  //I did not do this initially but I came to the conclution that they are
  //lines in the file and should be processed as such
  while(1){
    //reading in form queue
    pthread_mutex_lock(queueLock);
    while(!size(queue)){
      pthread_cond_wait(notEmpty,queueLock);
    }
    in = (char*)dequeue(queue);
    pthread_mutex_unlock(queueLock);
    //checking for end flag and if not processing
    if(strcmp(in,end)){
      //these values are to count occurences
      int num = 0;
      int vowe = 0;
      int cons = 0;
      int non = 0;
      int word = 0;
      int cluster = 0;
      int size = strlen(in);
      for(int i = 0; i<size ; i++){
	if(isalnum(in[i])){
	  cluster = 1;
	}
	if(isalpha(in[i])){
	  if(isVowel(in[i])){
	    vowe++;
	  }
	  else{
	    cons++;
	  }
	}
	else{
	  non++;
	}
	if(isspace(in[i])){
	  if(cluster){
	    word++;
	    cluster = 0;
	  }
	}
	if(isdigit(in[i])){
	  num = 1;
	}
      }
      if(vowe + cons+ num > 0){
        word++;
      }
      printf("%4d %4d %4d %4d: %s\n", vowe, cons, non, word, in);
      vowe = 0;
      cons = 0;
      non = 0;
      num = 0;
      word = 0;
    }
    //exiting file on the end being passed
    else{
      return NULL;
    }
  }
}
//checking for vowels
int isVowel(char in){
  switch(in){
  case 'a':
  case 'A':
  case 'e':
  case 'E':
  case 'i':
  case 'I':
  case 'o':
  case 'O':
  case 'u':
  case 'U':
    return 1;
    break;
  default:
    return 0;
    break;
  }
}
