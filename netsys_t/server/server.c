
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


typedef struct{
	int pckt_index;						
	int pckt_ack;											
	char data_buff[BUFSIZE];				
	int len_data;											
}struct_pckt;

void (uint8_t* temp_buf, int size_len, uint8_t key)
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
			
	int sockfd, portno, file_del, clientlen, optval, n, bytestot =0, read_length =0;                           
	struct sockaddr_in sin, clientaddr;     	
	int nbytes;                        		
	char buffer[BUFSIZE], buf[BUFSIZE], hash_buf[BUFSIZE], recv_buf[BUFSIZE], msg_val[] = " ";             	
	char *cname;													
	char *filename;												
	struct_pckt* c_pckt = malloc(sizeof(struct_pckt));
	struct_pckt* s_pckt = malloc(sizeof(struct_pckt));
	FILE *fptr;														
	uint8_t cmd_out_exit, val[BUFSIZE], cmd[70], fname[70], check = 1, key = 10;
  	bzero(cmd, sizeof(cmd));
  	bzero(fname, sizeof(fname));
  	bzero(val, sizeof(val));
  	uint8_t msg_conf[] = " ";
	struct timeval timeout;								
	


	if (argc != 2)
	{
		printf ("USAGE:  <port>\n");
		exit(1);
	}

	/******************
	  This code populates the sockaddr_in struct with
	  the information about our socket
	 ******************/
	bzero(&sin,sizeof(sin));                    //zero the struct
	sin.sin_family = AF_INET;                   //address family
	sin.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order
	sin.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("unable to create socket");
	}
	if (bind(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf("unable to bind socket\n");
	}

	clientlen = sizeof(struct sockaddr_in);

	
	while(1){
		/* Initializing timeout struct */
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

		bzero(buffer, sizeof(buffer));

		/* Recieving the command from the client */
		optval = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientaddr, &clientlen);

		cname = strdup(buffer);
		strtok(cname," ");
		filename = strtok(NULL, " ");

		printf("Command Recieved: %s \n", cname);
		printf("File name:%s\n", filename );

		
		if(!strcmp(cname, "get"))
		{
			/* Setting Timeout */
			timeout.tv_sec = 0;
			timeout.tv_usec = 300000;
			setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

			s_pckt->pckt_index = 1;	// Initializing the server packet index with 1
			c_pckt->pckt_ack = 0;		// Initializing the packet acknowledgement with 0

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
				bzero(s_pckt->data_buff, sizeof(s_pckt->data_buff));
				read_length = fread(s_pckt->data_buff, 1, BUFSIZE , fptr);
				s_pckt->len_data = read_length;
				printf("Read length%d\n", read_length );
				encode(s_pckt->data_buff, s_pckt->len_data, key);//Encrypting data to be sent to the client
				optval = sendto(sockfd, (struct_pckt*) s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&clientaddr, clientlen);
				printf("Packet Size send to client: %d\n", optval);
				bzero(s_pckt->data_buff, sizeof(s_pckt->data_buff));
				optval = recvfrom(sockfd, (struct_pckt*) c_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&clientaddr, &clientlen);
				if(optval<0)
				{
					printf("Timeout occurs\n");
					fseek(fptr, (-1)*read_length, SEEK_CUR);
					continue;
				}
				printf("Packet Size from client: %d and client ack_index: %d \n",optval, c_pckt->pckt_ack);

				if(c_pckt->pckt_ack != s_pckt->pckt_index)
				{
					fseek(fptr, (-1)*read_length, SEEK_CUR);
					
				}
				else if(c_pckt->pckt_ack == s_pckt->pckt_index)
				{
					s_pckt->pckt_index++;
				}
				if(read_length != BUFSIZE)
				{
					break;
				}
			}
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));
		}
		else if(!strcmp(cname,"put"))
		{
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));

			int exp_index=1;	// Initializing the expected packet with 1
			s_pckt->pckt_ack = 1;//Initializing packet structure acknowledgement
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
				bzero(c_pckt->data_buff, sizeof(c_pckt->data_buff));
				optval = recvfrom(sockfd, (struct_pckt*) c_pckt, sizeof(struct_pckt), 0, (struct sockaddr *) &clientaddr, &clientlen);
				printf("Packet Size from the client: %d \n",optval);
				encode(c_pckt->data_buff, c_pckt->len_data, key);//decrypting data sent by the client
				if(c_pckt->pckt_index != exp_index)
				{
					s_pckt->pckt_ack=c_pckt->pckt_index;
					optval = sendto(sockfd, (struct_pckt*)s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *) &clientaddr, clientlen);
					
				}
				else if(c_pckt->pckt_index == exp_index)
				{
					fwrite(c_pckt->data_buff, c_pckt->len_data, 1, fptr);
					bzero(c_pckt->data_buff, sizeof(c_pckt->data_buff));
					s_pckt->pckt_ack=exp_index;
					optval = sendto(sockfd, (struct_pckt*) s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *) &clientaddr, clientlen);
					printf("Packet Size being sent to client: %d and ACK from server: %d\n",optval, s_pckt->pckt_ack);
					exp_index++;
				}
				if(c_pckt->len_data != BUFSIZE)
				{
					break;
				}
			}
			fclose(fptr);
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));
		}
		else if(!strcmp(cname,"delete"))
		{
			FILE *f;
			bytestot = recvfrom(sockfd, filename, (BUFSIZE), 0, (struct sockaddr*)&clientaddr, &(clientlen));
			printf("The file name is %s\n", filename);
			f = fopen(filename,"r");			
			if(f != NULL)
			{
				file_del = remove(filename);
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
		else if(!strcmp(cname,"ls"))
		{
			DIR *direct_ls;
			struct dirent *dir_ls;
			direct_ls = opendir(".");
			if(direct_ls == NULL)
			{
				strcpy(msg_conf, "The drirectory can't be read");
				bytestot = sendto(sockfd, msg_conf, sizeof(msg_conf), 0, (struct sockaddr*)&clientaddr, clientlen);
				perror("Error");
			}
			else
			{
				bzero(fname, BUFSIZE);
				while((dir_ls = readdir(direct_ls)) != NULL)
				{
					strcat(fname, dir_ls->d_name);
					strcat(fname, "\n");
				}
				bytestot = sendto(sockfd, fname, sizeof(fname), 0, (struct sockaddr*)&clientaddr, clientlen);
			}
		}

		else if(!strcmp(cname,"exit"))
		{
			bzero(buf, sizeof(buf));
			strcat(filename, "Exit");
			printf(" Exit buffer: %s\n", buf);
			bytestot = sendto(sockfd, buf, BUFSIZE, 0, (struct sockaddr*)&clientaddr, clientlen);
			exit(0);
		}
		else if(strcmp("md5", cname) == 0)
		{
			strcpy(hash_buf, "md5sum");
			printf("To get the hash value of the file: %s\n", filename);
			strncat(hash_buf,filename,strlen(filename));
			printf("**************************\n");
			system(hash_buf);
			printf("***************************\n");
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
