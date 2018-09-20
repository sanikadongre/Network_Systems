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
void get_file(int socket_id, uint8_t *name_file, struct sockaddr_in remote_add, uint32_t len_data)
{  
	
	char  msg_conf[BUFSIZE]=" ", sizef[BUFSIZE], temp_file[BUFSIZE];          
	int temp_bytes;
	int exist_conf = 0;
	ssize_t size_file = 0;
	ssize_t file_encrypted = 0;
	ssize_t conff_size;
	int obt_bytes = 0, value2 = 0;
	int info_send = 0;
	int bytes_received = 0;
	int seq_get = 0, seq = 0, seq_check = 0, seq_dec = 0;
	int fs = 0, val2 = 0;
        FILE *fptr;
	uint8_t nbuf[BUFSIZE], key[4] = {'A', 'B', '5' , '9'};
	int remote_len = sizeof(remote_add);
	fptr = fopen(name_file,"r");
	if(fptr != NULL)
	{
		bzero(msg_conf,sizeof(msg_conf));
		strcpy(msg_conf,"File present");
		exist_conf = sendto(socket_id, msg_conf, strlen(msg_conf), 0, (struct sockaddr*)&remote_add, remote_len);
		printf("The file present is: %s\n", msg_conf);
	        bzero(sizef,sizeof(sizef));
		recvfrom(socket_id, sizef, BUFSIZE, 0, (struct sockaddr*)&remote_add, &remote_len);
		printf("The client says : %s\n", msg_conf);
		printf("Sending the size of the file\n");
		fseek(fptr, 0, SEEK_END);
   		size_file = ftell(fptr);
  		file_encrypted = htonl(size_file);
		rewind(fptr);
		fs = sendto(socket_id, &file_encrypted, sizeof(file_encrypted), 0, (struct sockaddr*)&remote_add, remote_len);
		printf("The size of the file is: %ld\n", size_file);
		conff_size = 0;
		time_vals.tv_sec = 0;
		time_vals.tv_usec = 100000;
		if (setsockopt(socket_id, SOL_SOCKET, SO_RCVTIMEO,&time_vals,sizeof(time_vals)) < 0) 
		{
		    perror("Error");
		}
		while(conff_size < size_file)
		{
			Packet_Details *buf_pkt = malloc(sizeof(Packet_Details));  
			if(buf_pkt != NULL)
			{
				buf_pkt->packet_index = seq;
				obt_bytes = fread(buf_pkt->packet_descp,sizeof(uint8_t),BUFSIZE,fptr);
				buf_pkt->packet_len = obt_bytes;
				for(int values =0; values<buf_pkt->packet_len; values++)
				{

					{
						buf_pkt->packet_descp[values] = nbuf[values] - key[value2];
						val2++;
						if(val2 == 3)
						{
							val2 = 0;
						}
					}
				}
				printf("\nSequence count : %d\n", buf_pkt->packet_index);
				info_send = sendto(socket_id, buf_pkt, (sizeof(*buf_pkt)), 0, (struct sockaddr*)&remote_add, remote_len);
				seq_check = recvfrom(socket_id, &seq_get, sizeof(seq_get), 0, (struct sockaddr*)&remote_add, &remote_len)
				if(seq_check < 0)
				{	
					
					printf("Sending the same sequence inside receive from %d again\n", seq);
					fseek(fptr, size_check, SEEK_SET);
				}
				else if(seq_check > 0)
				{
					seq_dec = htonl(seq_get);
					printf("The received acknowledgment is %d\n", seq_dec);
					if(seq_dec != seq)
					{
						printf("The same sequence has to be sent again%d\n", seq_dec);
						fseek(fptr, conff_size, SEEK_SET);
					}
					else if(seq_dec == seq)
					{
						seq++;
						conff_size = conff_size + obt_bytes;
						printf("The size is : %ld\n", conff_Size);
					}
	
				}
			  free(buf_pkt);
			}
			}
		}
		fclose(fptr);	
	}
		
	else
	{		
		printf("File is not present\n");
		printf("Enter an appropiate name of the file\n");
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
