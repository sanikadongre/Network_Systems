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

#define BUFSIZE 1024

typedef struct
{
	uint16_t index_packet;
	uint16_t length;
	uint8_t pack_size[BUFSIZE];
}Packet_details;

void encryptdata(int buf_size, uint8_t* buffer, uint8_t* encrypteddata)
{
	uint8_t encryptedkey[8] = {'A','B','C','D','1', '2', '3', '4'};
	int key_index=0;
	for(int i=0;i<buf_size;i++)
	{
		encrypteddata[i] = buffer[i] + encryptedkey[key_index];
		key_index++;
		if(key_index==8)
		{
			key_index=0;
		}
	}
}

void decryptdata(int buf_size, uint8_t* buffer, uint8_t* decrypteddata){
	uint8_t encryptedkey[8] = {'A','B','C','D','1','2','3', '4'};
	int key_index=0;
	for(int i=0;i<datasize;i++)
	{
		decryptedbuff[i] = buffer[i] - encryptedkey[key_index] ;
		key_index++;
		if(key_index==8)
		{
			key_index=0;
		}
	}
}


void get(uint8_t* file_name, int sock, struct sockaddr_in remote)
{
	int file_receive_check = 0;
	uint8_t message[BUFSIZE] = " ";
	file_receive_check = recvfrom(sock, message, sizeof(message), 0 , NULL, NULL);
	
/* 
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}
int main(int argc, char **argv) {
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

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
        error("ERROR opening socket");

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
    printf("Please enter msg: ");
    fgets(buf, BUFSIZE, stdin);

    /* send the message to the server */
    serverlen = sizeof(serveraddr);
    n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
    if (n < 0) 
      error("ERROR in sendto");
    
    /* print the server's reply */
    n = recvfrom(sockfd, buf, strlen(buf), 0, &serveraddr, &serverlen);
    if (n < 0) 
      error("ERROR in recvfrom");
    printf("Echo from server: %s", buf);
    return 0;
}
