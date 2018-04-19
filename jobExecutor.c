#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "ipc.h"

//File descriptor positions
#define READ 0
#define WRITE 1

int main(int argc, char * const *argv){
  char str[100];

  //Get command line arguments
  int c;
  char *docFile = NULL;
  int w = 0;

  while((c = getopt(argc, argv, "d:w:")) != -1){
    switch(c){
      case 'd':
        docFile = optarg;
        break;
      case 'w':
        w = atoi(optarg);
        break;
    }
  }
  //Check arguments
  if(docFile == NULL || w == 0){
    fprintf(stderr, "Usage: ./jobExecutor -d <docfile> -w <workerCount>\n");
    return 1;
  }
  else if(w <= 0){
    fprintf(stderr, "Error: w must be greater than 0\n");
    return 1;
  }

  //Create folders for fifos and logs
  if(mkdir("./fifos", 0774) != 0){
    perror("Error creating fifos directory");
    return 1;
  }
  if(mkdir("./logs", 0774) != 0){
    perror("Error creating logs directory");
    return 1;
  }

  //Store child processes' pids
  pid_t *wpid = malloc(w * sizeof(pid_t));
  if(wpid == NULL){
    fprintf(stderr, "Memory allocation error\n");
    return 1;
  }

  //Create workers and fifos for each worker
  int **fifo = malloc(w * sizeof(int*));
  if(fifo == NULL){
    fprintf(stderr, "Memory allocation error\n");
    return 1;
  }

  for(int i = 0; i < w; i++){
    sprintf(str, "./fifos/to_w%d.fifo", i);
    if(mkfifo(str, 0774) != 0){
      perror("Error creating fifo");
      //TODO: Handle error
    }
    sprintf(str, "./fifos/from_w%d.fifo", i);
    if(mkfifo(str, 0774) != 0){
      perror("Error creating fifo");
      //TODO: Handle error
    }

    if((wpid[i] = fork()) == 0){
      sprintf(str, "%d", i);
      execlp("./worker", "worker", str, NULL);
    }

    //Open fifos
    fifo[i] = malloc(2 * sizeof(int));
    if(fifo[i] == NULL){
      fprintf(stderr, "Memory allocation error\n");
      return 1;
    }

    sprintf(str, "./fifos/to_w%d.fifo", i);
    if((fifo[i][WRITE] = open(str,O_WRONLY)) == -1){
      perror("Error opening fifo");
      //TODO: Handle error
    }
    sprintf(str, "./fifos/from_w%d.fifo", i);
    if((fifo[i][READ] = open(str,O_RDONLY)) == -1){
      perror("Error opening fifo");
      //TODO: Handle error
    }
  }

  //Give directories to workers
  FILE *docFd = fopen(docFile, "r");
  if(docFd == NULL){
    perror("Error opening docfile");
    return 1;
  }
  char *buffer = NULL;
  size_t bufferSize = 0;

  for(int i = 0; getline(&buffer, &bufferSize, docFd) != -1; i++){
    if(writelineIPC(fifo[i % w][WRITE], buffer) == -1){
      perror("Error sending dir to worker");
    }
    //Get confirmation from worker
    if(getlineIPC(&buffer, &bufferSize, fifo[i % w][READ]) == -1){
      perror("Error receiving msg from worker");
    }
    //Check confirmation
    if(!strcmp(buffer, "OK") == 0){
      fprintf(stderr, "Error: No confirmation received\n");
    }
  }

  if(fclose(docFd) == EOF){
    perror("Error closing docfile");
  }

  for(int i = 0; i < w; i++){
    //Tell workers that all directories are sent
    sprintf(str, "STOP");
    writelineIPC(fifo[i][WRITE], str);

    //Wait for confirmation that worker is ready
    if(getlineIPC(&buffer, &bufferSize, fifo[i][READ]) == -1){
      perror("Error receiving msg from worker");
    }
    //Check confirmation
    if(!strcmp(buffer, "READY") == 0){
      fprintf(stderr, "Error: No ready confirmation received\n");
    }
  }

  //Command Line Interface
  char *cmd[12]; //A command can have 11 words at most
  char *cp;
  char file[100], bestFile[100];

  printf(">/");
  while(getline(&buffer, &bufferSize, stdin) > 0){
    cmd[0] = strtok(buffer, " \t\n"); //Get command
    for(int i = 1; i <= 10; i++){
      cmd[i] = strtok(NULL, " \t\n"); //Get command parameters
    }
    cmd[11] = NULL;


    if(strcmp(cmd[0], "exit") == 0){
      break;
    }
    else if(strcmp(cmd[0], "maxcount") == 0){
      if(cmd[1] == NULL){
        fprintf(stderr, "Error: No word given\n");
      }
      else{
        //Send command to workers
        sprintf(buffer, "%s %s\n", cmd[0], cmd[1]); // "maxcount <word>"
        for(int i = 0; i < w; i++){
          writelineIPC(fifo[i][WRITE], cmd[0]);
        }

        //Get results from workers
        int max = 0;
        int cur;

        for(int i = 0; i < w; i++){
          if(getlineIPC(&buffer, &bufferSize, fifo[i][READ]) == -1){
            perror("Error receiving msg from worker");
          }
          cp = strtok(buffer, " \n");
          cur = atoi(cp);
          if(cur != 0){
            cp = strtok(NULL, "\n");
            strcpy(file, cp);
          }

          if(cur > max){
            max = cur;
            strcpy(bestFile, file);
          }
        }

        //Print file with most appearances
        if(max == 0){
          printf("Word not found\n");
        }
        else{
          printf("%d %s\n", max, bestFile);
        }
      }
    }
    else if(strcmp(cmd[0], "mincount") == 0){
      if(cmd[1] == NULL){
        fprintf(stderr, "Error: No word given\n");
      }
      else{
        //Send command to workers
        sprintf(buffer, "%s %s\n", cmd[0], cmd[1]); // "maxcount <word>"
        for(int i = 0; i < w; i++){
          writelineIPC(fifo[i][WRITE], cmd[0]);
        }

        //Get results from workers
        int min = 0;
        int cur;

        for(int i = 0; i < w; i++){
          if(getlineIPC(&buffer, &bufferSize, fifo[i][READ]) == -1){
            perror("Error receiving msg from worker");
          }
          cp = strtok(buffer, " \n");
          cur = atoi(cp);
          if(cur != 0){
            cp = strtok(NULL, "\n");
            strcpy(file, cp);
          }
          if(cur > 0 && (cur < min || min == 0)){
            min = cur;
            strcpy(bestFile, file);
          }
        }

        //Print file with most appearances
        if(min == 0){
          printf("Word not found\n");
        }
        else{
          printf("%d %s\n", min, bestFile);
        }
      }
    }
    else if(strcmp(cmd[0], "wc") == 0){
      //Send command to workers
      for(int i = 0; i < w; i++){
        writelineIPC(fifo[i][WRITE], cmd[0]);
      }

      //Get results from workers
      int chars = 0;
      int words = 0;
      int texts = 0;

      for(int i = 0; i < w; i ++){
        if(getlineIPC(&buffer, &bufferSize, fifo[i][READ]) == -1){
          perror("Error receiving msg from worker");
        }

        cp = strtok(buffer, " ");
        chars += atoi(cp);

        cp = strtok(NULL, " ");
        words += atoi(cp);

        cp = strtok(NULL, "\n");
        texts += atoi(cp);
      }

      printf("%d %d %d\n", chars, words, texts);
    }
    else if(strcmp(cmd[0], "help") == 0){
      printf("jobExecutor commands:\n");
      printf("\t-help\n");
      printf("\t-exit\n");
    }
    else{
      fprintf(stderr, "Error: Not a valid command! Type 'help' to see available commands\n");
    }

    printf(">/");
  }

  //Tell workers to stop
  sprintf(str, "STOP");
  for(int i = 0; i < w; i++){
    writelineIPC(fifo[i][WRITE], str);
  }

  //Close and delete fifos
  for(int i = 0; i < w; i++){
    //Close
    if(close(fifo[i][READ]) != 0){
      perror("Error closing fifo");
      //TODO: Handle error
    }
    if(close(fifo[i][WRITE]) != 0){
      perror("Error closing fifo");
      //TODO: Handle error
    }

    //Free memory
    free(fifo[i]);

    //Delete
    sprintf(str, "./fifos/to_w%d.fifo", i);
    if(remove(str) != 0){
      perror("Error deleting fifo");
      //TODO: Handle error
    }
    sprintf(str, "./fifos/from_w%d.fifo", i);
    if(remove(str) != 0){
      perror("Error deleting fifo");
      //TODO: Handle error
    }
  }
  if(remove("./fifos") != 0){
    perror("Error deleting fifo folder");
    //TODO: Handle error
  }

  if(remove("./logs") != 0){
    perror("Error deleting fifo folder");
    //TODO: Handle error
  }

  //Free allocated memory
  free(wpid);
  free(fifo);
  free(buffer);

  return 0;
}
