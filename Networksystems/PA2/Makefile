CC= gcc

CFLAGS=	-O0	-g
 
CFILES= server.c

COBJS= ${CFILES:.c=.o}

build:	clean	server.elf

clean:
	-rm -f server	server.elf 

server.elf:		
	gcc	-pthread	-o	server.elf	server.c
