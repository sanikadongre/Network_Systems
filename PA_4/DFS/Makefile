
CC=gcc

CFLAGS= -O0 -g 

CFILES= dfs.c

SRCS= ${CFILES}

OBJS= ${CFILES:.c=.o}

build:	dfs

clean:
	-rm -f *.o *.d
	-rm -f dfs

dfs:	dfs.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o dfs dfs.o -lpthread

.c.o:
	$(CC) $(CFLAGS) -c $<
