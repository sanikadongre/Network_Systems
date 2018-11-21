
CC=gcc

CFLAGS= -O0 -g 

HFILES= webproxy.h
CFILES= webproxy.c

SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

build:	webproxy

clean:
	-rm -f *.o *.d
	-rm -f webproxy

webproxy:	webproxy.o
	$(CC)	$(LDFLAGS)	$(CFLAGS)	-o	$@	$@.o	-lpthread	-lcrypto	-lssl

%.o:%.c	webproxy.h
	$(CC) $(CFLAGS) -c $<
