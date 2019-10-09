CC=gcc

CFLAGS= -O0 -g
 
CFILES= server.c

SRCS= ${CFILES}
OBJS= ${CFILES:.c=.o}

build:	server.elf

clean:
	-rm -f *.o *.d
	-rm -f server.elf

server.elf:	server.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lm
