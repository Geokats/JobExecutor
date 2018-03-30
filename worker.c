#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


//File descriptor positions
#define READ 0
#define WRITE 1

int main(int argc, char const *argv[]) {
  int wnum;
  char str[100];

  if(argc != 2){
    fprintf(stderr, "Error: Excpected worker number\n");
    return 1;
  }
  else{
    wnum = atoi(argv[1]);
    printf("Worker %d started!\n", wnum);
  }


  //Open fifos
  int fifo[2];
  sprintf(str, "./fifos/to_w%d.fifo", wnum);
  if((fifo[READ] = open(str,O_RDONLY)) == -1){
    perror("Error opening fifo");
    //TODO: Handle error
  }
  sprintf(str, "./fifos/from_w%d.fifo", wnum);
  if((fifo[WRITE] = open(str,O_WRONLY)) == -1){
    perror("Error opening fifo");
    //TODO: Handle error
  }

  printf("Worker %d ready!\n", wnum);


  //Close fifos
  if(close(fifo[READ]) != 0){
    perror("Error closing fifo");
    //TODO: Handle error
  }
  if(close(fifo[WRITE]) != 0){
    perror("Error closing fifo");
    //TODO: Handle error
  }

  return 0;
}
