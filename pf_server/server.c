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

typedef struct
{
	uint32_t packet_index;
	uint8_t packet_descp[BUFSIZE];
	uint32_t packet_len;
	int byte;
	uint32_t packet_ack;
}Packet_Details;

int cond = 1;

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

void data_encrypt(uint8_t *temp_buf, int len_d, uint8_t key[], uint8_t key1[])
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

void data_decrypt(uint8_t* temp_buf, int len_d, uint8_t key[], uint8_t key1[])
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

int main(int argc, char **argv)
 {
  int sockfd, portno, clientlen, optval, n, client_socket, check = 1, bytestot = 0, file_del, exit_recv, bytestot1 = 0; /* socket */
  struct sockaddr_in serveraddr, clientaddr; /* server's addr */
  struct hostent *hostp; /* client host info */
  FILE *fptr;
  char buf[BUFSIZE], hash_buf[BUFSIZE], recv_buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  uint8_t cmd_out_exit, val[BUFSIZE] = "The server is exiting", cmd[70], fname[70], key[4] = {'A', 'B', '5', '9'}, key1[6] = {'K', 'L', '8', '6'};
  bzero(cmd, sizeof(cmd));
  bzero(fname, sizeof(fname));
  bzero(val, sizeof(val));
  Packet_Details* buf_pkt = malloc(sizeof(Packet_Details));
  Packet_Details* pkt_ack = malloc(sizeof(Packet_Details));
  struct timeval timeout;
  int read_length, info_send;
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
	  	timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0)
		{
			perror("Error\n");

                } 
		if(strcmp("get", cmd) == 0)
		{
			timeout.tv_sec = 0;
			timeout.tv_usec = 300000;
			if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
			{
				perror("Error\n");
			}
			buf_pkt->packet_index = 1;
			pkt_ack->packet_ack = 0;
			FILE *fptr;
			fptr = fopen(fname, "rb");
			if(fptr == NULL)
			{
				uint8_t msg_conf[] = "Error";
				bytestot = sendto(sockfd, msg_conf, sizeof(msg_conf), 0, (struct sockaddr*)&clientaddr, clientlen);
				continue;
			}
			else{
			       uint8_t msg_conf[] = "Success";
			       bytestot = sendto(sockfd, msg_conf, sizeof(msg_conf), 0, (struct sockaddr*)&clientaddr, clientlen);
			}
			do{
				bzero(buf_pkt->packet_descp, sizeof(buf_pkt->packet_descp));
				read_length = fread(buf_pkt->packet_descp,1,BUFSIZE, fptr);
				buf_pkt->byte = read_length;
				data_encrypt(buf_pkt->packet_descp, buf_pkt->byte, key, key1);
				bytestot= sendto(sockfd, (Packet_Details*)buf_pkt, sizeof(Packet_Details), 0, (struct sockaddr*)&clientaddr, clientlen);
				bzero(buf_pkt->packet_descp, sizeof(buf_pkt->packet_descp));
				bytestot = recvfrom(sockfd, (Packet_Details*)pkt_ack, sizeof(Packet_Details), 0, (struct sockaddr*)&clientaddr, &clientlen);
				if(bytestot < 0)
				{
					fseek(fptr, (-1)*read_length, SEEK_CUR);
					continue;
				}
				printf("Packet size from client and ack %d and %d\n", bytestot, pkt_ack->packet_ack);
				if(pkt_ack->packet_ack == buf_pkt->packet_index)
				{
					buf_pkt->packet_index++;
				}
				else
				{
					fseek(fptr, (-1)*read_length, SEEK_CUR);
				}
				if(read_length != BUFSIZE)
				{
					break;
				}
			}while(1);
			memset(pkt_ack, 0, (sizeof(Packet_Details)));
			memset(buf_pkt, 0, (sizeof(Packet_Details)));
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
