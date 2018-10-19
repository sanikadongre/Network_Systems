CC= gcc

CFLAGS=	-O0	-g	
 
CFILES= server.c

COBJS= ${CFILES:.c=.o}

build:	server.elf

clean:
	-rm -f server 

server.elf:		
	$(CC)	$(LDFLAGS)	$(CFLAGS)	-lpthread	-pthread	-o	server.elf	server.c
