#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 100


int getlineIPC(char **buffer, size_t *bufferSize, int fd){
  char c;
  int i = 0;
  int ret;

  if(*buffer == NULL || *bufferSize == 0){
    *bufferSize = BUFFER_SIZE;
    *buffer = malloc(*bufferSize * sizeof(char));
    if(*buffer == NULL){
      return -1;
    }
  }

  while((ret = read(fd, &c, 1)) > 0){
    if(c == '\n' || c == '\0'){
      break;
    }

    if(i - 2 >= *bufferSize){
      *bufferSize *= 2;
      *buffer = realloc(*buffer, *bufferSize * sizeof(char));
      if(*buffer == NULL){
        return -1;
      }
    }

    (*buffer)[i] = c;
    i++;
  }

  if(ret == -1){
    return -1;
  }

  (*buffer)[i] = '\0';
  return i;
}

int writelineIPC(int fd, char *str){
  int i = 0;
  while(str[i] != '\n' && str[i] != '\0'){
    i++;
  }
  if(write(fd, str, i + 1) != i + 1){
    return -1;
  }
  return i;
}
