CC=gcc

CFLAGS= -O0 -g
 
CFILES= client1.c

SRCS= ${CFILES}
OBJS= ${CFILES:.c=.o}

build:	client1

clean:
	-rm -f *.o *.d
	-rm -f client1

client1:	client1.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lm
perform:
	./client1	128.138.201.66	5567
