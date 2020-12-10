#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include "fs.h"
#include "sync.h"
#include "sem.h"

#define MAX_COMMANDS 10
#define MAX_INPUT_SIZE 100

char *globalInputFile = NULL;
char *globalOutputFile = NULL;
int numberThreads = 0;
int numberCommands = 0;
int headQueue = 0;
int numberBuckets = 0;
tecnicofs* fs;
char inputCommands[MAX_COMMANDS][MAX_INPUT_SIZE];

Semaphore* semCons;
Semaphore* semProd;


static void displayUsage (const char* appName){
    printf("Usage: %s input_filepath output_filepath threads_number buckets_number\n", appName);
    exit(EXIT_FAILURE);
}

/* function that checks if the number of threads is valid */
void checkNumberThreads(){
    if(!numberThreads){
        fprintf(stderr, "Error... Invalid thread number.\n");
        exit(EXIT_FAILURE);
    }
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

static void parseArgs (long argc, char* const argv[]){
    if (argc != 5) {
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }

    globalInputFile = argv[1];    /* opens the input file */
    globalOutputFile = argv[2];  /* opens the output file */
    numberThreads = atoi(argv[3]);
    numberBuckets = atoi(argv[4]);
    checkNumberThreads();

    if(!numberBuckets || numberBuckets < 1){
        fprintf(stderr, "Invalid number of buckets\n");
        displayUsage(argv[0]);
    }
}

void insertCommand(char* data) {
    strcpy(inputCommands[numberCommands], data);
    numberCommands = (numberCommands + 1) % MAX_COMMANDS;
}

char* removeCommand() {
    char *buf = inputCommands[headQueue];
    headQueue = (headQueue + 1) % MAX_COMMANDS; // variavel consumidora
    return buf;
}

void errorParse(int lineNumber){
    fprintf(stderr, "Error: %d command invalid\n", lineNumber);
    exit(EXIT_FAILURE);
}

void * processInput(){
    FILE* inputFile;
    inputFile = fopen(globalInputFile, "r");
    if(!inputFile){
        fprintf(stderr, "Error: Could not read %s\n", globalInputFile);
        exit(EXIT_FAILURE);
    }

    char line[MAX_INPUT_SIZE];
    int lineNumber = 0;

    while (fgets(line, sizeof(line)/sizeof(char), inputFile)) {
        char token;
        char name[MAX_INPUT_SIZE];
        lineNumber++;
        int numTokens = sscanf(line, "%c %s", &token, name);

        /* perform minimal validation */
        if (numTokens < 1) {
            continue;
        }
        switch (token) {
            case 'c':
            case 'l':
            case 'd':
            case 'r':
            case 'e':
                semaphore_wait(semProd);
                lock("write", 2);
                if(numTokens != 2)
                    errorParse(lineNumber);
                insertCommand(line);
                unlock(2);
                semaphore_post(semCons);
                break;
            case '#':
                break;
            default: { /* error */
                errorParse(lineNumber);
            }
        }
    }
    fclose(inputFile);

    semaphore_wait(semProd);
    lock("write", 2);
    insertCommand("e exit");
    unlock(2);
    semaphore_post(semCons);
    //pthread_exit(NULL);
    return NULL;
}

FILE * openOutputFile() {
    FILE *fp;
    fp = fopen(globalOutputFile, "w");
    if (fp == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }
    return fp;
}

void * applyCommands() {
    while (1) {
        semaphore_wait(semCons);
        lock("write", 1);

        const char *command = removeCommand();          /* locked */
        if (command == NULL) {
            unlock(1);
            semaphore_post(semProd);
            continue;
        }
        char token;
        char name[MAX_INPUT_SIZE];
        char newName[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %s", &token, name, newName);

        if (numTokens != 2 && numTokens != 3) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int searchResult;
        int renameResult;
        int iNumber;
        switch (token) {
            case 'c':
                iNumber = obtainNewInumber(fs);    /* locked */
                unlock(1);
                semaphore_post(semProd);
                create(fs, name, iNumber);         /* locked */
                break;
            case 'l':
                unlock(1);
                semaphore_post(semProd);
                searchResult = lookup(fs, name);   /* locked */
                if (!searchResult)
                    fprintf(stderr, "%s not found\n", name);
                else
                    fprintf(stderr, "%s found with inumber %d\n", name, searchResult);
                break;
            case 'd':
                unlock(1);
                semaphore_post(semProd);
                delete(fs, name);     /* locked */
                break;
            case 'r':
                unlock(1);
                semaphore_post(semProd);
                renameResult = changeName(fs, name, newName);
                if(renameResult == 1)
                    fprintf(stderr, "%s not found\n", name);
                else if(renameResult == 2)
                    fprintf(stderr, "%s already exists\n", newName);
                break;
            case 'e':
                unlock(1);
                insertCommand("e exit");
                semaphore_post(semCons);
                //pthread_exit(NULL);
                return NULL;
            default: { /* error */
                unlock(1);
                semaphore_post(semProd);
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

/* function that creates the thread pool */
void setThreadPool(){
    int i;
    struct timeval start, end;
    pthread_t tid[numberThreads+1];

    gettimeofday(&start, NULL);   /* start clock */

    if(pthread_create(&tid[0], NULL, processInput, NULL) != 0) {
        perror("Error... Could not create task."); /* error */
        exit(EXIT_FAILURE);
    }

    for (i = 1; i < numberThreads + 1 ; i++)
        if(pthread_create(&tid[i], NULL, applyCommands, NULL) != 0) {
            perror("Error... Could not create task."); /* error */
            exit(EXIT_FAILURE);
        }

    for (i = 0; i < numberThreads + 1; i++)
        if(pthread_join(tid[i], NULL) != 0)
            perror("Error... A deadlock was detected."); /* error */

    gettimeofday(&end, NULL);     /* close clock */
    double seconds = (double) (end.tv_usec - start.tv_usec) / 1000000 + (double) (end.tv_sec - start.tv_sec);

    printf("TecnicoFS completed in %0.4f seconds.\n", seconds);
}


int main(int argc, char* argv[]) {
    parseArgs(argc, argv);
    FILE * outputFile = openOutputFile();
    semCons = semaphore_init(0);
    semProd = semaphore_init(MAX_COMMANDS);

    fs = new_tecnicofs(numberBuckets);
    setThreadPool();
    print_tecnicofs_tree(outputFile, fs);

    fflush(outputFile);
    fclose(outputFile);

    lockDestroy();
    lockTreeDestroy(fs);
    semaphore_free(semCons);
    semaphore_free(semProd);
    free_tecnicofs(fs);

    exit(EXIT_SUCCESS);
}
