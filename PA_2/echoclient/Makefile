CC=gcc

CFLAGS= -O0 -g -lpthread
 
CFILES= client1.c

SRCS= ${CFILES}
OBJS= ${CFILES:.c=.o}

build:	client1.elf

clean:
	-rm -f *.o *.d
	-rm -f client1

client1.elf:	
	$(CC)	$(LDFLAGS)	$(CFLAGS)	-o	client1.elf	client1.c

do:
	./client1	5567	128.138.201.66
	


