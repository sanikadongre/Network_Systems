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
#include <signal.h>
#include <fcntl.h>

#define BUFSIZE (1024)

typedef struct
{
	uint8_t packet_descp[BUFSIZE];
	int packet_len;
	int byte;
	int packet_index;
	int packet_ack;
}Packet_Details;

void data_decrypt(uint8_t *temp_buf, int len_d, uint8_t key[], uint8_t key1[])
{
	for(int values = 0; values < len_d; values++)
	{
		temp_buf[values] ^= key[values%3];
	}
	for(int val2 = 0; val2 < len_d; val2++)
	{
		temp_buf[val2] ^= key1[val2%5];
	}
}

void data_encrypt(uint8_t* temp_buf, int len_d, uint8_t key[], uint8_t key1[])
{
	for(int values = 0; values < len_d; values++)
	{
		temp_buf[values] ^= key1[values%5];
	}
        for(int val2 = 0; val2 < len_d; val2++)
	{
		temp_buf[val2] ^= key[val2%3];
	}
}

int cond = 1;

/* cmd_get
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
    uint8_t hash_buf[100], val[BUFSIZE], cmd[70], fname1[70], key[4] = {'A', 'B', '5', '9'}, key1[6] = {'K', 'L', '8', '6'};
    uint8_t* fname;
    uint8_t* name_cmd;
    struct hostent *server;
    char *hostname;
    FILE *fptr;
    char buf[BUFSIZE];
    bzero(cmd, sizeof(cmd));
    bzero(fname1, sizeof(fname1));
    bzero(val, sizeof(val));
    Packet_Details* buf_pkt = malloc(sizeof(Packet_Details));
    Packet_Details* pkt_ack = malloc(sizeof(Packet_Details));
    struct timeval timeout;
    int read_length, info_send;
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
    serverlen = sizeof(serveraddr);
    /* get a message from the user */
    bzero(buf, BUFSIZE);
    while(1)
  	{
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
		{
			perror("Error\n");
	        }
		printf("Enter the command to be performed and type it:get [filename],put [filename],delete [filename],md5sum, ls,exit\n");
		scanf("%s", cmd);
		if(strcmp("get", cmd) == 0)
		{	
			scanf("%s", fname1);
		}
		if(strcmp("put", cmd) == 0)
		{
			scanf("%s", fname1);
		}
		if(strcmp("delete", cmd) == 0)
		{
			scanf("%s", fname1);
		}

		printf("First sending the command to the server : %s\n",cmd);
		bytestot = sendto(sockfd, cmd, strlen(cmd), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		bytestot1 = sendto(sockfd, fname1, strlen(fname1), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		printf("Number of bytes for the operation sent : %d\n",bytestot);
		//printf("The name of the command is: %s\n", name_cmd);
		//printf("The file name is %s\n", fname);
		if(strcmp("get", name_cmd) == 0)
		{
			printf("\nTo obtain the name of the file from the server %s\n", fname);
			int exp_index = 1;
			bytestot = recvfrom(sockfd,fname, strlen(fname), 0, (struct sockaddr*)&serveraddr, &serverlen);
			if(!strcmp(fname, "Error"))
			{
				printf("File doesnt exist on server\n");
				continue;
			}
			do{	
				bzero(buf_pkt->packet_descp, sizeof(buf_pkt->packet_descp));
				bytestot = recvfrom(sockfd, (Packet_Details*)buf_pkt, sizeof(Packet_Details), 0, (struct sockaddr *) &serveraddr, &serverlen);

				printf("Packet Size from the server: %d \n",bytestot);

				data_decrypt(buf_pkt->packet_descp, buf_pkt->byte, key, key1);// Decrypting the encoded data from server

				if(buf_pkt->packet_index == exp_index)
				{
					FILE* fptr;
					fptr = fopen(fname, "ab");
					fwrite(buf_pkt->packet_descp, buf_pkt->byte, 1, fptr);
					bzero(pkt_ack->packet_descp, sizeof(pkt_ack->packet_descp));
					fclose(fptr);
					pkt_ack->packet_ack = exp_index;
					bytestot = sendto(sockfd, (Packet_Details*)pkt_ack, sizeof(Packet_Details), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
					printf("%d packet size and %d ack\n", bytestot, pkt_ack->packet_ack);
					exp_index++;
				}
				else{
					pkt_ack->packet_ack = buf_pkt->packet_index;
					bytestot = sendto(sockfd, (Packet_Details*)pkt_ack, sizeof(Packet_Details), 0, (struct sockaddr*)&serveraddr, serverlen);
				}
				if(buf_pkt->byte != BUFSIZE)
				{
					break;
				}
			}while(1);
			memset(pkt_ack, 0, sizeof(Packet_Details));
			memset(buf_pkt, 0, sizeof(Packet_Details));
		}

		else if(strcmp("put", name_cmd) == 0)
		{
			printf("\nTo put the file by the client %s\n", fname);
			//put_file(sockfd, fname1, serveraddr);
			printf("\nThe file put is done\n");
		 }
		else if(strcmp("ls", name_cmd) == 0)
		{
			printf("\nTo list all the files in the directory%s\n", fname);
			//list_files(sockfd, fname, serveraddr);
			printf("\nThe dircetories and files are listed\n");
		}
					
		else if(strcmp("delete", name_cmd) == 0)
		{	
       			printf("Deleting the file with name: %s\n",fname);
			/* Sending information of the required file */
			bytestot = sendto(sockfd, fname, strlen(fname), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		}
		
		else if(strcmp("exit", name_cmd) == 0)
		{
	 		printf("Exiting the server\n");
			bzero(fname, sizeof(fname));
			bytestot = recvfrom(sockfd, fname, strlen(fname), 0, (struct sockaddr*)&serveraddr, &(serverlen));
			printf("%s\n", fname);
			if(strcmp(fname, "Exit") ==0)
			{
				printf("The server has exited successfully\n");
			}
			else
			{
				printf("There is error in exiting the server\n");
			}
		}
		else if(strcmp("md5sum", name_cmd) == 0)
		{
			strcpy(hash_buf, "md5sum");
			printf("To get the hash value of the file: %s\n", fname);
			strncat(hash_buf,fname,strlen(fname));
			printf("**************************\n");
			system(hash_buf);
			printf("***************************\n");
		}
		/* Resetting the local variables */
 		bzero(cmd,sizeof(cmd));
		bzero(val, sizeof(val));
		bzero(fname1, sizeof(fname1));
		    /* send the message to the server */

		    n = sendto(sockfd, cmd, strlen(cmd), 0, (struct sockaddr*)&serveraddr, serverlen);
		    if (n < 0) 
		 {
		      error("ERROR in sendto");
		 }
		    
		    /* print the server's reply */
		    n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serveraddr, &serverlen);
		    if (n < 0) 
		{
		      error("ERROR in recvfrom");
		}
	    	     printf("Echo from server: %s\n", buf);
		    n = recvfrom(sockfd, cmd, strlen(cmd), 0, (struct sockaddr*)&serveraddr, &serverlen);
		    if (n < 0) 
		{
		      error("ERROR in recvfrom");
		}
	    printf("Echo from server: %s\n", cmd);
	}
    	    return 0;
}
