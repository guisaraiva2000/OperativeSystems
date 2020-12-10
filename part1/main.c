#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <pthread.h>
#include "fs.h"

#define MAX_COMMANDS 150000
#define MAX_INPUT_SIZE 100


pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER; /*----------------------*/
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER; /* mutex initialization */
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER; /*----------------------*/

pthread_rwlock_t rwlock1 = PTHREAD_RWLOCK_INITIALIZER; /*-----------------------*/
pthread_rwlock_t rwlock2 = PTHREAD_RWLOCK_INITIALIZER; /* rwlock initialization */
pthread_rwlock_t rwlock3 = PTHREAD_RWLOCK_INITIALIZER; /*-----------------------*/

int numberThreads = 0;
int numberCommands = 0;
int headQueue = 0;
tecnicofs* fs;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];


static void displayUsage (const char* appName){
    printf("Usage: %s\n", appName);
    exit(EXIT_FAILURE);
}

static void parseArgs (long argc, char* const argv[]){
    if (argc != 4) {
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }
}

int insertCommand(char* data) {
    if(numberCommands != MAX_COMMANDS) {
        strcpy(inputCommands[numberCommands++], data);
        return 1;
    }
    return 0;
}

char* removeCommand() {
    if(numberCommands > 0){
        numberCommands--;
        return inputCommands[headQueue++];
    }
    return NULL;
}

void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    //exit(EXIT_FAILURE);
}

void processInput(FILE* f){
    char line[MAX_INPUT_SIZE];

    while (fgets(line, sizeof(line)/sizeof(char), f)) {
        char token;
        char name[MAX_INPUT_SIZE];

        int numTokens = sscanf(line, "%c %s", &token, name);

        /* perform minimal validation */
        if (numTokens < 1) {
            continue;
        }
        switch (token) {
            case 'c':
            case 'l':
            case 'd':
                if(numTokens != 2)
                    errorParse();
                if(insertCommand(line))
                    break;
                return;
            case '#':
                break;
            default: { /* error */
                errorParse();
            }
        }
    }
}

void lockError(int n){
    /* 0 for lock; 1 for unlock */
    if (n == 0) {
        fprintf(stderr, "Error... Could not lock\n");
        exit(EXIT_FAILURE);
    }

    else if (n == 1)  {
        fprintf(stderr, "Error... Could not unlock\n");
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
        else if (n == 3){
            if (pthread_mutex_lock( &mutex3 ) != 0)
                lockError(0);
        }
    #elif RWLOCK /* same as above but consider the rwlock state */
        if (n == 1){
            if(strcmp(state, "read") == 0){
                if (pthread_rwlock_rdlock( &rwlock1) != 0)
                    lockError(0);
            } else{
                if (pthread_rwlock_wrlock( &rwlock1) != 0)
                    lockError(0);
            }
        }
        else if (n == 2){
            if(strcmp(state, "read") == 0){
                if (pthread_rwlock_rdlock( &rwlock2) != 0)
                    lockError(0);
            } else{
                if (pthread_rwlock_wrlock( &rwlock2) != 0)
                    lockError(0);
            }
        }
        else if (n == 3){
            if(strcmp(state, "read") == 0){
                if (pthread_rwlock_rdlock( &rwlock3) != 0)
                    lockError(0);
            } else{
                if (pthread_rwlock_wrlock( &rwlock3) != 0)
                    lockError(0);
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
        else if (n == 3){
            if (pthread_mutex_unlock( &mutex3 ) != 0)
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
        else if (n == 3){
            if (pthread_rwlock_unlock( &rwlock3) != 0)
                lockError(1);
        }
    #endif
}

void * applyCommands() {
    while (numberCommands > 0) {
        const char *command = removeCommand();
        if (command == NULL) {
            continue;
        }

        char token;
        char name[MAX_INPUT_SIZE];

        lock("read", 1);
        int numTokens = sscanf(command, "%c %s", &token, name);   /* locked */
        unlock(1);

        if (numTokens != 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int searchResult;
        int iNumber;

        switch (token) {
            case 'c':
                lock("read", 2);
                iNumber = obtainNewInumber(fs);    /* locked */
                unlock(2);

                lock("write", 3);
                create(fs, name, iNumber);         /* locked */
                unlock(3);

                break;
            case 'l':
                lock("read", 3);
                searchResult = lookup(fs, name);   /* locked */
                unlock(3);

                if (!searchResult)
                    printf("%s not found\n", name);
                else
                    printf("%s found with inumber %d\n", name, searchResult);

                break;
            case 'd':
                lock("write", 3);
                delete(fs, name);     /* locked */
                unlock(3);

                break;
            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }

    }
    return NULL;
}

/* Function that creates the thread pool */
void setThreadPool(){
    int i;
    pthread_t tid[numberThreads];

    for (i = 0; i < numberThreads; i++)
        if(pthread_create(&tid[i], NULL, applyCommands, NULL) != 0) {
            fprintf(stderr, "Error... Could not create task."); /* error */
            exit(EXIT_FAILURE);
        }

    for (i = 0; i < numberThreads; i++)
        if(pthread_join(tid[i], NULL) != 0){
            fprintf(stderr, "Error... A deadlock was detected."); /* error */
            exit(EXIT_FAILURE);
        }
}

 /* function that checks if the number of threads
  * in "no-sync" is greater than 1 */
void checkNumberThreads(){
    #ifdef MUTEX
        if (numberThreads < 1) {
            fprintf(stderr, "Error... Invalid thread number.\n");
            exit(EXIT_FAILURE);
        }
    #elif RWLOCK
        if (numberThreads < 1) {
            fprintf(stderr, "Error... Invalid thread number.\n");
            exit(EXIT_FAILURE);
        }
    #else
        if (numberThreads != 1) {
            fprintf(stderr, "Error... Invalid thread number.\n");
            exit(EXIT_FAILURE);
        }
    #endif
}

int main(int argc, char* argv[]) {
    FILE *inputFile, *outputFile;
    struct timeval start, end;

    parseArgs(argc, argv);

    fs = new_tecnicofs();

    inputFile = fopen(argv[1], "r");    /* opens the input file */
    outputFile = fopen(argv[2], "w");   /* opens the output file */
    numberThreads = atoi(argv[3]);

    checkNumberThreads();

    processInput(inputFile);
    gettimeofday(&start, NULL);   /* start clock */
    setThreadPool();
    print_tecnicofs_tree(outputFile, fs);

    fclose(inputFile);
    fclose(outputFile);

    free_tecnicofs(fs);

    gettimeofday(&end, NULL);     /* close clock */

    double seconds = (double) (end.tv_usec - start.tv_usec) / 1000000 + (double) (end.tv_sec - start.tv_sec);

    printf("TecnicoFS completed in %0.4f seconds.\n", seconds);

    exit(EXIT_SUCCESS);
}
