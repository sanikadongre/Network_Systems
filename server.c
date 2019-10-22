
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
//#define ip_addr ("192.168.7.1")


typedef struct{
	int pckt_index;						//struct for packet details
	int pckt_ack;														
	int packet_length;
	uint8_t data_descp[BUFSIZE];												
}Packet_Details;

void encode(uint8_t *temp_buf, int size_len, uint8_t key)
{
	for(int i=0; i<size_len; i++)
	{
		temp_buf[i] ^= key;
	}
}


/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main (int argc, char * argv[] )
{
			
	int sockfd, portno, file_del, clientlen, optval, n, bytestot =0, read_bytes =0, packet_id;                           
	struct sockaddr_in sin, clientaddr;   
        struct hostent* server_hp;  	
	int nbytes;                        		
	char buffer[BUFSIZE], buf[BUFSIZE], hash_buf[BUFSIZE], recv_buf[BUFSIZE], msg_val[] = " ";             	
	char *cname;													
	char *filename;												
	FILE *fptr;														
	uint8_t cmd_out_exit, val[BUFSIZE], cmd[70], fname[70], check = 1, key = 10;
  	bzero(cmd, sizeof(cmd));
  	bzero(fname, sizeof(fname));
  	bzero(val, sizeof(val));
  	uint8_t msg_conf[] = " ";
	struct timeval timeout;								
	Packet_Details* a = malloc(sizeof(Packet_Details));
	Packet_Details* s = malloc(sizeof(Packet_Details));
	if (argc < 3)
	{
		printf ("USAGE: <host_ip> <port>\n");
		exit(1);
	}
	bzero(&sin,sizeof(sin));
       // memset((char*)&sin,0,sizeof(sin));                   
	sin.sin_family = AF_INET;                   
	sin.sin_port = htons(atoi(argv[2]));
	//ippointer = gethostbyname(ip_addr);
        //memcpy(&sin.sin_addr,ippointer->h_addr,ippointer->h_length);        
	//sin.sin_addr.s_addr = INADDR_ANY;    
	sin.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address

	server_hp = gethostbyname(argv[1]);					 
	if(server_hp < 0){
			perror("Host Unknown");
	}

	bcopy((char*)server_hp->h_addr, (char*)&sin.sin_addr, server_hp->h_length);
      
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("unable to create socket");
	}
	if (bind(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf("unable to bind socket\n");
	}

	clientlen = sizeof(struct sockaddr_in);
	while(1)
	{
	
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

		bzero(buffer, sizeof(buffer));

		/* Recieving the command from the client */
		optval = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientaddr, &clientlen);

		cname = strdup(buffer);
		strtok(cname," ");   //string splitting
		filename = strtok(NULL, " ");

		printf("Command Recieved: %s \n", cname);
		printf("File name:%s\n", filename );

		
		if(strcmp(cname, "get") == 0)
		{
			/* Setting Timeout */
			timeout.tv_sec = 0;
			timeout.tv_usec = 300000;
			setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

			s->pckt_index = 1;	// Initializing the server packet index with 1
			a->pckt_ack = 0;		// Initializing the packet acknowledgement with 0
			bzero(s->data_descp, sizeof(s->data_descp));
			FILE *fptr;
			fptr = fopen(filename, "rb");
			if(fptr != NULL)
			{
				strcpy(msg_val, "Filefound");
				printf("File is present\n");
				optval = sendto(sockfd, msg_val, sizeof(msg_val), 0, (struct sockaddr *)&clientaddr, clientlen);
				
			}
			else if(fptr == NULL)
			{
				
				strcpy(msg_val, "Fileerror");
				printf("File cant be found\n");
				optval = sendto(sockfd, msg_val, sizeof(msg_val), 0, (struct sockaddr *)&clientaddr, clientlen);
				continue;
			}
			
			while(1)
			   {
				
				read_bytes = fread(s->data_descp, 1, BUFSIZE , fptr);
				s->packet_length = read_bytes;
				printf("Read length%d\n", read_bytes);
				encode(s->data_descp, s->packet_length, key);//Encrypting data to be sent to the client
				optval = sendto(sockfd, (Packet_Details*) s, sizeof(Packet_Details), 0, (struct sockaddr *)&clientaddr, clientlen);
				printf("Packet Size send to client: %d\n", optval);
				bzero(s->data_descp, sizeof(s->data_descp));
				optval = recvfrom(sockfd, (Packet_Details*) a, sizeof(Packet_Details), 0, (struct sockaddr *)&clientaddr, &clientlen);
				if(optval<0)
				{
					printf("Timeout occurs\n");
					fseek(fptr, (-1)*(s->packet_length), SEEK_CUR);
					continue;
				}
				printf("The size of the packets and acknowledgments are: %d \n",optval, a->pckt_ack);

				if(a->pckt_ack != s->pckt_index)
				{
					fseek(fptr, (-1)*(s->packet_length), SEEK_CUR); //Timeout has occurred
					
				}
				else if(a->pckt_ack == s->pckt_index)
				{
					s->pckt_index++;  //Sending next packet
				}
				if(s->packet_length != BUFSIZE)
				{
					break;
				}
			}
			memset(a, 0, sizeof(Packet_Details)); //Free the memory
			memset(s, 0, sizeof(Packet_Details));
		}
		else if(strcmp(cname,"put") == 0)
		{
			memset(a, 0, sizeof(Packet_Details));
			memset(s, 0, sizeof(Packet_Details));

			packet_id=1;	// Initializing the expected packet with 1
			s->pckt_ack = 1;//Initializing packet structure acknowledgement
			bzero(buffer, sizeof(buffer));
			optval = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &clientaddr, &clientlen);
			if(!strcmp(buffer, "Fileerror"))
			{
				continue;
			}
			FILE *fptr;
			fptr = fopen(filename,"ab");
			if(fptr == NULL)
			{
				perror("Error opening file \n");
			}

			while(1)
			{
				bzero(a->data_descp, sizeof(a->data_descp));
				optval = recvfrom(sockfd, (Packet_Details*) a, sizeof(Packet_Details), 0, (struct sockaddr *) &clientaddr, &clientlen);
				printf("Packet Size from the client: %d \n",optval);
				encode(a->data_descp, a->packet_length, key);//encoding data sent by client
				if(a->pckt_index != packet_id)
				{
					s->pckt_ack=a->pckt_index;
					optval = sendto(sockfd, (Packet_Details*)s, sizeof(Packet_Details), 0, (struct sockaddr *) &clientaddr, clientlen);
					
				}
				else if(a->pckt_index == packet_id)
				{
					fwrite(a->data_descp, a->packet_length, 1, fptr);
					bzero(a->data_descp, sizeof(a->data_descp));
					s->pckt_ack=packet_id;  
					optval = sendto(sockfd, (Packet_Details*) s, sizeof(Packet_Details), 0, (struct sockaddr *) &clientaddr, clientlen);
					printf("Packet Size being sent to  the client: %d and ACK from server: %d\n",optval, s->pckt_ack);
					packet_id++;
				}
				if(a->packet_length != BUFSIZE)
				{
					break;
				}
			}
			fclose(fptr); //close fptr
			memset(a, 0, sizeof(Packet_Details)); //free memory
			memset(s, 0, sizeof(Packet_Details));
		}
		else if(strcmp(cname,"delete") == 0)
		{
			FILE *f;
			bytestot = recvfrom(sockfd, filename, (BUFSIZE), 0, (struct sockaddr*)&clientaddr, &(clientlen));
			printf("The file name is %s\n", filename);
			f = fopen(filename,"r");			
			if(f != NULL)
			{
				file_del = remove(filename); //command for deleting file
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
		else if(strcmp(cname,"exit") == 0)
		{
			bzero(buf, sizeof(buf));
			strcpy(buf, "Exit by the server");
			printf(" Exit buffer is: %s\n", buf);
			bytestot = sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)&clientaddr, clientlen);
			exit(0);
		}
		else
		{
			printf("The command in not correct\n");
			printf("Enter a valid command\n");
		}

		bzero(buffer, sizeof(buffer));
		bzero(fname, sizeof(fname));
		bzero(val,sizeof(val));

		}

}
