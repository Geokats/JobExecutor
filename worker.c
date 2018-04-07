#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>

#include "ipc.h"
#include "textIndex.h"
#include "postingList.h"
#include "trie.h"


//File descriptor positions
#define READ 0
#define WRITE 1

/****************************** Helping Functions *****************************/

int getFiles(char* dirName, char ***files, int *filesCount, int *filesSize){
  //Allocate memory if files array hasn't been allocated
  if(*files == NULL || filesSize == 0){
    *filesCount = 0;
    *filesSize = 10;
    *files = malloc(*filesSize * sizeof(char**));
    if(*files == NULL){
      perror("Memory allocation error");
      //TODO: Handle error
    }
  }

  //Open dir
  DIR *dir = opendir(dirName);
  if(dir == NULL){
    perror("Error opening directory");
  }
  //Iterate over dir's files
  struct dirent *dirEntry;
  while((dirEntry = readdir(dir)) != NULL){
    //If the entry is a regular file
    if(dirEntry->d_type == DT_REG){
      //If needed allocate more memory
      if(*filesCount >= *filesSize){
        *filesSize *= 2;
        *files = realloc(*files, *filesSize * sizeof(char**));
        if(*files == NULL){
          perror("Memory allocation error");
          //TODO: Handle error
        }
      }
      (*files)[*filesCount] = malloc((strlen(dirName) + strlen(dirEntry->d_name) + 2) * sizeof(char));
      if((*files)[*filesCount] == NULL){
        perror("Memory allocation error");
        //TODO: Handle error
      }
      strcpy((*files)[*filesCount], dirName);
      strcpy((*files)[*filesCount] + strlen(dirName), "/");
      strcpy((*files)[*filesCount] + strlen(dirName) + 1, dirEntry->d_name);
      *filesCount += 1;
    }
  }
  closedir(dir);
}

/*********************************** Worker ***********************************/

int main(int argc, char const *argv[]) {
  int wnum;
  char str[100];

  //Get worker number
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

  //Get directories from jobExecutor and extract the files
  char *buffer = NULL;
  size_t bufferSize = 0;

  char **files = NULL;
  int filesCount,filesSize;

  while(1){
    if(getlineIPC(&buffer, &bufferSize, fifo[READ]) == -1){
      perror("Error getting message from executor");
    }
    if(strcmp(buffer, "STOP") == 0){
      break;
    }

    //Get files from directory and save them
    getFiles(buffer, &files, &filesCount, &filesSize);

    //Send confirmation
    writelineIPC(fifo[WRITE], "OK");
  }

  //Create text indices from assigned files
  textIndex **ti = malloc(filesCount * sizeof(textIndex*));
  if(ti == NULL){
    perror("Memory allocation error");
  }

  for(int i = 0; i < filesCount; i++){
    ti[i] = createTI(files[i]);
    if(ti[i] == NULL){
      perror("Error creating text index");
    }
  }

  //Create trie from text indices
  trie *t = createTrie();
  if(t == NULL){
    perror("Error creating trie");
  }

  for(int i = 0; i < filesCount; i++){
    if(insertTextIndexTrie(t, ti[i], i) == 0){
      perror("Error inserting text index in trie");
    }
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

  //Free allocated memory
  free(buffer);
  for(int i = 0; i < filesCount; i++){
    deleteTI(ti[i]);
    free(files[i]);
  }
  free(ti);
  free(files);
  deleteTrie(t);

  return 0;
}
