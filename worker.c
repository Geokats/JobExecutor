#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>

#include "ipc.h"
#include "textIndex.h"
#include "postingList.h"
#include "trie.h"


//File descriptor positions
#define READ 0
#define WRITE 1

/****************************** Helping Functions *****************************/

int strmcat(char **dest, char *src, size_t *destSize){
  if(strlen(*dest) + strlen(src) + 1 > *destSize){
    *destSize += strlen(src) + 10;
    *dest = realloc(*dest, *destSize * sizeof(char));
    if(*dest == NULL){
      return -1;
    }
  }
  strcat(*dest, src);
  return 1;
}

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
  time_t rawTime;
  struct tm *logtime;
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

  //If worker has no files then it must stop :(
  if(filesCount <= 0){
    fprintf(stderr, "Error: Worker %d has no files\n", wnum);
    return 1;
  }

  //Create log file
  sprintf(str, "./logs/w%d_%d.log", wnum, getpid());
  FILE *logFd = fopen(str, "w");
  if(logFd == NULL){
    perror("Error creating log file");
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

  //Inform executor that you're ready to receive a command
  writelineIPC(fifo[WRITE], "READY");

  char *cmd[12]; //A command can have 11 words at most
  postingList *pl;
  int appearances;
  int fileIndex;

  //Get command from executor
  while(1){
    if(getlineIPC(&buffer, &bufferSize, fifo[READ]) == -1){
      perror("Error getting message from executor");
    }
    else{
      cmd[0] = strtok(buffer, " \t\n"); //Get command
      for(int i = 1; i <= 10; i++){
        cmd[i] = strtok(NULL, " \t\n"); //Get command parameters
      }
      cmd[11] = NULL;
    }

    time(&rawTime);
    logtime = localtime(&rawTime);

    if(strcmp(cmd[0], "STOP") == 0){
      break;
    }
    else if(strcmp(cmd[0], "search") == 0){
      postingList *resPL = createPL();
      postingList *pl;

      //Get all text that have atleast one of the keywords
      for(int i = 1; cmd[i] != NULL; i++){
        pl = searchWordTrie(t, cmd[i]);
        fprintf(logFd, "%d-%d-%d %02d:%02d:%02d : search : %s"
                     ,logtime->tm_mday, logtime->tm_mon, logtime->tm_year + 1900
                     ,logtime->tm_hour, logtime->tm_min, logtime->tm_sec
                     ,cmd[i]);

        if(pl != NULL){
          plNode *pln = getStartPL(pl);
          while(pln != NULL){
            fprintf(logFd, " : %s", files[getFileIndexPLN(pln)]);

            addAppearancePL(resPL, getFileIndexPLN(pln), getTextIndexPLN(pln));
            pln = getNextPLN(pln);
          }
        }
        fprintf(logFd, "\n");
      }

      //Send texts to jobExecutor
      plNode *pln = getStartPL(resPL);
      while(pln != NULL){
        sprintf(buffer, "%s %d ", files[getFileIndexPLN(pln)], getTextIndexPLN(pln));
        strmcat(&buffer, getTextTI(ti[getFileIndexPLN(pln)], getTextIndexPLN(pln)), &bufferSize);

        writelineIPC(fifo[WRITE], buffer);

        pln = getNextPLN(pln);
      }

      sprintf(buffer, "STOP");
      writelineIPC(fifo[WRITE], buffer);
      deletePL(resPL);
    }
    else if(strcmp(cmd[0], "maxcount") == 0){
      fprintf(logFd, "%d-%d-%d %02d:%02d:%02d : maxcount : %s"
                   ,logtime->tm_mday, logtime->tm_mon, logtime->tm_year + 1900
                   ,logtime->tm_hour, logtime->tm_min, logtime->tm_sec
                   ,cmd[1]);

      pl = searchWordTrie(t, cmd[1]);
      if(pl == NULL){
        sprintf(buffer, "0\n");
      }
      else{
        fileIndex = getMaxcountFilePL(pl, &appearances);
        sprintf(buffer, "%d %s\n", appearances, files[fileIndex]);
      }

      if(pl != NULL){
        fprintf(logFd, " : %s\n", files[fileIndex]);
      }
      else{
        fprintf(logFd, "\n");
      }

      //Send result to jobExecutor
      writelineIPC(fifo[WRITE], buffer);
    }
    else if(strcmp(cmd[0], "mincount") == 0){
      pl = searchWordTrie(t, cmd[1]);
      fprintf(logFd, "%d-%d-%d %02d:%02d:%02d : mincount : %s"
                   ,logtime->tm_mday, logtime->tm_mon, logtime->tm_year + 1900
                   ,logtime->tm_hour, logtime->tm_min, logtime->tm_sec
                   ,cmd[1]);

      if(pl == NULL){
        sprintf(buffer, "0\n");
      }
      else{
        fileIndex = getMincountFilePL(pl, &appearances);
        sprintf(buffer, "%d %s\n", appearances, files[fileIndex]);
      }

      if(pl != NULL){
        fprintf(logFd, " : %s\n", files[fileIndex]);
      }
      else{
        fprintf(logFd, "\n");
      }

      //Send result to jobExecutor
      writelineIPC(fifo[WRITE], buffer);
    }
    else if(strcmp(cmd[0], "wc") == 0){
      int chars = 0;
      int words = 0;
      int texts = 0;

      for(int i = 0; i < filesCount; i++){
        chars += getCharCountTI(ti[i]);
        words += getWordCountTI(ti[i]);
        texts += getTextCountTI(ti[i]);
      }

      //Send result to jobExecutor
      sprintf(buffer, "%d %d %d\n", chars, words, texts);
      writelineIPC(fifo[WRITE], buffer);

      //
      fprintf(logFd, "%d-%d-%d %02d:%02d:%02d : wc : %d : %d : %d\n"
                   ,logtime->tm_mday, logtime->tm_mon, logtime->tm_year + 1900
                   ,logtime->tm_hour, logtime->tm_min, logtime->tm_sec
                   ,chars, words, texts);
    }
  }

  //Close fifos
  if(close(fifo[READ]) != 0){
    perror("Error closing fifo");
    //TODO: Handle error
  }
  if(close(fifo[WRITE]) != 0){
    perror("Error closing fifo");
    //TODO: Handle error
  }

  //Close log file
  fclose(logFd);

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
