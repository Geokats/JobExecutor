#ifndef POSTING_LIST_H
#define POSTING_LIST_H

typedef struct postingList postingList;
typedef struct plNode plNode;

postingList *createPL();
/*  Creates new posting list and returns a pointer to it.
 *  If memory allocation fails NULL is returned
*/

void deletePL(postingList *pl);
/* Given the first node of the posting list, it deletes all the list's nodes
* freeing all memory allocated for the list
*/

int addAppearancePL(postingList *pl, int fileIndex, int textIndex);
/* Adds one apperance to the index given in an already existing posting list
 * if the posting list doesn't contain the index then a new node is created
 * if memory allocation fails for the new node 0 is returned otherwise 1
*/

int getSizePL(postingList *pl);
/* Returns the number of nodes in the posting list
*/

int getTotalAppearancesPL(postingList *pl);
/* Returns the sum of appearance counts in all the list's nodes
*/

void printPL(postingList *pl);
/* Prints all the indices and appearance counts in the list
*/

int getMaxcountFilePL(postingList *pl, int *appearances);
/* Returns the index of the file with the most appearances
*/

#endif
