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

typedef struct
{
	uint8_t packet_descp[BUFSIZE];
	uint32_t packet_len;
	int byte;
	uint32_t packet_index;
	uint32_t packet_ack;
}Packet_Details;

uint32_t seq = 1, ack_seq = 0, seq_recv= 1, seq_sent = 0, seq_dec = 1;
int cond = 1;

struct timeval time_vals, time_val1, time_val2, time_done;
/* cmd_get
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

void get_file(int socket_id, uint8_t* name_file, struct sockaddr_in rem)
{
	int info_send = 0, bytes_read = 0, values = 0, val2 = 0, rem_length = sizeof(rem);;
	FILE *fptr;
	uint8_t msg_conf[BUFSIZE] = " ", nbuf[BUFSIZE], cont[4] = {'A', 'B', '5', '9'};
	info_send = recvfrom(socket_id, msg_conf, BUFSIZE, 0, (struct sockaddr*)&rem, &(rem_length));
	Packet_Details *buf_pkt = malloc(sizeof(Packet_Details));
	Packet_Details *pkt_ack = malloc(sizeof(Packet_Details));
	bzero(buf_pkt->packet_descp, BUFSIZE);
	
	if(strcmp(msg_conf, "File is present"))
	{
		printf("The file has been found\n");
		while(cond)
		{
			info_send = recvfrom(socket_id, (Packet_Details*) buf_pkt, sizeof(*buf_pkt), 0, (struct sockaddr*)&rem, &(rem_length)); 
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
				info_send = sendto(socket_id, (Packet_Details*)pkt_ack, sizeof(*pkt_ack), 0, (struct sockaddr*)&rem, sizeof(rem));
			}
			else if(seq_sent == seq_dec)
			{
				fptr = fopen(name_file, "ab");
				fwrite(buf_pkt->packet_descp, buf_pkt->byte, 1, fptr);
				bzero(pkt_ack->packet_descp, BUFSIZE);
				fclose(fptr);
				pkt_ack->packet_ack = seq_dec;
				info_send = sendto(socket_id, (Packet_Details*)pkt_ack, sizeof(*pkt_ack), 0, (struct sockaddr*)&rem, sizeof(rem));
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


void put_file(int socket_id, uint8_t* name_file, struct sockaddr_in rem)
{
	uint8_t msg_conf[BUFSIZE] = " ", nbuf[BUFSIZE], cont[4] = {'A' , 'B', '5', '9'};
	int info_send = 0, bytes_read = 0, values = 0, val2 = 0;
	Packet_Details *buf_pkt = malloc(sizeof(Packet_Details));
	Packet_Details *pkt_ack = malloc(sizeof(Packet_Details));
	ack_time_cond(socket_id);
	FILE *fptr;
	int rem_len = sizeof(rem);
	fptr = fopen(name_file, "rb");
	if(fptr != NULL)
	{
		strcpy(msg_conf, "File is present");
		info_send = sendto(socket_id, msg_conf, sizeof(msg_conf), 0,(struct sockaddr*)&rem, rem_len);
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
			info_send = sendto(socket_id, (Packet_Details*) buf_pkt, (sizeof(*buf_pkt)), 0, (struct sockaddr*)&rem, rem_len);
			info_send = recvfrom(socket_id, (Packet_Details*) pkt_ack, (sizeof(*pkt_ack)), 0, (struct sockaddr*)&rem, &rem_len);
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
	else 
	{
		if(fptr == NULL)
		{	
			strcpy(msg_conf, "File is not found");
			info_send = sendto(socket_id, msg_conf, sizeof(msg_conf), 0, (struct sockaddr*)&rem, rem_len);
		}
	}
	
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
		//printf("The name of the command is: %s\n", name_cmd);
		//printf("The file name is %s\n", fname);
		if(strcmp("get", name_cmd) == 0)
		{
			printf("\nTo obtain the name of the file from the server %s\n", fname);
			get_file(sockfd, fname1, serveraddr);
			printf("\nThe file get is done\n");
		}

		else if(strcmp("put", name_cmd) == 0)
		{
			printf("\nTo put the file by the client %s\n", fname);
			put_file(sockfd, fname1, serveraddr);
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
		    serverlen = sizeof(serveraddr);
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
