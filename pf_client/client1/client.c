
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
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdint.h>

#define BUFSIZE 1024

/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

typedef struct{
	uint8_t msg[BUFSIZE];
	uint16_t packet_len;
	uint16_t packet_index;



}packet_details;


 void get_file_name(uint8_t* filename, int sock_id, struct sockaddr_in remote)
{
       int msg_file
	if(strcmp(msg_file, "File present") != 0)
	{
		printf(" File not found\n");
	}
 	else
	{
		





int main(int argc, char *argv[]) {
    int sockfd, portno, n, nbytes, end;
    int serverlen, cmd_out, file_name;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];
    uint8_t cmd[50], fname[50];
    
    /* check command line arguments */
    if (argc < 3) {
       fprintf("usage: <hostname> <port>\n");
       exit(1);
    }
  //  hostname = argv[1];
  //  portno = atoi(argv[2]);

    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(argv[1]);
   /* if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
      printf("Error opening socket");
    }*/
    
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR creating socket");
    
    /* gethostbyname: get the server's DNS entry */
    /*server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);*/
    }

    /* build the server's Internet address */
    /*bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);*/

    /* get a message from the user */
    bzero(buf, BUFSIZE);
    printf("Please enter msg: ");
    fgets(buf, BUFSIZE, stdin);
    bzero(cmd, BUFSIZE); 
    bzero(fname, BUFSIZE);
    /* send the message to the server */
    printf("Select a command from these and type it: get, put, delete, ls , exit\n");
    scanf("%s", cmd);
    fgets(cmd, BUFSIZE, stdin);
    printf("Type the name of the file\n");
    cmd_out = strcmp("get", cmd);
    if(cmd_out == 0)
    {
  	 scanf("%s", fname);
    }
    cmd_out = strcmp("put", cmd);
    if(cmd_out == 0)
   {
 	scanf("%s", fname);
   }
   cmd_out = strcmp("delete", cmd);  
   if(cmd_out == 0)
   {
   	scanf("%s", fname);
   }
    serverlen = sizeof(serveraddr);
    n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
    if (n < 0) 
      error("ERROR in sendto");
    nbytes = sendto(sockfd, cmd, strlen(cmd), 0, &serveraddr, serverlen);
    /* print the server's reply */
    n = recvfrom(sockfd, buf, strlen(buf), 0, &serveraddr, &serverlen);
    if (n < 0) 
      error("ERROR in recvfrom");
    n = recvfrom(sockfd, cmd, strlen(cmd), 0, &serveraddr, &serverlen);
    if (n < 0) 
      error("ERROR in recvfrom");
     n = recvfrom(sockfd, fname, strlen(fname), 0, &serveraddr, &serverlen);
    if (n < 0) 
      error("ERROR in recvfrom");

     cmd_out = strcmp(cmd , "Exit")
	if(cmd_out == 0)
	{
		end = close(sockfd);
		if(end == 0)
			{
				printf("Exit the server%d\n", end);
			}
			else
			printf("The server has not exited%d\n", end);
        }
		
    printf("Echo from server: %s", buf);
   if (n < 0) 
      error("ERROR in recvfrom");
    printf("Echo from server: %s", cmd);
    if (n < 0) 
      error("ERROR in recvfrom");
    printf("Echo from server: %s", fname);
    bzero(buf, BUFSIZE);
    bzero(cmd, BUFSIZE);
    bzero(fname, BUFSIZE);
    close(sockfd);
    return 0;
}
