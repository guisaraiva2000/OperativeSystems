#include "hash.h"
#include <stdlib.h>
#include <string.h>

/* Simple hash function for strings.
 * Receives a string and resturns its hash value
 * which is a number between 0 and n-1
 * In case the string is null, returns -1 */
int hash(char* name, int n) {
	if (!name) 
		return -1;
    return (int) name[0] % n;
}

/* initialize  hashtable */
node **initializeHashTable(int numberBuckets) {
    node **hashtable = (node**) malloc(numberBuckets * sizeof(node**));
    if (!hashtable){
        perror("Error... Could not allocate hashtable.");
        exit(EXIT_FAILURE);
    }
    for(int key = 0; key < numberBuckets; key++)
        hashtable[key] = NULL;
    return hashtable;
}

/* frees the hash and its content */
void freeHash(node **hashtable, int numberBuckets) {
    for(int i = 0; i < numberBuckets; i++)
        free_tree(hashtable[i]);
    free(hashtable);
}

