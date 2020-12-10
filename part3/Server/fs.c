#include "fs.h"
#include "unix.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>


client *newClient(uid_t uidcred){
    client *cl = (client*) malloc(sizeof(client));
    if (!cl) {
        perror("failed to allocate client");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAXFILES; i++){
        cl->files[i].filename = NULL;
        cl->files[i].inumber = -1;
    }
    cl->openedFiles = 0;
    cl->uid = uidcred;
    return cl;
}

tecnicofs* new_tecnicofs(){
    tecnicofs*fs = malloc(sizeof(tecnicofs));
    if (!fs) {
        perror("failed to allocate tecnicofs");
        exit(EXIT_FAILURE);
    }
    fs->hashtable = NULL;
    return fs;
}

void free_tecnicofs(tecnicofs* fs){
    free_tree(fs->hashtable);
    free(fs);
}

void freeClient(client* cl){
    for (int i = 0; i < MAXFILES; i++) {
        if (cl->files[i].inumber != -1) {
            free(cl->files[i].filename);
        }
    }
    free(cl);
}

int create(tecnicofs *fs, char *filename, permission ownerPermissions, permission othersPermissions, uid_t uid){
    lock("write", 3);
    node *searchNode = search(fs->hashtable, filename);

    if (searchNode){
        fprintf(stderr, "Error: File already exists.\n");
        unlock(3);
        return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
    }
    int inumber = inode_create(uid, ownerPermissions, othersPermissions);
    if (inumber == -1){
        fprintf(stderr, "Error: Could not create file(inode).\n");
        unlock(3);
        return TECNICOFS_ERROR_OTHER;
    }
    fs->hashtable = insert(fs->hashtable, filename, inumber);
    unlock(3);
    return 0;
}

int delete(client *cl, tecnicofs* fs, char *name) {
    lock("write", 4);
    if (cl->openedFiles != 0)
        for(int i = 0; i < MAXFILES; i++)
            if(strcmp(cl->files[i].filename, name) == 0){
                fprintf(stderr, "Error: File is opened.\n");
                unlock(4);
                return TECNICOFS_ERROR_FILE_IS_OPEN;
            }
    node *searchNode = search(fs->hashtable, name);
    if (!searchNode){
        fprintf(stderr, "Error: Could not found file.\n");
        unlock(4);
        return TECNICOFS_ERROR_FILE_NOT_FOUND;
    }
    uid_t *owner = malloc(sizeof(uid_t));
    if(!owner){
        fprintf(stderr, "Error: Could not alloc.\n");
        unlock(4);
        return TECNICOFS_ERROR_OTHER;
    }
    if (inode_get(searchNode->inumber, owner, NULL, NULL, NULL, 1) == -1){
        fprintf(stderr, "Error: Could not get value.\n");
        unlock(4);
        return TECNICOFS_ERROR_OTHER;
    }
    if(*owner != cl->uid){
        fprintf(stderr, "Error: filename UID does not correspond to effective process UID.\n");
        unlock(4);
        return TECNICOFS_ERROR_PERMISSION_DENIED;
    }
    int err = inode_delete(searchNode->inumber);
    if (err != 0){
        fprintf(stderr, "Error: Could not delete file(inode).\n");
        unlock(4);
        return TECNICOFS_ERROR_OTHER;
    }
    fs->hashtable = remove_item(fs->hashtable, name);
    free(owner);
    unlock(4);
    return 0;
}

/* function that renames a file */
int changeName(client* cl, tecnicofs* fs, char *oldName, char *newName){
    lock("write", 5);
    if (cl->openedFiles != 0)
        for(int i = 0; i < MAXFILES; i++)
            if(strcmp(cl->files[i].filename, oldName) == 0){
                fprintf(stderr, "Error: File is opened.\n");
                unlock(5);
                return TECNICOFS_ERROR_FILE_IS_OPEN;
            }

    node *searchNode = search(fs->hashtable, oldName);
    if (!searchNode){
        fprintf(stderr, "Error: Could not found file.\n");
        unlock(5);
        return TECNICOFS_ERROR_FILE_NOT_FOUND;
    }
    uid_t *owner = malloc(sizeof(uid_t));
    if(!owner){
        fprintf(stderr, "Error: Could not alloc.\n");
        unlock(5);
        return TECNICOFS_ERROR_OTHER;
    }
    if (inode_get(searchNode->inumber, owner, NULL , NULL, NULL, 0) == -1){
        fprintf(stderr, "Error: Could not get value.\n");
        unlock(5);
        return TECNICOFS_ERROR_OTHER;
    }
    if(*owner != cl->uid){
        fprintf(stderr, "Error: filename UID does not correspond to effective process UID.\n");
        unlock(5);
        return TECNICOFS_ERROR_PERMISSION_DENIED;
    }
    node *searchNode1 = search(fs->hashtable, newName);
    if (searchNode1){
        fprintf(stderr, "Error: File already exists.\n");
        unlock(5);
        return TECNICOFS_ERROR_FILE_ALREADY_EXISTS;
    }
    fs->hashtable = insert(fs->hashtable, newName, searchNode->inumber);
    fs->hashtable = remove_item(fs->hashtable, oldName);

    free(owner);
    unlock(5);
    return 0;
}

int openFile (client *cl, tecnicofs *fs, char *filename, permission mode){
    lock("write", 6);

    node *searchNode = search(fs->hashtable, filename);
    if (!searchNode){
        fprintf(stderr, "Error: Could not found file.\n");
        unlock(6);
        return TECNICOFS_ERROR_FILE_NOT_FOUND;
    }
    if (cl->openedFiles != 0)
        for(int i = 0; i < MAXFILES; i++)
            if(cl->files[i].inumber == searchNode->inumber){
                fprintf(stderr, "Error: File is opened.\n");
                unlock(6);
                return TECNICOFS_ERROR_FILE_IS_OPEN;
            }
    uid_t *owner = malloc(sizeof(uid_t));
    permission *perm = malloc(sizeof(permission));
    permission *otherperm = malloc(sizeof(permission));
    if(!perm || !otherperm){
        fprintf(stderr, "Error: Could not alloc.\n");
        unlock(6);
        return TECNICOFS_ERROR_OTHER;
    }
    int len = MAXCONTENT + 1;
    if (inode_get(searchNode->inumber, owner, perm , otherperm, NULL, len) == -1){
        fprintf(stderr, "Error: Could not get value.\n");
        unlock(6);
        return TECNICOFS_ERROR_OTHER;
    }
    if(*owner == cl->uid) {
        if (*perm != mode && *perm != RW) {
            fprintf(stderr, "Error: Client does not have permission to open the file.\n");
            unlock(6);
            return TECNICOFS_ERROR_INVALID_MODE;
        }
    } else {
        if (*otherperm != mode && *perm != RW) {
            fprintf(stderr, "Error: Client does not have permission to open the file.\n");
            unlock(6);
            return TECNICOFS_ERROR_INVALID_MODE;
        }
    }
    if(cl->openedFiles == MAXFILES){
        fprintf(stderr, "Error: Client cannot open more files (MAX 5).\n");
        unlock(6);
        return TECNICOFS_ERROR_MAXED_OPEN_FILES;
    }
    /* adiciona ao vet de files abertos */
    int i;
    for (i = 0; i < MAXFILES; i++) {
        if (cl->files[i].inumber == -1){
            cl->files[i].filename = malloc(sizeof(char) * (strlen(filename) + 1));
            strncpy(cl->files[i].filename,  filename, strlen(filename));
            cl->files[i].filename[strlen(filename)] = '\0';
            cl->files[i].mode = mode;
            cl->files[i].inumber = searchNode->inumber;
            break;
        }
    }
    cl->openedFiles++;
    free(perm);
    free(otherperm);
    free(owner);
    unlock(6);
    return i;
}

int closeFile(client *cl,  int fd){
    lock("write", 7);
    if(cl->files[fd].inumber == -1){
        fprintf(stderr, "Error: No such file with descriptor %d.\n", fd);
        unlock(7);
        return TECNICOFS_ERROR_FILE_NOT_OPEN;
    }
    cl->files[fd].filename = NULL;
    cl->files[fd].inumber = -1;
    cl->openedFiles--;
    unlock(7);
    return 0;
}

int readFile(client  *cl, int fd, char *buffer, int len){
    lock("write", 8);
    if(cl->files[fd].inumber == -1){
        fprintf(stderr, "Error: No such file with descriptor %d.\n", fd);
        unlock(8);
        return TECNICOFS_ERROR_FILE_NOT_OPEN;
    }
    if(cl->files[fd].mode != READ && cl->files[fd].mode != RW) {
        fprintf(stderr, "Error: No permission to read.\n");
        unlock(8);
        return TECNICOFS_ERROR_INVALID_MODE;
    }
    char* content = malloc(sizeof(char) * (len));
    if(!content){
        fprintf(stderr, "Error: Could not alloc.\n");
        unlock(8);
        return TECNICOFS_ERROR_OTHER;
    }
    if (inode_get(cl->files[fd].inumber, NULL, NULL , NULL, content, len - 1) == -1){
        fprintf(stderr, "Error: Could not get value.\n");
        unlock(8);
        return TECNICOFS_ERROR_OTHER;
    }
    if(strlen(content) > 0 && len > 0){
        strncpy(buffer, content, len);
        int n = strlen(buffer);
        buffer[n] = '\0';
        free(content);
        unlock(8);
        return n;
    }
    fprintf(stderr, "Error: Could not copy.\n");
    free(content);
    unlock(8);
    return TECNICOFS_ERROR_OTHER;
}

int writeFile(client *cl, int fd, char *buffer, int len){
    lock("write", 9);
    if(cl->files[fd].inumber == -1){
        fprintf(stderr, "Error: No such file with descriptor %d.\n", fd);
        unlock(9);
        return TECNICOFS_ERROR_FILE_NOT_OPEN;
    }
    if(cl->files[fd].mode != RW && cl->files[fd].mode != WRITE){
        fprintf(stderr, "Error: No permission to write.\n");
        unlock(9);
        return TECNICOFS_ERROR_INVALID_MODE;
    }
    int err = inode_set(cl->files[fd].inumber, buffer, len);
    if (err != 0){
        fprintf(stderr, "Error: Could not update the i-node file content.");
        unlock(9);
        return TECNICOFS_ERROR_OTHER;
    }
    unlock(9);
    return 0;
}

void print_tecnicofs_tree(FILE * fp, tecnicofs *fs){
    print_tree(fp, fs->hashtable);
}
