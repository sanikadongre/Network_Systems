/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <memory.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>

#define BUFSIZE 1024

typedef struct
{
	uint8_t packet_descp[BUFSIZE];
	uint32_t packet_len;
	uint32_t packet_index;
}Packet_Details;

struct timeval time_vals, time_val1, time_val2, time_done;
/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno, bytestot, bytestot1, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    uint8_t hash_buf[100], val[BUFSIZE], cmd[70], fname1[70], cmd_out_exit;
    uint8_t* fname;
    uint8_t* name_cmd;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];
    bzero(cmd, sizeof(cmd));
    bzero(fname1, sizeof(fname1));
    bzero(val, sizeof(val));

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
    {
        error("ERROR opening socket");
    }

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

    /* get a message from the user */
    bzero(buf, BUFSIZE);
    while(1)
  	{
		    time_val1.tv_sec = 0;
		    time_val1.tv_usec = 0;
		    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &time_val1, sizeof(time_val1)) < 0)
		   {
			perror("Error\n");
		    }
		    printf("Please enter msg: ");
		    fgets(buf, BUFSIZE, stdin);
		
		    /* send the message to the server */
		    serverlen = sizeof(serveraddr);
		    n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serveraddr, serverlen);
		    if (n < 0) 
		      error("ERROR in sendto");
		    
		    /* print the server's reply */
		    n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serveraddr, &serverlen);
		    if (n < 0) 
		      error("ERROR in recvfrom");
	    printf("Echo from server: %s", buf);
	}
    	    return 0;
}
