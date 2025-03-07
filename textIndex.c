#include "textIndex.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define INIT_TEXT_SIZE 100
#define INIT_STRING_LENGTH 50

/****************************** Helping Functions *****************************/

void countWordsAndChars(char *str, int *wordCount, int *charCount){
  int i;
  int count = 0;
  int afterSpace = 1;

  for(i = 0; str[i] != '\0'; i++){
    if(isspace(str[i])){
      afterSpace = 1;
    }
    else if(afterSpace){
      //If we find a non-space char after space then we have a word
      count++;
      afterSpace = 0;
    }
  }

  *charCount = i;
  *wordCount = count;
}

/********************************* Text Index *********************************/

struct textIndex{
  char **text;
  int textCount;
  int wordCount;
  int charCount;
};

// Constructor

textIndex *createTI(const char *fileName){
  textIndex *ti = malloc(sizeof(textIndex));
  if(ti == NULL){
    return NULL;
  }

  int textSize = INIT_TEXT_SIZE; //Number of available pointers
  ti->textCount = 0; //Number of used pointers
  ti->wordCount = 0;
  ti->charCount = 0;

  ti->text = malloc(textSize * sizeof(char*));
  //TODO: Error checking

  FILE *fp = fopen(fileName, "r");
  if(fp == NULL){
    fprintf(stderr, "Error opening file\n");
    return NULL;
  }

  int textIndex = 0;
  int charIndex = 0;
  int curLength = 0;

  char c;
  while((c = fgetc(fp)) != EOF){
    if(c == '\n'){
      if(charIndex == 0){
        continue; //Ignore empty lines
      }

      ti->text[textIndex][charIndex] = '\0';

      //Count words and chars in new text
      int words, chars;
      countWordsAndChars(ti->text[textIndex], &words, &chars);

      //Update char, word and text count
      ti->charCount += chars;
      ti->wordCount += words;
      ti->textCount += 1;

      //Go to next line
      textIndex++;

      if(textIndex == textSize){
        //If necessary increase array's size
        textSize *= 2;
        ti->text = realloc(ti->text, textSize * sizeof(char*));
        if(ti->text == NULL){
          return NULL;
        }
      }

      curLength = 0;
      charIndex = 0;
    }
    else{
      if(curLength == 0){
        curLength = INIT_STRING_LENGTH;
        ti->text[textIndex] = malloc(curLength * sizeof(char));
        if(ti->text[textIndex] == NULL){
          return NULL;
        }
      }
      else if(charIndex + 1 == curLength){
        //If necessary increase string's length
        //+1 is to make sure we always have an extra position for '\0'
        curLength *= 2;
        ti->text[textIndex] = realloc(ti->text[textIndex], curLength * sizeof(char));
        if(ti->text[textIndex] == NULL){
          return NULL;
        }
      }

      ti->text[textIndex][charIndex] = c;
      charIndex++;
    }
  }

  fclose(fp);
  return ti;
}

// Destructor

void deleteTI(textIndex *ti){
  for(int i = 0; i < ti->textCount; i++){
    free(ti->text[i]);
  }
  free(ti->text);
  free(ti);
}

// Accessors

char *getTextTI(textIndex *ti, int index){
  return ti->text[index];
}

int getWordCountTI(textIndex *ti){
  return ti->wordCount;
}

int getTextCountTI(textIndex *ti){
  return ti->textCount;
}

int getCharCountTI(textIndex *ti){
  return ti->charCount;
}
