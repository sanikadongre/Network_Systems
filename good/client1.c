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
#include <fcntl.h>

#define BUFSIZE 1024
#define pk_size 32

uint8_t msg_send[] = "Done";
typedef struct
{
	uint8_t packet_descp[BUFSIZE];
	uint32_t packet_len;
	uint32_t packet_index;
}Packet_Details;

struct timeval time_vals, time_val1, time_val2, time_done;
/* cmd_get
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

int32_t file_send(int socket_id, uint8_t* file_name, struct sockaddr_in remote)
{
	int32_t file_closing = 0, send_data =0;
	uint8_t* data_send = (uint8_t*)malloc(pk_size);
	FILE *fptr;
	if(!access(file_name, F_OK)
	{
		fptr = fopen(file_name, "r");
		while(file_closing! = 1)
		{
			fgets(data_send, pk_size, fptr)
			file_closing = feof(fptr);
			if(file_closing == 1)
			{
				break;
			}
			if(fptr != NULL)
			{
				if(data_send != NULL)
				{
					send_data = sendto(socket_id, data_send, 32, 0, (struct sockaddr*)&remote, sizeof(remote));
				}
			 }
			
			}
			fclose(fptr);
		}
		else
		{
			file_closing = 0;
		}
	send_data = sendto(socket_id, data_send, 32, 0, (struct sockaddr*)&remote, sizeof(remote));
	return file_closing;		
}

void file_obtain(int socket_id, uint8_t* file_name, struct sockaddr_in remote)
{
	int32_t flag =0, k=0;
	uint8_t* data_read = (uint8_t*)malloc(pk_size);
	uint8_t check_val = 1;
	FILE *fptr = fopen(file_name, "w");
	int remote_len = sizeof(remote);
	while(check_val)
	{
		k= recvfrom(socket_id, data_read, 32, 0, (struct sockaddr*)&remote, remote_len);
		if(strcmp(data_read, msg_send) == 0)
		{
			check_val =0;
		}
		else if(data_read!=NULL)
		{
			flag = fputs(data_read, fptr);
		}
		else
		{
			check_val =0;
		}
		
         }
	fclose(fptr);
}
		

int main(int argc, char **argv) {
    int sockfd, portno, bytestot, bytestot1, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    uint8_t hash_buf[100], val[BUFSIZE], cmd[70], fname1[70];
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
		printf("The name of the command is: %s\n", cmd);
		printf("The file name is %s\n", fname1);
		if(strcmp("get", name_cmd) == 0)
		{
			file_obtain(sockfd, fname, serveraddr);
		
		}

		else if(strcmp("put", name_cmd) == 0)
		{
			printf("\nTo put the file by the client %s\n", fname);
			//put_file(sockfd, fname, serveraddr);
			printf("\nThe file put is done\n");
		 }
		 else if(strcmp("ls", name_cmd) == 0)
		{
			printf("\nTo list all the files in the directory%s\n", buf);
			bzero(buf, sizeof(buf));
			bytestot = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)&serveraddr, &(serverlen));
			printf("\nThe dircetories and files are \n");
			printf("%s\n", buf);
			printf("1.txt\n");
			
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
		    serverlen = sizeof(serveraddr);
		    n = sendto(sockfd, cmd, strlen(cmd), 0, (struct sockaddr*)&serveraddr, serverlen);
		    if (n < 0) 
		      error("ERROR in sendto");
		    
		    /* print the server's reply */
		 n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serveraddr, &serverlen);
		    if (n < 0) 
		      error("ERROR in recvfrom");
	}
    	    return 0;
}
