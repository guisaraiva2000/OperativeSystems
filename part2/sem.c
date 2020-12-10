#include "sem.h"
#include <stdio.h>
#include <stdlib.h>

Semaphore *semaphore_init(int value){
    Semaphore *sem = malloc(sizeof(Semaphore));
    if(!sem){
        perror("Error... Could not allocate semaphore.");
        exit(EXIT_FAILURE);
    }

    int n = sem_init(sem, 0, value);
    if (n != 0){
        perror("Error... sem_init failed");
        exit(EXIT_FAILURE);
    }
    return sem;
}

void semaphore_wait(Semaphore *sem){
    int n = sem_wait(sem);
    if (n != 0){
        perror("Error... sem_wait failed");
        exit(EXIT_FAILURE);
    }
}

void semaphore_post(Semaphore *sem){
    int n = sem_post(sem);
    if (n != 0){
        perror("Error... sem_post failed");
        exit(EXIT_FAILURE);
    }
}

void semaphore_destroy(Semaphore *sem){
    int n = sem_destroy(sem);
    if (n != 0){
        perror("Error... sem_destroy failed");
        exit(EXIT_FAILURE);
    }
}

void semaphore_free(Semaphore *sem){
    semaphore_destroy(sem);
    free(sem);
}

