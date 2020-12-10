#ifndef SO1_SEM_H
#define SO1_SEM_H

#include <semaphore.h>

typedef sem_t Semaphore;

Semaphore *semaphore_init(int value);
void semaphore_wait(Semaphore *sem);
void semaphore_post(Semaphore *sem);
void semaphore_destroy(Semaphore *sem);
void semaphore_free(Semaphore *sem);

#endif //SO1_SEM_H
