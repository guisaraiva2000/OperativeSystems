#ifndef SYNC_H
#define SYNC_H
#include <stdio.h>

void initializeLockVets();
void lockError(int n);
void lock(char state[], int n);
void lockDestroy();
void unlock(int n);

#endif //SYNC_H
