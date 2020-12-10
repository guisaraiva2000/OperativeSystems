#ifndef FS_H
#define FS_H

#include <sys/types.h>
#include "lib/bst.h"
#include "lib/hash.h"
#include "lib/inodes.h"
#include "sync.h"
#include "../Client/tecnicofs-api-constants.h"

#define MAXFILES 5
#define MAXCONTENT 250

typedef struct OpenedFile{
    char * filename;
    permission mode;
    int inumber;
} openedFile;

typedef struct Client{
    openedFile files[MAXFILES];
    int openedFiles;
    uid_t uid;
} client;

typedef struct tecnicofs {
    node* hashtable;
} tecnicofs;

client *newClient(uid_t uidcred);
tecnicofs* new_tecnicofs();
void free_tecnicofs(tecnicofs* fs);
void freeClient(client* cl);
int readFile (client *cl, int fd, char *buffer, int len);
int openFile (client *cl, tecnicofs *fs, char *filename, permission mode);
int closeFile(client *cl, int fd);
int writeFile(client *cl, int fd, char *buffer, int len);
int create(tecnicofs *fs, char *filename, permission ownerPermissions, permission othersPermissions, uid_t uid);
int delete(client *cl, tecnicofs* fs, char *name);
int changeName(client *cl, tecnicofs* fs, char *oldName, char *newName);
void print_tecnicofs_tree(FILE * fp, tecnicofs *fs);

#endif /* FS_H */