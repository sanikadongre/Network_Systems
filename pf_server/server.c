/* 
 * udpserver.c - A simple UDP echo server 
 * usage: udpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <memory.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>


#define BUFSIZE (1024)
struct timeval time_vals, time_val1, time_val2, time_done;

typedef struct
{
	uint32_t packet_index;
	uint8_t packet_descp[BUFSIZE];
	uint32_t packet_len;
}Packet_Details;

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}
void get_file(int socket_id, uint8_t *file_name, struct sockaddr_in remote_addr, uint32_t len_data)
{  
	char command_buffer[BUFSIZE],size_buffer[BUFSIZE];            
	char *file_buffer;
	int temp_bytes;
	int file_exist_confirmation = 0;
	ssize_t fileSize;
	ssize_t encodedFileSize;
	ssize_t size_check;
	int bytes_read = 0;
	int bytes_sent = 0;
	int actual_sequence_count = 0;
	int received_sequence_count = 0;
	int decoded_sequence_count = 0;
	int file_size = 0;

	printf("The client requires the file : %s\n", file_name);

	/*Creating a file pointer to the requested file from the client */
         FILE *fp;
	fp = fopen(file_name,"r");
	if(fp == NULL)
	{
		printf("Sending File does not exist confirmation to client\n");

		bzero(command_buffer,sizeof(command_buffer));
		strcat(command_buffer,"File does not exist");
		file_exist_confirmation = sendto(socket_id, command_buffer, strlen(command_buffer), 0, (struct sockaddr*)&remote_addr, remote_len);
	}
	else
	{		
		printf("Sending File exist confirmation to client\n");

		bzero(command_buffer,sizeof(command_buffer));
		strcpy(command_buffer,"File exist");
		file_exist_confirmation = sendto(socket_id, command_buffer, strlen(command_buffer), 0, (struct sockaddr*)&remote_addr, remote_len);
		printf("File exist confiramtion : %s\n",command_buffer);
	
		bzero(size_buffer,sizeof(size_buffer));
		recvfrom( socket_id, size_buffer, BUFSIZE, 0, (struct sockaddr*)&remote_addr, &remote_len);
		printf("The client says : %s\n", command_buffer);

		printf("Sending the size of the file\n");
		fseek(fp, 0, SEEK_END);
   		fileSize = ftell(fp);
  		encodedFileSize = htonl(fileSize);
		rewind(fp);

		file_size = sendto(socket_id, &encodedFileSize, sizeof(encodedFileSize), 0, (struct sockaddr*)&remote_addr, remote_len);
		printf("File size : %ld\n",fileSize);
		size_check = 0;

		/* Key for encrypting message */
		char key = 10;
		
		/* Setting the timeout for recvfrom function */
		time_vals.tv_sec = 0;
		time_vals.tv_usec = 100000;
		if (setsockopt(socket_id, SOL_SOCKET, SO_RCVTIMEO,&time_vals,sizeof(time_vals)) < 0) {
		    perror("Error");
		}
	
		/* Loop till the entire file is sent */
		while(size_check < fileSize)
		{
			/* Structure for storing the packet to be sent */
			struct Datagram *temp = malloc(sizeof(struct Datagram));
			if(temp != NULL)
			{
				temp->datagram_id = actual_sequence_count;
				bytes_read = fread(temp->datagram_message,sizeof(char),BUFSIZE,fp);

				/* Encrypting the message */
				for(long int i=0;i<bytes_read;i++)
				{

					temp->datagram_message[i] ^= key;
				}

				temp->datagram_length = bytes_read;
				printf("\nSequence count : %d\n",temp->datagram_id);

				bytes_sent = sendto(socket_id, temp, (sizeof(*temp)), 0, (struct sockaddr*)&remote_addr, remote_len);
				
				/* Check for the acknowledgement from client */
				if(recvfrom( socket_id, &received_sequence_count, sizeof(received_sequence_count), 0, (struct sockaddr*)&remote_addr, &remote_len)>0)
				{	
					printf("ACK received %d\n", htonl(received_sequence_count));
					decoded_sequence_count = htonl(received_sequence_count);
					if(decoded_sequence_count == actual_sequence_count)
					{
						/* Incrementing the sequence count and local file size variable */
						actual_sequence_count++;
						size_check = size_check + bytes_read;
						printf("size_check : %ld\n",size_check);
					}
					else
					{
						printf("Sending the same sequence inside receive from %d again",actual_sequence_count);
						fseek(fp, size_check, SEEK_SET);	
					}
				}
				else
				{
					printf("Sending the same sequence %d again",actual_sequence_count);
					fseek(fp, size_check, SEEK_SET);
				}
				free(temp);
			}
		}
		printf("Done\n");
		fclose(fp);	
	}
}
int main(int argc, char **argv)
 {
  int sockfd, portno, clientlen, optval, n, client_socket, bytestot = 0, file_del, exit_recv, bytestot1 = 0; /* socket */
  struct sockaddr_in serveraddr, clientaddr; /* server's addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE], hash_buf[BUFSIZE], recv_buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  uint8_t cmd_out_exit, val[BUFSIZE] = "The server is exiting", cmd[70], fname[70], check = 1;
  bzero(cmd, sizeof(cmd));
  bzero(fname, sizeof(fname));
  bzero(val, sizeof(val));
  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
   {
    	error("ERROR on binding");
   }

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
  while (check) {

    /*
     * recvfrom: receive a UDP datagram from a client
     */
    bzero(buf, BUFSIZE);
    n = recvfrom(sockfd, cmd, BUFSIZE, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0)
      error("ERROR in recvfrom");

    /* 
     * gethostbyaddr: determine who sent the datagram
     */
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server received datagram from %s (%s)\n", 
	   hostp->h_name, hostaddrp);
    printf("server received %d/%d bytes: %s\n", strlen(cmd), n, cmd);
	  	time_val1.tv_sec = 0;
		time_val1.tv_usec = 0;
		/*if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&time_val1,sizeof(time_val1)) < 0);
		{
			perror("Error\n");

                } */
		if(strcmp("get", cmd) == 0)
		{
			get_file(sockfd, fname, clientaddr);
		}

		else if(strcmp("put", cmd) == 0)
		{

		 }
		 else if(strcmp("ls", cmd) == 0)
		{
			
		}
					
		else if(strcmp("delete", cmd) == 0)
		{	
			FILE *f;
			bytestot = recvfrom(sockfd, fname, (BUFSIZE), 0, (struct sockaddr*)&clientaddr, &(clientlen));
			printf("The file name is %s\n", fname);
			f = fopen(fname,"r");			
			if(f != NULL)
			{
				file_del = remove(fname);
				if(file_del != 0)
				{
					perror("Error");
					printf("Enter a valid file name\n");
				}
				if(file_del == 0)
				{
					printf("The file is deleted\n");
				}
			}
			if(f == NULL)
			{
			  	perror("Error");
				printf("The file is not found and can't be deleted\n");
			}
		}
		else if(strcmp("md5sum", cmd) == 0)
		{
			strcpy(hash_buf, "md5sum");
			printf("To get the hash value of the file: %s\n", fname);
			strncat(hash_buf,fname,strlen(fname));
			printf("**************************\n");
			system(hash_buf);
			printf("***************************\n");
		}
		else if(strcmp("exit", cmd) == 0)
		{
			
			
			bzero(fname, sizeof(fname));
			strcat(fname, "Exit");
			printf(" Exit buffer: %s\n", fname);
			bytestot = sendto(sockfd, fname, BUFSIZE, 0, (struct sockaddr*)&clientaddr, clientlen);
			exit(0);

        	 }
		bzero(cmd,sizeof(cmd));
		bzero(fname, sizeof(fname));
		bzero(val,sizeof(val));

    
    /* 
     * sendto: echo the input back to the client 
     */
	   n = sendto(sockfd, cmd, strlen(cmd), 0, 
	       (struct sockaddr *) &clientaddr, clientlen);
    n = sendto(sockfd, cmd, strlen(cmd), 0, 
	       (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0) 
      error("ERROR in sendto");
  }
}
