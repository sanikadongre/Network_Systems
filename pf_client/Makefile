CC=gcc

CFLAGS= -O0 -g
 
CFILES= client.c

SRCS= ${CFILES}
OBJS= ${CFILES:.c=.o}

build:	client

clean:
	-rm -f *.o *.d
	-rm -f client

client: client.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lm
