TARGETS=ringmaster player
CFLAGS=-Werror -Wall -ggdb3 -pthread 
all: $(TARGETS)
clean:
	rm -f $(TARGETS) *.o *~

player: player.o potato.o potato.h
	gcc $(CFLAGS) -o $@ player.o potato.o
ringmaster: ringmaster.o potato.o potato.h
	gcc $(CFLAGS) -o $@ ringmaster.o potato.o

%.o:%.c
	gcc $(CFLAGS) -c $<
