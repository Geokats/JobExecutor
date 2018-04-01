#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>

#include "ipc.h"


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
  }

  printf("Worker %d started!\n", wnum);

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

  //Get directories
  char *buffer = NULL;
  size_t bufferSize = 0;

  while(1){
    if(getlineIPC(&buffer, &bufferSize, fifo[READ]) == -1){
      perror("Error getting message from executor");
    }

    if(strcmp(buffer, "STOP") == 0){
      break;
    }

    printf("worker #%d got dir: %s\n", wnum, buffer);

    //Open dir
    DIR *dir = opendir(buffer);
    if(dir == NULL){
      perror("Error opening directory");
    }
    //Iterate over dir's files
    struct dirent *dirEntry;
    while((dirEntry = readdir(dir)) != NULL){
      if(dirEntry->d_type == DT_REG){
        //If the entry is a regular file
        printf("%s\n", dirEntry->d_name);
      }
    }
    closedir(dir);

    //Send confirmation
    writelineIPC(fifo[WRITE], "OK");
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

  free(buffer);

  return 0;
}
