#ifndef TECNICOFS_CLIENT_API_H
#define TECNICOFS_CLIENT_API_H

#include "tecnicofs-api-constants.h"
#include "../Server/unix.h"
#define MAXCONTENT 250

int sockfd, servlen;
struct sockaddr_un serv_addr;

int tfsCreate(char *filename, permission ownerPermissions, permission othersPermissions);
int tfsDelete(char *filename);
int tfsRename(char *filenameOld, char *filenameNew);
int tfsOpen(char *filename, permission mode);
int tfsClose(int fd);
int tfsRead(int fd, char *buffer, int len);
int tfsWrite(int fd, char *buffer, int len);
int tfsMount(char * address);
int tfsUnmount();

#endif /* TECNICOFS_CLIENT_API_H */
