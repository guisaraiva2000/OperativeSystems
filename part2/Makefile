# Makefile, versao 2
# Sistemas Operativos, DEI/IST/ULisboa 2019-20
# Guilherme Saraiva 93717, Sara Ferreira 93756

SOURCES = main.c fs.c sync.c sem.c
SOURCES+= lib/hash.c lib/bst.c
OBJS_NOSYNC = $(SOURCES:%.c=%.o)
OBJS_MUTEX  = $(SOURCES:%.c=%-mutex.o)
OBJS_RWLOCK = $(SOURCES:%.c=%-rwlock.o)
OBJS = $(OBJS_NOSYNC) $(OBJS_MUTEX) $(OBJS_RWLOCK)
CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../ -g
LDFLAGS=-lm -pthread
TARGETS = tecnicofs-nosync tecnicofs-mutex tecnicofs-rwlock

.PHONY: all clean

all: $(TARGETS)

$(TARGETS):
	$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)


### no sync ###
lib/bst.o: lib/bst.c lib/bst.h
lib/hash.o: lib/hash.c lib/hash.h
fs.o: fs.c fs.h lib/bst.h
sync.o: sync.c sync.h
sem.o: sem.c sem.h
main.o: main.c fs.h lib/bst.h sem.h
tecnicofs-nosync: lib/bst.o fs.o sync.o lib/hash.o sem.o main.o

### MUTEX ###
lib/bst-mutex.o: CFLAGS+=-DMUTEX
lib/bst-mutex.o: lib/bst.c lib/bst.h

lib/hash-mutex.o: CFLAGS+=-DMUTEX
lib/hash-mutex.o: lib/hash.c lib/hash.h

fs-mutex.o: CFLAGS+=-DMUTEX
fs-mutex.o: fs.c fs.h lib/bst.h

sync-mutex.o: CFLAGS+=-DMUTEX
sync-mutex.o: sync.c sync.h

sem-mutex.o: CFLAGS+=-DMUTEX
sem-mutex.o: sem.c sem.h

main-mutex.o: CFLAGS+=-DMUTEX
main-mutex.o: main.c fs.h lib/bst.h lib/hash.h sync.h sem.h
tecnicofs-mutex: lib/bst-mutex.o lib/hash-mutex.o fs-mutex.o sync-mutex.o sem-mutex.o main-mutex.o

### RWLOCK ###
lib/bst-rwlock.o: CFLAGS+=-DRWLOCK
lib/bst-rwlock.o: lib/bst.c lib/bst.h

lib/hash-rwlock.o: CFLAGS+=-DRWLOCK
lib/hash-rwlock.o: lib/hash.c lib/hash.h

fs-rwlock.o: CFLAGS+=-DRWLOCK
fs-rwlock.o: fs.c fs.h lib/bst.h

sync-rwlock.o: CFLAGS+=-DRWLOCK
sync-rwlock.o: sync.c sync.h

sem-rwlock.o: CFLAGS+=-DRWLOCK
sem-rwlock.o: sem.c sem.h

main-rwlock.o: CFLAGS+=-DRWLOCK
main-rwlock.o: main.c fs.h lib/bst.h lib/hash.h sync.h sem.h
tecnicofs-rwlock: lib/bst-rwlock.o lib/hash-rwlock.o fs-rwlock.o sync-rwlock.o sem.o main-rwlock.o


%.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo Cleaning...
	rm -f $(OBJS) $(TARGETS)
