# Makefile, versao 1
# Sistemas Operativos, DEI/IST/ULisboa 2019-20
# Autores: Guilherme Saraiva (93717); Sara Ferreira (93756)

CC   = gcc
LD   = gcc
CFLAGS =-Wall -std=gnu99 -I../
LDFLAGS=-lm
MUTEX = -DMUTEX
RWL =-DRWLOCK

# A phony target is one that is not really the name of a file
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: all clean run

all: tecnicofs-nosync tecnicofs-mutex tecnicofs-rwlock

# no_sync

tecnicofs-nosync: lib/bst.o fs.o main-nosync.o
	$(LD) $(CFLAGS) $(LDFLAGS) -pthread -o tecnicofs-nosync lib/bst.o fs.o main-nosync.o

main-nosync.o: main.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -pthread -o main-nosync.o -c main.c

# mutex

tecnicofs-mutex: lib/bst.o fs.o main-mutex.o
	$(LD) $(CFLAGS) $(LDFLAGS) $(MUTEX) -pthread -o tecnicofs-mutex lib/bst.o fs.o main-mutex.o

main-mutex.o: main.c fs.h lib/bst.h
	$(CC) $(CFLAGS) $(MUTEX) -pthread -o main-mutex.o -c main.c

# rwlock

tecnicofs-rwlock: lib/bst.o fs.o main-rwlock.o
	$(LD) $(CFLAGS) $(LDFLAGS) $(RWL) -pthread -o tecnicofs-rwlock lib/bst.o fs.o main-rwlock.o

main-rwlock.o: main.c fs.h lib/bst.h
	$(CC) $(CFLAGS) $(RWL) -pthread -o main-rwlock.o -c main.c

# common files

lib/bst.o: lib/bst.c lib/bst.h
	$(CC) $(CFLAGS) -o lib/bst.o -c lib/bst.c

fs.o: fs.c fs.h lib/bst.h
	$(CC) $(CFLAGS) -o fs.o -c fs.c

clean:
	@echo Cleaning...
	rm -f lib/*.o *.o tecnicofs-nosync tecnicofs-mutex tecnicofs-rwlock

run: tecnicofs
	./tecnicofs
