#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "tecnicofs-api-constants.h"
#include "tecnicofs-client-api.h"


int tfsMount(char * address){
    if(sockfd){
        perror("client: already has active session");
        return TECNICOFS_ERROR_OPEN_SESSION;
    }
    if ((sockfd= socket(AF_UNIX, SOCK_STREAM, 0) ) < 0) {
        perror("client: can't open stream socket");
        return 1;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, address, sizeof(serv_addr.sun_path) - 1);
    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
    if(connect(sockfd, (const struct sockaddr *) &serv_addr, servlen) < 0) {
        perror("client: can't connect to server");
        return TECNICOFS_ERROR_CONNECTION_ERROR;
    }
    return 0;
}

int tfsUnmount(){
    if(!sockfd){
        perror("client: no active session");
        return TECNICOFS_ERROR_NO_OPEN_SESSION;
    }
    close(sockfd);
    return 0;
}

int tfsCreate(char *filename, permission ownerPermissions, permission othersPermissions){
    if(!sockfd){
        perror("client: no active session");
        return TECNICOFS_ERROR_NO_OPEN_SESSION;
    }
    if(dprintf(sockfd, "c %s %u", filename, ownerPermissions*10 + othersPermissions) < 0){
        perror("Error: Could not write on socket.");
        exit(EXIT_FAILURE);
    }
    char line[4] = {0};
    int n = read(sockfd, line, 4);
    if(n < 1){
        perror("Error: Could not read the socket.");
        exit(EXIT_FAILURE);
    }
    int err = atoi(line);
    if (err == TECNICOFS_ERROR_FILE_ALREADY_EXISTS){
        return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
    } else if(err == TECNICOFS_ERROR_OTHER) {
        return TECNICOFS_ERROR_OTHER;
    }
    return 0;
}

int tfsDelete(char *filename){
    if(!sockfd){
        perror("client: no active session");
        return TECNICOFS_ERROR_NO_OPEN_SESSION;
    }
    if(dprintf(sockfd, "d %s", filename) < 0){
        perror("Error: Could not write on socket.");
        exit(EXIT_FAILURE);
    }
    char line[4] = {0};
    int n = read(sockfd, line, 4);
    if(n < 1){
        perror("Error: Could not read the socket.");
        exit(EXIT_FAILURE);
    }
    int err = atoi(line);
    if (err == TECNICOFS_ERROR_FILE_NOT_FOUND){
        return TECNICOFS_ERROR_FILE_NOT_FOUND;
    } else if(err == TECNICOFS_ERROR_FILE_IS_OPEN) {
        return TECNICOFS_ERROR_FILE_IS_OPEN;
    } else if(err == TECNICOFS_ERROR_PERMISSION_DENIED) {
        return TECNICOFS_ERROR_PERMISSION_DENIED;
    } else if(err == TECNICOFS_ERROR_OTHER) {
        return TECNICOFS_ERROR_OTHER;
    }
    return 0;
}

int tfsRename(char *filenameOld, char *filenameNew){
    if(!sockfd){
        perror("client: no active session");
        return TECNICOFS_ERROR_NO_OPEN_SESSION;
    }
    if(dprintf(sockfd, "r %s %s", filenameOld, filenameNew) < 0){
        perror("Error: Could not write on socket.");
        exit(EXIT_FAILURE);
    }
    char line[4] = {0};

    /* PASSAR CONTEUDO DO SOCKET PARA UM BUFFER */
    int n = read(sockfd, line, 4);
    if(n < 1){
        perror("Error: Could not read the socket.");
        exit(EXIT_FAILURE);
    }
    int err = atoi(line);
    if (err == TECNICOFS_ERROR_FILE_ALREADY_EXISTS){
        return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
    } else if(err == TECNICOFS_ERROR_FILE_NOT_FOUND) {
        return TECNICOFS_ERROR_FILE_NOT_FOUND;
    } else if(err == TECNICOFS_ERROR_PERMISSION_DENIED) {
        return TECNICOFS_ERROR_PERMISSION_DENIED;
    } else if(err == TECNICOFS_ERROR_FILE_IS_OPEN) {
        return TECNICOFS_ERROR_FILE_IS_OPEN;
    }else if(err == TECNICOFS_ERROR_OTHER) {
        return TECNICOFS_ERROR_OTHER;
    }
    return 0;
}

int tfsOpen(char *filename, permission mode){
    if(!sockfd){
        perror("client: no active session");
        return TECNICOFS_ERROR_NO_OPEN_SESSION;
    }
    if(dprintf(sockfd, "o %s %u", filename, mode) < 0){
        perror("Error: Could not write on socket.");
        exit(EXIT_FAILURE);
    }
    char line[4] = {0};

    /* PASSAR CONTEUDO DO SOCKET PARA UM BUFFER */
    int n = read(sockfd, line, 4);
    if(n < 1){
        perror("Error: Could not read the socket.");
        exit(EXIT_FAILURE);
    }
    int err = atoi(line);
    if (err == TECNICOFS_ERROR_MAXED_OPEN_FILES){
        return TECNICOFS_ERROR_MAXED_OPEN_FILES;
    } else if(err == TECNICOFS_ERROR_FILE_NOT_FOUND) {
        return TECNICOFS_ERROR_FILE_NOT_FOUND;
    } else if(err == TECNICOFS_ERROR_FILE_IS_OPEN) {
        return TECNICOFS_ERROR_FILE_IS_OPEN;
    } else if(err == TECNICOFS_ERROR_INVALID_MODE) {
        return TECNICOFS_ERROR_INVALID_MODE;
    } else if(err == TECNICOFS_ERROR_OTHER) {
        return TECNICOFS_ERROR_OTHER;
    }
    return err;
}

int tfsClose(int fd){
    if(!sockfd){
        perror("client: no active session");
        return TECNICOFS_ERROR_NO_OPEN_SESSION;
    }
    if(dprintf(sockfd, "x %d", fd) < 0){
        perror("Error: Could not write on socket.");
        exit(EXIT_FAILURE);
    }
    char line[4] = {0};
    int n = read(sockfd, line, 4);
    if(n < 1){
        perror("Error: Could not read the socket.");
        exit(EXIT_FAILURE);
    }
    int err = atoi(line);
    if (err == TECNICOFS_ERROR_FILE_NOT_OPEN) {
        return TECNICOFS_ERROR_FILE_NOT_OPEN;
    }
    return 0;
}

int tfsRead(int fd, char *buffer, int len){
    if(!sockfd){
        perror("client: no active session");
        return TECNICOFS_ERROR_NO_OPEN_SESSION;
    }
    if(dprintf(sockfd, "l %d %d", fd, len) < 0){
        perror("Error: Could not write on socket.");
        exit(EXIT_FAILURE);
    }

    char line[4 + MAXCONTENT + 1] = {0};
    int n = read(sockfd, line, (4 + MAXCONTENT + 1));
    if(n < 1){
        perror("Error: Could not read the socket.");
        exit(EXIT_FAILURE);
    }
    if (atoi(line) > 0) {
        int err;
        int numTokens = sscanf(line, "%d %s", &err, buffer);
        if (numTokens != 2) {
            fprintf(stderr, "Error: invalid args\n");
            exit(EXIT_FAILURE);
        }
        return err;
    }
    int err = atoi(line);
    if (err == TECNICOFS_ERROR_FILE_NOT_OPEN){
        return TECNICOFS_ERROR_FILE_NOT_OPEN;
    } else if(err == TECNICOFS_ERROR_INVALID_MODE) {
        return TECNICOFS_ERROR_INVALID_MODE;
    } else if(err == TECNICOFS_ERROR_OTHER) {
        return TECNICOFS_ERROR_OTHER;
    }
    return 0;
}

int tfsWrite(int fd, char *buffer, int len){
    if(!sockfd){
        perror("client: no active session");
        return TECNICOFS_ERROR_NO_OPEN_SESSION;
    }
    if(dprintf(sockfd, "w %d %s", fd, buffer) < 0){
        perror("Error: Could not write on socket.");
        exit(EXIT_FAILURE);
    }
    char line[4] = {0};

    int n = read(sockfd, line, 4);
    if(n < 1){
        perror("Error: Could not read the socket.");
        exit(EXIT_FAILURE);
    }
    int err = atoi(line);
    if (err == TECNICOFS_ERROR_FILE_NOT_OPEN){
        return TECNICOFS_ERROR_FILE_NOT_OPEN;
    } else if(err == TECNICOFS_ERROR_INVALID_MODE) {
        return TECNICOFS_ERROR_INVALID_MODE;
    } else if(err == TECNICOFS_ERROR_OTHER) {
        return TECNICOFS_ERROR_OTHER;
    }
    return 0;
}
