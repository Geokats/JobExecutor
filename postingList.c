#include "postingList.h"

#include <stdlib.h>
#include <stdio.h>

struct plNode{
  int fileIndex;
  int textIndex;
  int appearanceCount;

  plNode *next;
};

struct postingList{
  plNode *start;
  int size;
};

/****************************** Posting List Node *****************************/

plNode *createPLN(int fileIndex, int textIndex, int count){
  plNode *pln = malloc(sizeof(plNode));
  if(pln == NULL){
    return NULL;
  }

  pln->fileIndex = fileIndex;
  pln->textIndex = textIndex;
  pln->appearanceCount = count;
  pln->next = NULL;

  return pln;
}

void deletePLN(plNode *pln){
  free(pln);
}

plNode *getNextPLN(plNode *pln){
  return pln->next;
}

int getFileIndexPLN(plNode *pln){
  return pln->fileIndex;
}

int getTextIndexPLN(plNode *pln){
  return pln->textIndex;
}

int getCountPLN(plNode *pln){
  return pln->appearanceCount;
}

/******************************** Posting List ********************************/

postingList *createPL(){
  postingList *pl = malloc(sizeof(postingList));
  if(pl == NULL){
    return NULL;
  }

  pl->start = NULL;
  pl->size = 0;

  return pl;
}

void deletePL(postingList *pl){
  plNode *pln = pl->start;
  plNode *next;

  while(pln != NULL){
    next = pln->next;
    deletePLN(pln);
    pln = next;
  }

  free(pl);
}

int addAppearancePL(postingList *pl, int fileIndex, int textIndex){
  plNode *prev = NULL;
  plNode *pln = pl->start;
  while(pln != NULL){
    if(getFileIndexPLN(pln) > fileIndex){
      break;
    }
    else if(getFileIndexPLN(pln) == fileIndex && getTextIndexPLN(pln) >= textIndex){
      break;
    }
    else{
      pln = getNextPLN(pln);
    }
  }


  if(pln != NULL && getFileIndexPLN(pln) == fileIndex && getTextIndexPLN(pln) == textIndex){
    pln->appearanceCount += 1;
  }
  else{
    plNode *new = createPLN(fileIndex, textIndex, 1);
    if(new == NULL){
      return -1;
    }
    pl->size += 1;

    if(prev == NULL){
      //The new node will be added at the start of the list
      new->next = pl->start;
      pl->start = new;
    }
    else{
      new->next = pln;
      prev->next = new;
    }
  }
  return 1;
}

int getSizePL(postingList *pl){
  return pl->size;
}

int getTotalAppearancesPL(postingList *pl){
  int total = 0;

  plNode *pln = pl->start;
  while(pln != NULL){
    total += getCountPLN(pln);
    pln = getNextPLN(pln);
  }

  return total;
}

void printPL(postingList *pl){
  plNode *pln = pl->start;
  printf("[ ");
  while(pln != NULL){
    printf("(%d,%d,%d) ", getFileIndexPLN(pln), getTextIndexPLN(pln), getCountPLN(pln));
    pln = getNextPLN(pln);
  }
  printf("]");
}

int getMaxcountFilePL(postingList *pl, int *appearances){
  int curFile = -1;
  int maxFile = -1;
  int curCount = 0;
  int maxCount = 0;

  plNode *pln = pl->start;
  while(pln != NULL){
    if(curFile == -1){
      curFile = getFileIndexPLN(pln);
      curCount = getCountPLN(pln);
    }
    else if(curFile != getFileIndexPLN(pln)){
      if(curCount > maxCount){
        maxCount = curCount;
        maxFile = curFile;
      }
      curFile = getFileIndexPLN(pln);
      curCount = getCountPLN(pln);
    }
    else{
      curCount += getCountPLN(pln);
    }

    pln = getNextPLN(pln);
  }

  if(curCount > maxCount){
    maxCount = curCount;
    maxFile = curFile;
  }

  *appearances = maxCount;
  return maxFile;
}

int getMincountFilePL(postingList *pl, int *appearances){
  int curFile = -1;
  int minFile = -1;
  int curCount = 0;
  int minCount = 0;

  plNode *pln = pl->start;
  while(pln != NULL){
    if(curFile == -1){
      curFile = getFileIndexPLN(pln);
      curCount = getCountPLN(pln);
    }
    else if(curFile != getFileIndexPLN(pln)){
      if(minCount == 0 || curCount < minCount){
        minCount = curCount;
        minFile = curFile;
      }
      curFile = getFileIndexPLN(pln);
      curCount = getCountPLN(pln);
    }
    else{
      curCount += getCountPLN(pln);
    }

    pln = getNextPLN(pln);
  }

  if(curCount < minCount){
    minCount = curCount;
    minFile = curFile;
  }

  *appearances = minCount;
  return minFile;
}
