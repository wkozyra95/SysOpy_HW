#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>


void * correct_thread(void *);
void * foulty_thread(void *);


int main(){
  pthread_t dw1,dw2,zw;

  pthread_create(&dw1, NULL, correct_thread, NULL);
  pthread_create(&dw2, NULL, correct_thread, NULL);
  pthread_create(&zw, NULL, foulty_thread, NULL);

  pthread_join(dw1,NULL);
  pthread_join(dw2,NULL);
  pthread_join(zw,NULL);

  return 0;
}

void * foulty_thread(void * data){
  int a=1;
  int b=0;
  int c;
  while(1){
    puts("divide by 0");
    c=a/b;
    printf("%d\n",c);
  }
}

void * correct_thread(void * data){
  
  while(1){  
    puts(".");
  }
}
