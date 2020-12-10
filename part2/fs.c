#include "fs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

int obtainNewInumber(tecnicofs* fs) {
    int newInumber = ++(fs->nextINumber);
    return newInumber;
}

tecnicofs* new_tecnicofs(int numberBuckets){
    tecnicofs*fs = malloc(sizeof(tecnicofs));
    if (!fs) {
        perror("failed to allocate tecnicofs");
        exit(EXIT_FAILURE);
    }
    fs->numberBuckets = numberBuckets;
    fs->hashtable = initializeHashTable(fs->numberBuckets);
    initializeLockVets(fs);
    fs->nextINumber = 0;
    return fs;
}

void free_tecnicofs(tecnicofs* fs){
    freeHash(fs->hashtable, fs->numberBuckets);
    freeLockVets(fs);
    free(fs);
}

void create(tecnicofs* fs, char *name, int inumber){
    int key = hash(name, fs->numberBuckets);
    lockTree("write", key, fs);
    fs->hashtable[key] = insert(fs->hashtable[key], name, inumber);
    unlockTree(key, fs);
}

void delete(tecnicofs* fs, char *name){
    int key = hash(name, fs->numberBuckets);
    lockTree("write", key, fs);
    fs->hashtable[key] = remove_item(fs->hashtable[key], name);
    unlockTree(key, fs);
}

int lookup(tecnicofs* fs, char *name){
    int key = hash(name, fs->numberBuckets);
    lockTree("read", key, fs);
    node* searchNode = search(fs->hashtable[key], name);
    unlockTree(key, fs);
    if ( searchNode ) return searchNode->inumber;
    return 0;
}

/* see if the old file exists */
int checkOld(tecnicofs* fs, char *oldName, int key1){
    node* searchOldNode = search(fs->hashtable[key1], oldName);
    if (!searchOldNode){
        return 1;
    }
    return searchOldNode->inumber;
}

/* see if the new file does't exist */
int checkNew(tecnicofs* fs, char *newName, int key2){
    node* searchNewNode = search(fs->hashtable[key2], newName);
    if (!searchNewNode) {
        return 1;
    }
    return 0;
}

/* function that renames a file */
int changeName(tecnicofs* fs, char *oldName, char *newName){
    int key1 = hash(oldName, fs->numberBuckets);
    int key2 = hash(newName, fs->numberBuckets);

    if (key1 == key2){ /* if the keys are equal just need one lock */
        lockTree("read", key1, fs);

        int old = checkOld(fs,oldName, key1);
        int new = checkNew(fs,newName, key1);
        if(old == 1){     /* checks the old file */
            unlockTree(key1, fs);
            return 1;
        }
        if(new == 2){    /* checks the new file */
            unlockTree(key1, fs);
            return 2;
        }
        fs->hashtable[key1] = remove_item(fs->hashtable[key1], oldName);
        fs->hashtable[key2] = insert(fs->hashtable[key2], newName, old);

        unlockTree(key1, fs);
        return 0;
    }

    /* checks the old file */
    lockTree("read", key1, fs);
    node* searchOldNode = search(fs->hashtable[key1], oldName);
    int old = checkOld(fs,oldName, key1);
    if(old == 1){
        unlockTree(key1, fs);
        return 1;
    }
    unlockTree(key1, fs);

    /* checks the new file */
    lockTree("read", key2, fs);
    int new = checkNew(fs,newName, key2);
    if(new == 2){
        unlockTree(key2, fs);
        return 2;
    }
    unlockTree(key2, fs);

    /* avoiding interlocking */
    while (1) {
        lockTree("write", key1, fs);
        if (trylockTree("write", key2, fs) != 0) {
            break;
        } else {
            unlockTree(key1, fs);
        }
    }
    fs->hashtable[key2] = insert(fs->hashtable[key2], newName, searchOldNode->inumber);
    fs->hashtable[key1] = remove_item(fs->hashtable[key1], oldName);

    unlockTree(key2, fs);
    unlockTree(key1, fs);
    return 0;
}

void print_tecnicofs_tree(FILE * fp, tecnicofs *fs){
    print_tree(fp, *fs->hashtable);
}

/* initialize mutex or rwlock array */
void initializeLockVets(tecnicofs *fs){
    #ifdef MUTEX
        fs->mutexVet = (pthread_mutex_t*) malloc(fs->numberBuckets * sizeof(pthread_mutex_t));
        if (!fs->mutexVet){
            fprintf(stderr, "Error... Could not allocate mutexVet.");
            exit(EXIT_FAILURE);
        }
        for(int i = 0; i< fs->numberBuckets;i++)
            if(pthread_mutex_init(&fs->mutexVet[i], NULL) != 0)
                perror("Error... Could not initialize mutexVet.");

   #elif RWLOCK
        fs->rwlockVet = (pthread_rwlock_t*) malloc(fs->numberBuckets * sizeof(pthread_rwlock_t));
        if (!fs->rwlockVet){
            fprintf(stderr, "Error... Could not allocate rwlockVet.");
            exit(EXIT_FAILURE);
        }
        for(int i = 0; i < fs->numberBuckets;i++)
            if(pthread_rwlock_init(&fs->rwlockVet[i], NULL) != 0)
                perror("Error... Could not initialize rwlockVet.");
    #endif
}

void freeLockVets(tecnicofs *fs){
    #ifdef MUTEX
        free(fs->mutexVet);
    #elif RWLOCK
        free(fs->rwlockVet);
    #endif
}

/* selects the lock in the vector by a giving key */
void lockTree(char state[], int key, tecnicofs* fs){
    /* state represents the state of the rwlock ("read" / "write") */

    #ifdef MUTEX  /* do the respective locks and check errors*/
        if (pthread_mutex_lock( &fs->mutexVet[key] ) != 0)
                lockError(0);
    #elif RWLOCK /* same as above but consider the rwlock state */
        if(strcmp(state, "read") == 0){
            if (pthread_rwlock_rdlock( &fs->rwlockVet[key] ) != 0)
                lockError(0);
        } else if(strcmp(state, "write") == 0){
            if (pthread_rwlock_wrlock( &fs->rwlockVet[key] ) != 0)
                lockError(0);
        } else{
            fprintf(stderr, "Error... Wrong lock state.\n");
            exit(EXIT_FAILURE);
        }
    #endif
}

void lockTreeDestroy(tecnicofs* fs){
    #ifdef MUTEX
        for(int i = 0; i < fs->numberBuckets; i++)
            if (pthread_mutex_destroy( &fs->mutexVet[i] ) != 0)
                lockError(2);
    #elif RWLOCK
        for(int i = 0; i < fs->numberBuckets; i++)
            if (pthread_rwlock_destroy( &fs->rwlockVet[i] ) != 0)
                lockError(2);
    #endif
}

/* same as lockTree but with trylocks */
int trylockTree(char state[], int key, tecnicofs* fs){
    /* state represents the state of the rwlock ("read" / "write") */

    #ifdef MUTEX  /* do the respective locks and check errors*/
        if (pthread_mutex_trylock( &fs->mutexVet[key] ) != 0)
                return 0;
    #elif RWLOCK /* same as above but consider the rwlock state */
        if(strcmp(state, "read") == 0){
            if (pthread_rwlock_tryrdlock( &fs->rwlockVet[key] ) != 0)
                return 0;
        } else if(strcmp(state, "write") == 0){
            if (pthread_rwlock_trywrlock( &fs->rwlockVet[key] ) != 0)
                return 0;
        } else {
            fprintf(stderr, "Error... Wrong lock state.\n.");
            exit(EXIT_FAILURE);
        }
    #endif
    return 1;
}

/* unlocks the lock in the vector by a giving key */
void unlockTree(int key, tecnicofs* fs){
    #ifdef MUTEX  /* do the respective unlocks and check errors*/
        if (pthread_mutex_unlock( &fs->mutexVet[key] ) != 0)
            lockError(1);
    #elif RWLOCK
        if (pthread_rwlock_unlock( &fs->rwlockVet[key] ) != 0)
            lockError(1);
    #endif
}