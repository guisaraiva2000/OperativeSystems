#include "sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

pthread_rwlock_t *rwlockVet;

void initializeLockVets(){
    rwlockVet = (pthread_rwlock_t*) malloc(10 * sizeof(pthread_rwlock_t));
    if (!rwlockVet){
        fprintf(stderr, "Error... Could not allocate mutexVet.");
        exit(EXIT_FAILURE);
    }
    for(int i = 0; i < 10; i++)
        if(pthread_rwlock_init(&rwlockVet[i], NULL) != 0) {
            perror("Error... Could not initialize rwlockVet.");
            exit(EXIT_FAILURE);
        }
}

void lockError(int n){
    /* 0 for lock; 1 for unlock; 2 for destroy */
    if (n == 0) {
        perror("Error... Could not lock\n");
        exit(EXIT_FAILURE);
    }
    else if (n == 1)  {
        perror("Error... Could not unlock\n");
        exit(EXIT_FAILURE);
    }
    else if (n == 2)  {
        perror("Error... Could not destroy\n");
        exit(EXIT_FAILURE);
    }
}

void lock(char state[], int n){
    /* n represents rwlock number
     * state represents the state of the rwlock ("read" / "write") */

    /* same as above but consider the rwlock state */
    if(strcmp(state, "read") == 0){
        if (pthread_rwlock_rdlock( &rwlockVet[n] ) != 0)
            lockError(0);
    } else if(strcmp(state, "write") == 0){
        if (pthread_rwlock_wrlock( &rwlockVet[n] ) != 0)
            lockError(0);
    } else{
        fprintf(stderr, "Error... Wrong lock state.\n");
        exit(EXIT_FAILURE);
    }
}

void unlock(int n){
    /* n represents rwlock number */

    /* do the respective unlocks and check errors */
    if (n > 9) {
        fprintf(stderr, "Error... Wrong unlock number(0-9).\n");
        exit(EXIT_FAILURE);
    }
    if (pthread_rwlock_unlock( &rwlockVet[n] ) != 0)
        lockError(1);

}

void lockDestroy(){
    if(pthread_rwlock_destroy(rwlockVet) != 0)
        lockError(2);
}

