#ifndef FS_H
#define FS_H

#include <sys/types.h>
#include "lib/bst.h"
#include "lib/hash.h"
#include "sync.h"

typedef struct tecnicofs {
    node** hashtable;
    pthread_mutex_t* mutexVet;
    pthread_rwlock_t* rwlockVet;
    int numberBuckets;
    int nextINumber;
} tecnicofs;

int obtainNewInumber(tecnicofs* fs);
tecnicofs* new_tecnicofs(int numberBuckets);
void free_tecnicofs(tecnicofs* fs);
void create(tecnicofs* fs, char *name, int inumber);
void delete(tecnicofs* fs, char *name);
int lookup(tecnicofs* fs, char *name);
int checkOld(tecnicofs* fs, char *oldName, int key1);
int checkNew(tecnicofs* fs, char *newName, int key2);
int changeName(tecnicofs* fs, char *oldName, char *newName);
void print_tecnicofs_tree(FILE * fp, tecnicofs *fs);
void initializeLockVets(tecnicofs *fs);
void freeLockVets(tecnicofs* fs);
int trylockTree(char state[], int key, tecnicofs* fs);
void lockTree(char state[], int key, tecnicofs* fs);
void lockTreeDestroy(tecnicofs* fs);
void unlockTree(int key, tecnicofs* fs);

#endif /* FS_H */