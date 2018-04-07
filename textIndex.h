#ifndef TEXT_INDEX_H

typedef struct textIndex textIndex;

textIndex *createTI(const char *fileName);
/* Creates a text index from the given file and return a pointer to it.
 * If any errors occur, NULL is returned.
*/

void deleteTI(textIndex *ti);
/* Deletes the text index and frees all memory used by it
*/

char *getTextTI(textIndex *ti, int index);
/* Returns char* to the document with the given index
*/

int getWordCountTI(textIndex *ti);
/* Returns the number of words in the text index
*/

int getTextCountTI(textIndex *ti);
/* Return the number of documents in the text index
*/

int getCharCountTI(textIndex *ti);
/* Return the number of characters (bytes) in the text index
*/

int textWordCountTI(textIndex *ti, int index);
/* Return the number of words in the document with the given index
*/

#define TEXT_INDEX_H
#endif
