#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/un.h>
#include <signal.h>
#include "fs.h"
#include "unix.h"

#define MAXCLIENTS 5
#define MAXLINE 1000

char *globalOutputFile = NULL;
char *socketName = NULL;
int numberBuckets = 0;
int signalFLAG = 0;
tecnicofs* fs;
struct timeval start, end;
struct ucred ucred;
int sockfd, servlen;;
struct sockaddr_un cli_addr, serv_addr;
pthread_t tid[MAXCLIENTS];
int i = 0;
int activeClients = 0;


static void displayUsage (const char* appName){
    printf("Usage: %s socket_name output_filepath buckets_number\n", appName);
    exit(EXIT_FAILURE);
}

/* function that checks if the number of threads is valid */
static void parseArgs (long argc, char* const argv[]){
    if (argc != 4) {
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }
    socketName = argv[1];    /* SOCKET NAME  */
    globalOutputFile = argv[2];  /* opens the output file */
    numberBuckets = atoi(argv[3]);

    if(!socketName){
        fprintf(stderr, "Invalid socket name\n");
        displayUsage(argv[0]);
    }
    if(!globalOutputFile){
        fprintf(stderr, "Invalid output file\n");
        displayUsage(argv[0]);
    }
    if(!numberBuckets || numberBuckets != 1){
        fprintf(stderr, "Invalid number of buckets\n");
        displayUsage(argv[0]);
    }
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

void shutDown(){
    for(int k = 0; k < activeClients; k++){
        if(pthread_join(tid[k], NULL) != 0){
            perror("Error... Could not join task."); /* error */
            exit(EXIT_FAILURE);
        }
    }

    FILE *outputFile = openOutputFile();

    print_tecnicofs_tree(outputFile, fs);

    gettimeofday(&end, NULL);     /* close clock */
    double seconds = (double) (end.tv_usec - start.tv_usec) / 1000000 + (double) (end.tv_sec - start.tv_sec);

    printf("TecnicoFS completed in %0.4f seconds.\n", seconds);

    fflush(outputFile);
    fclose(outputFile);

    unlink(socketName);
    lockDestroy();
    inode_table_destroy();
    free_tecnicofs(fs);
    close(sockfd);
    exit(EXIT_SUCCESS);
}

void parser(int error, int newsockfd){
    switch(error){
        case 0:
            if (dprintf(newsockfd, "0") < 0) {
                perror("Could not write on socket");
                pthread_exit(NULL);
            }
            break;
        case TECNICOFS_ERROR_FILE_ALREADY_EXISTS:
            if (dprintf(newsockfd, "%d", TECNICOFS_ERROR_FILE_ALREADY_EXISTS) < 0) {
                perror("Could not write on socket");
                pthread_exit(NULL);
            }
            break;
        case TECNICOFS_ERROR_FILE_NOT_FOUND:
            if (dprintf(newsockfd, "%d", TECNICOFS_ERROR_FILE_NOT_FOUND) < 0) {
                perror("Could not write on socket");
                pthread_exit(NULL);
            }
            break;
        case TECNICOFS_ERROR_PERMISSION_DENIED:
            if (dprintf(newsockfd, "%d", TECNICOFS_ERROR_PERMISSION_DENIED) < 0) {
                perror("Could not write on socket");
                pthread_exit(NULL);
            }
            break;
        case TECNICOFS_ERROR_MAXED_OPEN_FILES:
            if (dprintf(newsockfd, "%d", TECNICOFS_ERROR_MAXED_OPEN_FILES) < 0) {
                perror("Could not write on socket");
                pthread_exit(NULL);
            }
            break;
        case TECNICOFS_ERROR_FILE_NOT_OPEN:
            if (dprintf(newsockfd, "%d", TECNICOFS_ERROR_FILE_NOT_OPEN) < 0) {
                perror("Could not write on socket");
                pthread_exit(NULL);
            }
            break;
        case TECNICOFS_ERROR_FILE_IS_OPEN:
            if (dprintf(newsockfd, "%d", TECNICOFS_ERROR_FILE_IS_OPEN) < 0) {
                perror("Could not write on socket");
                pthread_exit(NULL);
            }
            break;
        case TECNICOFS_ERROR_INVALID_MODE:
            if (dprintf(newsockfd, "%d", TECNICOFS_ERROR_INVALID_MODE) < 0) {
                perror("Could not write on socket");
                pthread_exit(NULL);
            }
            break;
        case TECNICOFS_ERROR_OTHER:
            if (dprintf(newsockfd, "%d", TECNICOFS_ERROR_OTHER) < 0) {
                perror("Could not write on socket");
                pthread_exit(NULL);
            }
            break;
        default:
            fprintf(stderr, "Error: command to apply\n");
            pthread_exit(NULL);
    }
}

int applyCommands(client *cl, int newsockfd) {
    while (1) {
        char line[MAXLINE] = {0};
        lock("read", 2);
        /* PASSAR CONTEUDO DO SOCKET PARA UM BUFFER */
        int n = read(newsockfd, line, MAXLINE);
        if(n == 0) return -1;
        if (n < 0) {
            perror("Error: Could not read the socket");
            pthread_exit(NULL);
        }
        char token;
        char *name;
        char filename[MAXCONTENT] = {0};
        char filenameNew[MAXCONTENT] = {0};
        char *buffer;
        char *buffer1;
        permission permissions = NONE, mode = NONE;
        int fd = 0, fd1 = 0, renameResult = 0;
        int len = 0;

        int numTokens = sscanf(line, "%c", &token);
        if (numTokens != 1) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            pthread_exit(NULL);
        }

        switch (token) {
            case 'c':
                numTokens = sscanf(line, "%c %s %u", &token, filename, &permissions);
                if (numTokens != 3) {
                    fprintf(stderr, "Error: invalid command in Queue\n");
                    unlock(2);
                    pthread_exit(NULL);
                }
                permission otherPermission = permissions % 10;
                permission ownerPermission = (permissions - otherPermission) / 10;
                int err = create(fs, filename, ownerPermission, otherPermission, ucred.uid);  /* locked */
                parser(err, newsockfd);
                unlock(2);
                break;
            case 'l':
                buffer1 = malloc(sizeof(char) * (MAXCONTENT + 1));
                if(!buffer1){
                    perror("Error... Could not alloc.");
                    unlock(2);
                    pthread_exit(NULL);
                }
                numTokens = sscanf(line, "%c %d %d", &token, &fd1, &len);
                if (numTokens != 3) {
                    unlock(2);
                    fprintf(stderr, "Error: invalid command in Queue\n");
                    pthread_exit(NULL);
                }
                int buflen = readFile(cl, fd1, buffer1, len);
                if (buflen >= 0) {
                    if (dprintf(newsockfd, "%d %s", buflen, buffer1) < 0) {
                        perror("Could not write on socket");
                        unlock(2);
                        pthread_exit(NULL);
                    }
                } else{
                    parser(buflen, newsockfd);
                }
                free(buffer1);
                unlock(2);
                break;
            case 'd':
                numTokens = sscanf(line, "%c %s", &token, filename);
                if (numTokens != 2) {
                    unlock(2);
                    fprintf(stderr, "Error: invalid command in Queue\n");
                    unlock(2);
                    pthread_exit(NULL);
                }
                int err1 = delete(cl, fs, filename);     /* locked */
                parser(err1, newsockfd);
                unlock(2);
                break;
            case 'r':
                numTokens = sscanf(line, "%c %s %s", &token, filename, filenameNew);
                if (numTokens != 3) {
                    unlock(2);
                    fprintf(stderr, "Error: invalid command in Queue\n");
                    pthread_exit(NULL);
                }
                renameResult = changeName(cl, fs, filename, filenameNew);
                parser(renameResult, newsockfd);
                unlock(2);
                break;
            case 'o':
                name = malloc(sizeof(char) * (MAXCONTENT + 1));
                if(!name){
                    perror("Error... Could not alloc.");
                    unlock(2);
                    pthread_exit(NULL);
                }
                numTokens = sscanf(line, "%c %s %u", &token, name, &mode);
                if (numTokens != 3) {
                    unlock(2);
                    fprintf(stderr, "Error: invalid command in Queue\n");
                    pthread_exit(NULL);
                }
                int descriptor = openFile(cl, fs, name, mode);
                if (descriptor >= 0 && descriptor < 5) {
                    if (dprintf(newsockfd, "%d", descriptor) < 0) {
                        perror("Could not write on socket");
                        unlock(2);
                        pthread_exit(NULL);
                    }
                } else{
                    parser(descriptor, newsockfd);
                }
                free(name);
                unlock(2);
                break;
            case 'x':
                numTokens = sscanf(line, "%c %d", &token, &fd);
                if (numTokens != 2) {
                    fprintf(stderr, "Error: invalid command in Queue\n");
                    unlock(2);
                    pthread_exit(NULL);
                }
                int err3 = closeFile(cl, fd);
                parser(err3, newsockfd);
                unlock(2);
                break;
            case 'w':
                buffer = malloc(sizeof(char) * (MAXCONTENT + 1));
                if(!buffer){
                    perror("Error... Could not alloc.");
                    unlock(2);
                    pthread_exit(NULL);
                }
                numTokens = sscanf(line, "%c %d %s", &token, &fd, buffer);
                if (numTokens != 3) {
                    fprintf(stderr, "Error: invalid command in Queue\n");
                    unlock(2);
                    pthread_exit(NULL);
                }
                int err4 = writeFile(cl, fd, buffer, strlen(buffer));
                parser(err4, newsockfd);
                free(buffer);
                unlock(2);
                break;
            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                unlock(2);
                pthread_exit(NULL);
            }
        }
    }
}

void handleSignal(int sig) {
    printf("Caught signal %d\n", sig);
    signalFLAG++;
}

void * processClient(void * newsockfdd){
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    int s = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (s != 0){
        perror("Error... Could not set sigmask.");
        pthread_exit(NULL);
    }

    lock("write", 1);
    int *newsockfd = newsockfdd;
    activeClients++;
    client *cl = newClient(ucred.uid);
    unlock(1);
    applyCommands(cl, *newsockfd);
    freeClient(cl);
    activeClients--;
    return NULL;
}

void *mountSV(){
    /* Cria socket stream */
    if ((sockfd = socket(AF_UNIX,SOCK_STREAM,0) ) < 0){
        perror("server: can't open stream socket");
        exit(EXIT_FAILURE);
    }
    /* Elimina o nome, para o caso de já existir.*/
    unlink(UNIXSTR_PATH);
    /* O nome serve para que os clientes possam identificar o servidor */
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, socketName, sizeof(serv_addr.sun_path) - 1);
    if (bind(sockfd, (const struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        perror("server, can't bind local address");
        exit(EXIT_FAILURE);
    }
    listen(sockfd, 50);
    gettimeofday(&start, NULL);   /* start clock */
    while(signalFLAG == 0) {
        int clilen = sizeof(cli_addr);
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,(unsigned int *restrict) &clilen);
        if (newsockfd < 0){
            if (signalFLAG != 0){
                while (activeClients != 0); /* espera que todos acabem */
                return NULL;
            }
            perror("server: accept error");
            exit(EXIT_FAILURE);
        }
        socklen_t len = sizeof(struct ucred);
        int uid = getsockopt(newsockfd, SOL_SOCKET, SO_PEERCRED, &ucred, &len);
        if (uid < 0){
            perror("Error... Could not get client uid.");
            exit(EXIT_FAILURE);
        }
        if (activeClients == MAXCLIENTS)
            shutDown();

        if(pthread_create(&tid[i++], NULL, processClient, (void*) &newsockfd) != 0){
            perror("Error... Could not create task."); /* error */
            exit(EXIT_FAILURE);
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    struct sigaction saStruct;

    sigemptyset(&saStruct.sa_mask);
    saStruct.sa_flags = 0;
    saStruct.sa_handler = handleSignal;
    int a = sigaction(SIGINT, &saStruct, NULL);
    if (a != 0){
        perror("Error... Could not set sigaction");
        exit(EXIT_FAILURE);
    }
    parseArgs(argc, argv);
    fs = new_tecnicofs();
    inode_table_init();
    initializeLockVets();
    mountSV();
    shutDown();
    exit(EXIT_SUCCESS);
}
