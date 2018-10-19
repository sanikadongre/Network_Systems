CC= gcc

CFLAGS=	-O0	-g	-lpthread
 
CFILES= server.c

COBJS= ${CFILES:.c=.o}

build:	server.elf

clean:
	-rm -f server 

server.elf:		
	$(CC)	$(LDFLAGS)	$(CFLAGS)	-o	server.elf	server.c
