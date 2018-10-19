#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <netdb.h>
#include <netinet/in.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <dirent.h>
#include <stdlib.h>

#define BUFSIZE (1024)
#define size_elements (64)
#defibe element_number (64)
typedef struct{

  uint8_t req_url[BUFFER];
  uint8_t req_version[BUFFER];
  uint8_t req_method[BUFFER];
}request_info;

struct timeval time_val;

typedef struct{
  int port;
  int timeout;
  uint8_t* root_path;
  uint8_t content[size_elements] [element_number];
  uint8_t file[size_elements] [element_number];
}struct_main;

typedef enum {
	  invalidmeth = 0,
          invalidurl,
          invalidhttp
}req_error;

int main(int argc, char* argv[])
{
	int sock, bytestot, new_sock, rc;
        uint8_t buffer[BUFSIZE];
	struct_main parsing;
	struct sockaddr_in serveraddr, clientaddr;
	
 if((sock = socket(AF_INET, SOCK_STREAM, 0) < 0)
{
	printf("Error in creating socket\n");
}

int fileparse(

bzero(&serveraddr, sizeof(serveraddr));
serveraddr.sin_family =  AF_INET;
serveraddr.sin_port = htons(parsing.port);
serveraddr.sin_addr.s_addr = INADDR_ANY;


