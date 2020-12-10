# Makefile, versao 3
# Sistemas Operativos, DEI/IST/ULisboa 2019-20
# Guilherme Saraiva 93717, Sara Ferreira 93756

SOURCES = main.c fs.c sync.c
SOURCES+= lib/hash.c lib/bst.c lib/inodes.c
OBJS_NOSYNC = $(SOURCES:%.c=%.o)
OBJS_RWLOCK = $(SOURCES:%.c=%-rwlock.o)
OBJS = $(OBJS_NOSYNC) $(OBJS_RWLOCK)
CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../ -g
LDFLAGS=-lm -pthread
TARGETS = tecnicofs

.PHONY: all clean

all: $(TARGETS)

$(TARGETS):
	$(LD) $(CFLAGS) $^ -o $@ $(LDFLAGS)


### RWLOCK ###
lib/bst-rwlock.o: CFLAGS+=-DRWLOCK
lib/bst-rwlock.o: lib/bst.c lib/bst.h

lib/hash-rwlock.o: CFLAGS+=-DRWLOCK
lib/hash-rwlock.o: lib/hash.c lib/hash.h

lib/inodes-rwlock.o: CFLAGS+=-DRWLOCK
lib/inodes-rwlock.o: lib/inodes.c lib/inodes.h

fs-rwlock.o: CFLAGS+=-DRWLOCK
fs-rwlock.o: fs.c fs.h lib/bst.h lib/hash.h lib/inodes.h sync.h ../Client/tecnicofs-api-constants.h

sync-rwlock.o: CFLAGS+=-DRWLOCK
sync-rwlock.o: sync.c sync.h

main-rwlock.o: CFLAGS+=-DRWLOCK
main-rwlock.o: main.c fs.h unix.h

tecnicofs: lib/inodes-rwlock.o lib/bst-rwlock.o lib/hash-rwlock.o fs-rwlock.o sync-rwlock.o main-rwlock.o


%.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo Cleaning...
	rm -f $(OBJS) $(TARGETS)
