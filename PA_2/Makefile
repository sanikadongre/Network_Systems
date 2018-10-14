CC= gcc

CFLAGS=	-O0	-g	-lpthread
 
CFILES= server.c

COBJS= ${CFILES:.c=.o}

build:	server

clean:
	-rm -f server 

server:	server.o	
	$(CC)	$(LDFLAGS)	$(CFLAGS)	-o	server.c	server.o