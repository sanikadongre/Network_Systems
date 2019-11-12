CC=gcc

CFLAGS= -O0 -g
 
CFILES= client.c

SRCS= ${CFILES}
OBJS= ${CFILES:.c=.o}

build:	client.elf client1.elf

clean:
	-rm -f *.o *.d
	-rm -f client.elf client1.elf

#For native compile on client
client.elf:	client.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ client.c -lm -pthread 

#For cross-compile on client
client1.elf:	client.c
	arm-linux-gnueabihf-gcc -o client1.elf client.c -lm -pthread
