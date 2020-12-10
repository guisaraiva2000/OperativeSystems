#include "sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER; /*----------------------*/
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER; /* mutex initialization */

pthread_rwlock_t rwlock1 = PTHREAD_RWLOCK_INITIALIZER; /*-----------------------*/
pthread_rwlock_t rwlock2 = PTHREAD_RWLOCK_INITIALIZER; /* rwlock initialization */


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
    /* n represents mutex/rwlock number
     * state represents the state of the rwlock ("read" / "write") */

    #ifdef MUTEX  /* do the respective locks and check errors*/
        if (n == 1){
            if (pthread_mutex_lock( &mutex1 ) != 0)
                lockError(0);
        }
        else if (n == 2){
            if (pthread_mutex_lock( &mutex2 ) != 0)
                lockError(0);
        }
    #elif RWLOCK /* same as above but consider the rwlock state */
        if (n == 1){
            if(strcmp(state, "read") == 0){
                if (pthread_rwlock_rdlock( &rwlock1) != 0)
                    lockError(0);
            } else if(strcmp(state, "write") == 0){
                if (pthread_rwlock_wrlock( &rwlock1 ) != 0)
                    lockError(0);
            } else{
                fprintf(stderr, "Error... Wrong lock state.\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (n == 2){
            if(strcmp(state, "read") == 0){
                if (pthread_rwlock_rdlock( &rwlock2) != 0)
                    lockError(0);
            } else if(strcmp(state, "write") == 0){
                if (pthread_rwlock_wrlock( &rwlock2 ) != 0)
                    lockError(0);
            } else{
                fprintf(stderr, "Error... Wrong lock state.\n");
                exit(EXIT_FAILURE);
            }
        }
    #endif
}

void unlock(int n){
    /* n represents mutex/rwlock number */

    /* do the respective unlocks and check erros */
    #ifdef MUTEX
        if (n == 1){
            if (pthread_mutex_unlock( &mutex1 ) != 0)
                lockError(1);
        }
        else if (n == 2){
            if (pthread_mutex_unlock( &mutex2 ) != 0)
                lockError(1);
        }
    #elif RWLOCK
        if (n == 1){
            if (pthread_rwlock_unlock( &rwlock1) != 0)
                lockError(1);
        }
        else if (n == 2){
            if (pthread_rwlock_unlock( &rwlock2) != 0)
                lockError(1);
        }
    #endif
}

void lockDestroy(){
    #ifdef MUTEX
        if(pthread_mutex_destroy(&mutex1) != 0)
            lockError(2);
        if(pthread_mutex_destroy(&mutex2) != 0)
            lockError(2);
    #elif RWLOCK
        if(pthread_rwlock_destroy(&rwlock1) != 0)
            lockError(2);
        if(pthread_rwlock_destroy(&rwlock2) != 0)
            lockError(2);
    #endif
}

