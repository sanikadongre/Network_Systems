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
		    printf("Enter the command to be performed and type it:get [filename],put [filename],delete [filename],md5sum, ls,exit\n");
		scanf("%s", cmd);
		scanf("%s", fname1);
		printf("First sending the entire command to the server : %s\n",cmd);
		bytestot = sendto(sockfd, cmd, strlen(cmd), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		bytestot1 = sendto(sockfd, fname1, strlen(fname1), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		printf("Number of bytes for the operation sent : %d\n",bytestot);
		printf("Number of bytes for the operation sent : %d\n",bytestot1);
		name_cmd = strdup(cmd);
		fname = strdup(fname1);
		strtok(name_cmd, " ");
		printf("The name of the command is: %s\n", name_cmd);
		fname = strtok(fname, " ");
		printf("The file name is %s\n", fname);
		if(strcmp("get", name_cmd) == 0)
		{
			printf("\nTo obtain the name of the file from the server %s\n", fname);
			get_file(sockfd, fname, serveraddr);
			printf("\nThe file get is done\n");
		}

		else if(strcmp("put", name_cmd) == 0)
		{
			printf("\nTo put the file by the client %s\n", fname);
			put_file(sockfd, fname, serveraddr);
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
			bytestot = recvfrom(sockfd, cmd, strlen(cmd), 0, (struct sockaddr*)&serveraddr, &(serverlen));
			printf("%s\n", val);
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
	 	else
		{
			printf("The entered command isn't appropriate\n");

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
		    n = recvfrom(sockfd, cmd, strlen(cmd), 0, (struct sockaddr*)&serveraddr, &serverlen);
		    if (n < 0) 
		      error("ERROR in recvfrom");
	    printf("Echo from server: %s", cmd);
	}
    	    return 0;
}
