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
#include <server.h>
#include <stdint.h>
#include <sys/time.h>


#define BUFSIZE 1024
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

int main(int argc, char **argv)
 {
  int sockfd, portno, clientlen, optval, n, client_socket, bytestot = 0, file_del, exit_recv, bytestot1 = 0; /* socket */
  struct sockaddr_in serveraddr, clientaddr; /* server's addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  uint8_t* fname;
  uint8_t* name_cmd;
  uint8_t cmd_out_exit, val[BUFSIZE] = "The server is exiting", cmd[70], fname1[70], check = 1;
  bzero(cmd, sizeof(cmd));
  bzero(fname1, sizeof(fname1));
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
    n = recvfrom(sockfd, buf, BUFSIZE, 0,
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
    printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf);
	  time_val1.tv_sec = 0;
		time_val1.tv_usec = 0;
		(setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO,&time_val1,sizeof(time_val1)) < 0); 
	
		bytestot = recvfrom(udp_sock, cmd, strlen(cmd), 0, (struct sockaddr*)&remote_opt, &remote_length);
		bytestot1 = recvfrom(udp_sock, fname1, strlen(fname1), 0, (struct sockaddr*)&remote_opt, &remote_length);
		printf("The command received from the client is : %s\n", cmd);
		name_cmd = strdup(cmd);
		fname= strdup(fname1);
		strtok(name_cmd, " ");
		printf("The name of the command is: %s\n", name_cmd);
		fname = strtok(fname, " ");
		printf("The file name is %s\n", fname);
		if(strcmp("get", name_cmd) == 0)
		{
		}

		else if(strcmp("put", name_cmd) == 0)
		{

		 }
		 else if(strcmp("ls", name_cmd) == 0)
		{
			
		}
					
		else if(strcmp("delete", name_cmd) == 0)
		{	
			FILE *f;
			bytestot = recvfrom(udp_sock, recv_buf, strlen(recv_buf), 0, (struct sockaddr*)&remote_opt, &(remote_length));
			bytestot1 = recvfrom(udp_sock, recv_buf, strlen(recv_buf), 0, (struct sockaddr*)&remote_opt, &(remote_length));
			f = fopen(recv_buf,"r");			
			if(f != NULL)
			{
				file_del = remove(recv_buf);
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
		else if(strcmp("md5sum", name_cmd) == 0)
		{
			strcpy(hash_buf, "md5sum");
			printf("To get the hash value of the file: %s\n", fname);
			strncat(hash_buf,fname,strlen(fname));
			printf("**************************\n");
			system(hash_buf);
			printf("***************************\n");
		}
		else if(strcmp("exit", name_cmd) == 0)
		{
	 		strcat(recv_buf, "Exit");
			printf("The recv_buf message is %s\n", recv_buf);
			//bzero(recv_buf, sizeof(recv_buf));
			exit_recv = sendto(udp_sock, recv_buf, BUFSIZE, 0, (struct sockaddr*)&remote_opt, sizeof(remote_opt));
			printf("The exit command sent is %d\n", exit_recv);
			check = 0;

        	 }
	 	else
		{
			printf("The entered command isn't appropriate\n");

	  	}
		bzero(cmd,sizeof(cmd));
		bzero(fname1, sizeof(fname1));
		bzero(val,sizeof(val));

    
    /* 
     * sendto: echo the input back to the client 
     */
	   n = sendto(sockfd, buf, strlen(buf), 0, 
	       (struct sockaddr *) &clientaddr, clientlen);
    n = sendto(sockfd, cmd, strlen(cmd), 0, 
	       (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0) 
      error("ERROR in sendto");
  }
}
