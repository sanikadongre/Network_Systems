CC=gcc

CFLAGS= -O0 -g
 
CFILES= server.c

SRCS= ${CFILES}
OBJS= ${CFILES:.c=.o}

build:	server

clean:
	-rm -f *.o *.d
	-rm -f server

server:	server.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lm
perform:
	./server	5567
add:
	scp	Makefile	server.c	server	sado9543@elra-01.cs.colorado.edu:~/
upload:
	ssh	sado9543@elra-01.cs.colorado.edu:~/

