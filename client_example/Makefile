INCLUDES=	-I/usr/local/opencv/include

CC=	g++

CFLAGS=	-O0	-g	-std=c++11	

CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video -lpthread -lrt 
 
CPPFILES= client.c

CPPOBJS= ${CPPFILES:.c=.o}

build:	client

clean:
	-rm -f *.o *.d *.jpg *.ppm
	-rm -f	client

client:	client.o	
	$(CC)	$(LDFLAGS)	$(INCLUDES)	$(CFLAGS)	-o	client	client.o	`pkg-config --libs opencv`	$(CPPLIBS)
