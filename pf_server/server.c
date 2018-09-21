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

uint32_t struct_len = sizeof(Packet_Details), seq = 1, ack_seq = 0, seq_sent = 0, seq_dec = 0;
int cond = 1;

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

void ack_time_cond(int sock)
{
	struct timeval time_vals, time_val1, time_val2, time_done;
	Packet_Details *buf_pkt = malloc(sizeof(Packet_Details));
	Packet_Details *pkt_ack = malloc(sizeof(Packet_Details));
	time_vals.tv_sec = 0;
	time_vals.tv_usec = 300000;
	if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &time_vals, sizeof(time_vals)) < 0)
	{
		perror("Error\n");
	}
	buf_pkt->packet_index = seq;
	pkt_ack->packet_ack = ack_seq;
}

void get_file(int socket_id, uint8_t* name_file, struct sockaddr_in remote_add, unsigned int rem_len)
{
	uint8_t msg_conf[BUFSIZE] = " ", nbuf[BUFSIZE], cont[4] = {'A' , 'B', '5', '9'};
	int info_send = 0, bytes_read = 0, values = 0, val2 = 0;
	Packet_Details *buf_pkt = malloc(sizeof(Packet_Details));
	Packet_Details *pkt_ack = malloc(sizeof(Packet_Details));
	ack_time_cond(socket_id);
	FILE *fptr;
	fptr = fopen(name_file, "rb");
	if(fptr != NULL)
	{
		strcpy(msg_conf, "File is present");
		info_send = sendto(socket_id, msg_conf, sizeof(msg_conf), 0,(struct sockaddr*)&remote_add, rem_len);
		bzero(buf_pkt->packet_descp, BUFSIZE);
		while(cond)
		{
			buf_pkt->byte = fread(buf_pkt->packet_descp,1,BUFSIZE,fptr);
			bytes_read = buf_pkt->byte;

			for(values = 0; values < bytes_read; values++)
			{
				buf_pkt->packet_descp[values] = nbuf[values] - cont[val2];
				val2++;
				if(val2 == 3)
				{
					val2 = 0;
				}
			}
			printf("The sequence is %d\n", buf_pkt->packet_index);
			info_send = sendto(socket_id, (Packet_Details*) buf_pkt, (sizeof(*buf_pkt)), 0, (struct sockaddr*)&remote_add, rem_len);
			info_send = recvfrom(socket_id, (Packet_Details*) pkt_ack, (sizeof(*pkt_ack)), 0, (struct sockaddr*)&remote_add, &rem_len);
			if(info_send > 0)
			{
				printf("The size of the packet is %d\n", info_send);
			}
			if(info_send < 0)
			{
				printf("Sending the same sequence again %d\n", buf_pkt->packet_index);
				fseek(fptr,(-1)*bytes_read, SEEK_CUR);
			
			}
			printf("The acknowledgment of packets is %d\n", pkt_ack->packet_ack);
			seq_sent = buf_pkt->packet_index;
			seq_dec = pkt_ack->packet_ack;

			if(seq_sent != seq_dec)
			{
				printf("Sending the same sequence again %d\n", seq_sent);
				fseek(fptr, (-1)*bytes_read, SEEK_CUR);
			}
		
			if(bytes_read != BUFSIZE)
			{
				break;
			}
		
			if(seq_sent == seq_dec)
			{
				seq_sent++;
			}
		}
		free(buf_pkt);
		free(pkt_ack);
	}
	else if(fptr == NULL)
	{
		strcpy(msg_conf, "File is not found");
		info_send = sendto(socket_id, msg_conf, sizeof(msg_conf), 0, (struct sockaddr*)&remote_add, rem_len);
	}
	
}

void put_file(int socket_id, uint8_t* name_file, struct sockaddr_in remote_add, unsigned int rem_len)
{	
	int info_send = 0, bytes_read = 0, values = 0, val2 = 0;
	FILE *fptr;
	uint8_t msg_conf[BUFSIZE] = " ", nbuf[BUFSIZE], cont[4] = {'A', 'B', '5', '9'};
	info_send = recvfrom(socket_id, msg_conf, BUFSIZE, 0, (struct sockaddr*)&remote_add, &(rem_len));
	Packet_Details *buf_pkt = malloc(sizeof(Packet_Details));
	Packet_Details *pkt_ack = malloc(sizeof(Packet_Details));
	bzero(buf_pkt->packet_descp, BUFSIZE);
	
	if(strcmp(msg_conf, "File is present"))
	{
		printf("The file has been found\n");
		while(cond)
		{
			info_send = recvfrom(socket_id, (Packet_Details*) buf_pkt, sizeof(*buf_pkt), 0, (struct sockaddr*)&remote_add, &(rem_len)); 
			bytes_read = buf_pkt->byte;
			for(values = 0; values < bytes_read; values++)
			{
				buf_pkt->packet_descp[values] = nbuf[values] + cont[val2];
				val2++;
				if(val2 == 3)
				{
					val2 = 0;
				}
			}
			seq_sent = buf_pkt->packet_index;
			if(seq_sent != seq_dec)
			{
				printf("Sending the data again\n");
				info_send = sendto(socket_id, (Packet_Details*)pkt_ack, sizeof(*pkt_ack), 0, (struct sockaddr*)&remote_add, sizeof(remote_add));
			}
			else if(seq_sent == seq_dec)
			{
				fptr = fopen(name_file, "ab");
				fwrite(buf_pkt->packet_descp, buf_pkt->byte, 1, fptr);
				bzero(pkt_ack->packet_descp, BUFSIZE);
				fclose(fptr);
				pkt_ack->packet_ack = seq_dec;
				info_send = sendto(socket_id, (Packet_Details*)pkt_ack, sizeof(*pkt_ack), 0, (struct sockaddr*)&remote_add, sizeof(remote_add));
				seq_dec++;
			}
			if(bytes_read != BUFSIZE)
			{
				break;
			}
		}
		free(buf_pkt);
		free(pkt_ack);
	}
	else 
	{
		
		if(strcmp(msg_conf, "File is not found"))
		{
			printf("File is absent\n");
		}
		
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
  struct timeval time_vals, time_val1, time_val2, time_done;
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
		if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&time_val1,sizeof(time_val1)) < 0)
		{
			perror("Error\n");

                } 
		if(strcmp("get", cmd) == 0)
		{
			get_file(sockfd, fname, clientaddr, clientlen);
		}

		else if(strcmp("put", cmd) == 0)
		{
			put_file(sockfd, fname, clientaddr, clientlen);

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
