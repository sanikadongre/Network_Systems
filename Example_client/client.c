/* Headers section */
#include <sys/time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <memory.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

/* Global variables */
#define BUFSIZE 100

/* Structure for the packet */
typedef struct 
{
        uint8_t packet_descp[BUFSIZE];
	uint32_t packet_len;
	uint32_t packet_index;
	
}Packet_Details;

struct timeval time_vals, time_val1, time_val2, time_done;
void condition(int sock, uint8_t* name_of_file, struct sockaddr_in remote_addr)
{	
	Packet_Details *buf_pkt = malloc(sizeof(Packet_Details));     
	int info_send = 0, encrypted_msg = 0;
	printf("Acknowledgement sent %d\n",buf_pkt->packet_index);
	encrypted_msg = ntohl(buf_pkt->packet_index);
	info_send = sendto(sock, &encrypted_msg, sizeof(encrypted_msg), 0, (struct sockaddr*)&remote_addr, sizeof(remote_addr));
}

void file_trans(int udp_sock, uint8_t* nm_file, struct sockaddr_in rem_addr)
{
	int bytes_received =0, bytes_file_received = 0, values=0, val2=0;
	ssize_t encrypted_size, conff_size = 0;
	uint8_t temp_file[BUFSIZE],key[4] = {'A', 'B', '5', '9'}, nbuf[BUFSIZE];
	FILE *fptr;
	fptr = fopen(temp_file,"w");
	Packet_Details *buf_pkt = malloc(sizeof(Packet_Details));     
		if(fptr != NULL)
		{
			printf("File open successful\n");
			while(conff_size < encrypted_size)
			{
				if(NULL != buf_pkt)
				{
					bytes_file_received = recvfrom(udp_sock, buf_pkt, sizeof(*buf_pkt), 0, NULL,NULL);
				        if(bytes_received == buf_pkt->packet_index)

					{
						for(values=0; values<buf_pkt->packet_len; values++)
						{
							buf_pkt->packet_descp[values] = nbuf[values] - key[val2];
							val2++;
							if(val2 == 3)
							{
								val2 = 0;
							}

						   }
						
						fwrite(buf_pkt->packet_descp,1,buf_pkt->packet_len,fptr);
						printf("\nsize_check : %ld, encryptes_size : %ld, received bytes : %d\n",conff_size,encrypted_size,bytes_file_received);
						condition(udp_sock,nm_file,rem_addr);
						bytes_received++;
						conff_size = conff_size + sizeof(buf_pkt->packet_descp);
					}

					if(bytes_received != buf_pkt->packet_index)
					{
					 	condition(udp_sock,nm_file,rem_addr);
					}
					memset(buf_pkt, 0, sizeof(Packet_Details));
				}
			}
			fclose(fptr);
		}				
}
/* Definition of 'client_get_file' function */
void get_file(int socket_id, char *name_file, struct sockaddr_in remote)
{

	uint8_t msg_conf[BUFSIZE]="", conf_recvd, sizef[BUFSIZE], temp_file[BUFSIZE]; 
	bzero(msg_conf,sizeof(msg_conf));
	bzero(sizef, sizeof(sizef));
	int info_send = 0, file_encrypted = 0, bytes_file_recvd = 0,  get_msg = 0, length, encrypted_msg = 0;
	ssize_t size_file = 0;
	long int limit =0;
	conf_recvd = recvfrom(socket_id, msg_conf, sizeof(msg_conf), 0, NULL,NULL);
	printf("The file is received confirmation %s\n", msg_conf);

	
	if(strcmp(msg_conf,"File present") == 0)
	{
		printf("\n File found\n");
		strncpy(sizef, "The size of file", strlen("The size of file"));
		info_send = sendto(socket_id, sizef, strlen(sizef), 0, (struct sockaddr*)&remote, sizeof(remote));
		bytes_file_recvd = recvfrom(socket_id, &size_file, sizeof(size_file), 0, NULL,NULL);
		file_encrypted = ntohl(size_file);
		printf("File  of size received is : %ld\n",file_encrypted);
		strcpy(temp_file,name_file);
		file_trans(socket_id, name_file, remote);
	}
	else
	{
		printf(" File is not present\n");
		printf("Enter an appropriate name of the file\n");
	}
	
}

/* Definition of 'client_put_file' function */
void put_file(int socket_id, char *name_file, struct sockaddr_in remote)
{
	int msg_conf = 0, obt_bytes = 0, info_send = 0, seq = 0, seq_get = 0, seq_dec = 0, seq_htonl =0, seq_check= 0, fs, values = 0, val2 = 0;
	ssize_t size_file,file_encrypted,conff_size = 0;
	uint8_t present_file[BUFSIZE],key[4] = {'A', 'B', '5', '9'}, nbuf[BUFSIZE];
	bzero(present_file,sizeof(present_file));
	/* Creating a file pointer to the file to be sent to server */
	time_vals.tv_sec = 0;
	time_vals.tv_usec = 100000;
	Packet_Details *buf_pkt = malloc(sizeof(Packet_Details));     
	FILE *fptr;
	fptr = fopen(name_file,"rb");
		if(fptr == NULL)
		{
		
			strcpy(present_file,"File is not present\n");
			msg_conf = sendto(socket_id, present_file, strlen(present_file), 0, (struct sockaddr*)&remote,  sizeof(remote));
			printf("File exist confirmation message : %s\n",present_file);
			printf("Please enter an appropriate file name\n");
		}
		else
		{
			strcpy(present_file,"File is present");
			msg_conf = sendto(socket_id, present_file, strlen(present_file), 0, (struct sockaddr*)&remote,  sizeof(remote));
			printf("The message obtained is: %s\n",present_file);
			fseek(fptr, 0, SEEK_END);
   			size_file = ftell(fptr);
  			file_encrypted = htonl(size_file);
			rewind(fptr);
			fs = sendto(socket_id, &file_encrypted, sizeof(file_encrypted), 0, (struct sockaddr*)&remote, sizeof(remote));
			printf("The size of the file is: %ld\n",size_file);
			setsockopt(socket_id, SOL_SOCKET, SO_RCVTIMEO,&time_vals,sizeof(time_vals));
				while(conff_size < size_file)
				{
				if(buf_pkt != NULL)
				{
					buf_pkt->packet_index = seq;
					printf("\nThe packet : %d\n",buf_pkt->packet_index);
					obt_bytes = fread(buf_pkt->packet_descp,sizeof(char),BUFSIZE,fptr);
					buf_pkt->packet_len = obt_bytes;
					for(values=0; values<buf_pkt->packet_len; values++)
					{
						buf_pkt->packet_descp[values] = nbuf[values] + key[val2];
						val2++;
						if(val2 == 3)
						{
							val2 = 0;
						 }

				          }
					
					
					info_send = sendto(socket_id, buf_pkt, (sizeof(*buf_pkt)), 0, (struct sockaddr*)&remote, sizeof(remote));
					seq_check = recvfrom(socket_id, &seq_get, sizeof(seq_get), 0, NULL, NULL);
					if(seq_check<0)
					{
						printf("Same sequence has to be sent again %d\n",seq);
						fseek(fptr, conff_size, SEEK_SET);
					}
					else if(seq_check>0)
					{
						seq_dec = htonl(seq_get);
						printf("The received acknowledgment is %d\n", seq_dec);
						
						if(seq_dec != seq)
						{
							printf("The same sequence has to be sent again%d\n",seq);
							fseek(fptr, conff_size, SEEK_SET);	
							
						}

						else if(seq_dec == seq)
						{
							seq++;
							conff_size = conff_size + obt_bytes;
						
						}
					   }
				memset(buf_pkt, 0, sizeof(Packet_Details));
				}
				}
		
			}
			fclose(fptr);
}

/*void list_files(struct sockaddr_in remote_addr,int socket_id)
{
	printf("In client_list_directory case\n");
	ssize_t fileSize,encodedFileSize,size_check=0;
	int bytes_received=0;	
	int encoded_id;
	int sent_bytes=0;         
	char *file_buffer;
	int file_size_bytes = recvfrom( socket_id, &fileSize, sizeof(fileSize), 0, NULL,NULL);
	encodedFileSize = ntohl(fileSize);
	printf("File size received : %ld\n",encodedFileSize);
	int received_sequence_count = 0;
	char new_file[] = "list_file";
	FILE *fp;
	char key = 10;
	int i;
	fp = fopen(new_file,"w+");
	if(NULL == fp)
	{
		printf("Error opening the file\n");
		exit(0);
	}
	else
	{
		
		while(size_check < encodedFileSize)
		{
	
			struct Datagram *temp = malloc(sizeof(struct Datagram));
			if(temp != NULL)
			{
				bytes_received = recvfrom( socket_id, temp, sizeof(*temp), 0, NULL,NULL);
				if(received_sequence_count == temp->datagram_id)
				{
				
					for(long int count=0; count<temp->datagram_length; count++)
					{

						temp->datagram_message[count] ^= key;
					}
					
					fwrite(temp->datagram_message,1,temp->datagram_length,fp);
					printf("\nSending ACK : %d\n",temp->datagram_id);
					
					
					
					encoded_id = ntohl(temp->datagram_id);
					sent_bytes = sendto( socket_id, &encoded_id, sizeof(encoded_id), 0, (struct sockaddr*)&remote_addr, sizeof(remote_addr));

					received_sequence_count++;
					size_check += sizeof(temp->datagram_message);
				}
				else
				{
				
					encoded_id = ntohl(temp->datagram_id);
					sent_bytes = sendto( socket_id, &encoded_id, sizeof(encoded_id), 0, (struct sockaddr*)&remote_addr, sizeof(remote_addr));
					printf("\nSending ACK : %d\n",temp->datagram_id);
				
				}
				free(temp);
			}
		}
		//printf("\n**********************************************************************\n");
		printf("Contents of the Server:\n");
		//printf("**********************************************************************\n");
		rewind(fp);
		char print_buffer[BUFSIZE]="";
		bzero(print_buffer,sizeof(print_buffer));
		while (fgets(print_buffer, sizeof(print_buffer), fp)) {
			printf("%s", print_buffer); 
			bzero(print_buffer,sizeof(print_buffer));
		}
		//printf("**********************************************************************\n");
		printf("\nDone\n");
		fclose(fp);
	}
}*/

/* Main Function definition */
int main (int argc, char * argv[])
{
 
	/* Creating socket name */                          
	int sockfd, bytestot;
	/* Creating internet socket address structure */  
	struct sockaddr_in serveraddr; 
	uint8_t hash_buf[100];        
	uint8_t* fname;
	uint8_t* name_cmd;
	uint8_t cmd_out_exit, val[BUFSIZE], cmd[70];
	bzero(cmd,sizeof(cmd));
	bzero(val, sizeof(val));
	/* Check for input paramaters during execution */
	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	 
	/* Populating the sockaddr_in struct with information of server */
	/* Clearing the struct */	
	bzero(&serveraddr,sizeof(serveraddr)); 
                      
	/* Creating the address family */	
	serveraddr.sin_family = AF_INET;                 
	/* Sets port number to network byte order */	
	serveraddr.sin_port = htons(atoi(argv[2]));      
	/* Sets remote IP address */	
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]); 

	/* Creating a generic socket of type UDP (datagram) */
	if ((sockfd = socket(PF_INET,SOCK_DGRAM,0)) < 0)
	{
		printf("unable to create socket\n");
	}
	/* Loop for getting input from the user for different operation */
	while(1)
	{
		/* Initialising the timeout to be infinite for 'recvfrom' function */
		time_val1.tv_sec = 0;
		time_val1.tv_usec = 0;
		if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&time_val1,sizeof(time_val1)) < 0) 
		{
		    perror("Error\n");
		}		
		
		printf("Enter the command to be performed and type it:get [filename],put [filename],delete [filename],md5sum, ls,exit\n");
		scanf("%s", cmd);
		printf("First sending the entire command to the server : %s\n",cmd);
		bytestot = sendto(sockfd, cmd, strlen(cmd), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		printf("Number of bytes for the operation sent : %d\n",bytestot);
		name_cmd = strdup(cmd);
		strtok(name_cmd, " ");
		printf("The name of the command is: %s\n", name_cmd);
		fname = strtok(NULL, " ");
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
			ls_display(sockfd, fname, serveraddr);
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
			bytestot = recvfrom(sockfd, val, strlen(val), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
			printf("%s\n", val);
			‌
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

	}/* Closing the socket */
	close(sockfd);
}
