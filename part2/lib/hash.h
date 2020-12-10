#ifndef HASH_H
#define HASH_H 1
#include "bst.h"


int hash(char* name, int n);
node **initializeHashTable(int numberBuckets);
void freeHash(node **hashtable, int numberBuckets);

#endif

