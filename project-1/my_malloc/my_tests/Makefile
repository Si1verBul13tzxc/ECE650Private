CC=gcc
CFLAGS=-O3 -fPIC
MALLOC_VERSION=FF
WDIR=../

all: my_test1

my_test1: test1.c
	$(CC) $(CFLAGS) -I$(WDIR) -L$(WDIR) -D$(MALLOC_VERSION) -Wl,-rpath=$(WDIR) -o $@ test1.c -lmymalloc -lrt

clean:
	rm -f *~ *.o my_test1

clobber:
	rm -f *~ *.o
