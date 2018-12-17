
CC=gcc

CFLAGS= -O0 -g

CFILES= dfc.c

SRCS= ${CFILES}

OBJS= ${CFILES:.c=.o}

build:	dfc

clean:
	-rm -f *.o *.d
	-rm -f dfc

dfc:	dfc.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o dfc dfc.o -lpthread -lcrypto -lssl

.c.o:
	$(CC) $(CFLAGS) -c $<
