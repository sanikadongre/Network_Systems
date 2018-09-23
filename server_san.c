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
#include <dirent.h>


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
  uint8_t msg_conf[] = " ";
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
		}

		else if(strcmp("put", cmd) == 0)
		{

		 }
		 else if(strcmp("ls", cmd) == 0)
		{
			DIR *direct_ls;
			struct dirent *dir_ls;
			direct_ls = opendir("/home/pi/Network_Systems_deleteexithashworking/pf_server");
			if(direct_ls == NULL)
			{
				strcpy(msg_conf, "The drirectory can't be read");
				bytestot = sendto(sockfd, msg_conf, sizeof(msg_conf), 0, (struct sockaddr*)&clientaddr, clientlen);
				perror("Error");
			}
			else
			{
				bzero(fname, BUFSIZE);
				while((dir_ls == readdir(direct_ls)) != NULL)
				{
					strcat(fname, dir_ls->d_name);
					strcat(fname, "\n");
				}
				bytestot = sendto(sockfd, fname, sizeof(fname), 0, (struct sockaddr*)&clientaddr, clientlen);
			}
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

  }
}
