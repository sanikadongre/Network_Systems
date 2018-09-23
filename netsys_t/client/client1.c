
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <stdint.h>


#define BUFSIZE (1024)

/* cmd_get
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
}

void data_encryption(char *buffer, int data_len, char key1[], char key2[]);
void data_decryption(char *buffer, int data_len, char key1[], char key2[]);


typedef struct{
	int pckt_index; 						
	int pckt_ack;								
	char data_buff[BUFSIZE];	
	int len_data;								
}struct_pckt;


void data_encryption(char *buffer, int data_len, char key1[], char key2[]){
	for(int i=0; i<data_len; i++){
		buffer[i] ^= key1[i%38];
	}
	for(int j=0; j<data_len; j++){
		buffer[j] ^= key2[j%58];
	}
}

/***** DATA DECRYPTION FUNCTION *****/
void data_decryption(char *buffer, int data_len, char key1[], char key2[]){
	for(int i=0; i<data_len; i++){
		buffer[i] ^= key2[i%58];
	}
	for(int j=0; j<data_len; j++){
		buffer[j] ^= key1[j%38];
	}
}


int main (int argc, char * argv[])
{
		 			
	int sockfd, nbytes, portno, bytestot, bytestot1, n, read_length;                            
       	int serverlen;																								
       	uint8_t buffer[BUFSIZE], buf[BUFSIZE], val[BUFSIZE], fname1[70],  cmd[100], hash_buf[BUFSIZE];								
       	struct sockaddr_in serveraddr, remote;     
      	struct hostent *server_hp;							
      	uint8_t *name_cmd;														
      	uint8_t *fname;													
       	FILE *fptr;															
       	struct_pckt* c_pckt = malloc(sizeof(struct_pckt)); 				
       	struct_pckt* s_pckt = malloc(sizeof(struct_pckt)); 				
        struct timeval timeout; 								
        bzero(cmd, sizeof(cmd));
        bzero(fname1, sizeof(fname1));
        bzero(val, sizeof(val));
	char key1[39] = "MyMethodOfEncrptingTheDataIIntendToSend";
	char key2[59] = "ThisIsMySecondKeyForMyMethodOfEncrptingTheDataIIntendToSend";

	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	/* build the server's Internet address */
	bzero(&serveraddr,sizeof(serveraddr));               //zero the struct
	serveraddr.sin_family = AF_INET;                 //address family
	serveraddr.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address

	server_hp = gethostbyname(argv[1]);					 // Return information about host in argv[1]
	if(server_hp < 0){
			perror("Host Unknown");
	}

	bcopy((char*)server_hp->h_addr, (char*)&serveraddr.sin_addr, server_hp->h_length);

	/***** Causes the system to create a generic socket of type UDP (datagram) *****/
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("unable to create socket");
	}

	serverlen = sizeof(struct sockaddr_in);

	while(1){
		
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

		bzero(buffer, sizeof(buffer)); 

		
		printf("Enter the command to be performed and type it: get [filename], put[filename], delete[filename], ls, exit\n");
		gets(cmd);	
		nbytes = sendto(sockfd, cmd, sizeof(cmd) , 0, (struct sockaddr *)&serveraddr, serverlen);
		name_cmd = strdup(cmd);
		strtok(name_cmd, " ");
		fname = strtok(NULL, " ");
		printf("Filename: %s\n", fname);
		if(!strcmp(name_cmd, "get")){
			printf("Get File: %s from the server.\n", fname);
			int exp_index=1; 	

			nbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &remote, &serverlen);
			if(!strcmp(buffer, "Error")){
				printf("File does not exist on the server. \n");
				continue;
			}
			do{
				bzero(s_pckt->data_buff, sizeof(s_pckt->data_buff));
				nbytes = recvfrom(sockfd, (struct_pckt*) s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *) &remote, &serverlen);

				printf("Packet Size from the server: %d \n",nbytes);

				data_decryption(s_pckt->data_buff, s_pckt->len_data, key1, key2);
				if(s_pckt->pckt_index == exp_index){
					printf("Writing to the file\n" );
					FILE *fptr;
					fptr = fopen(fname,"ab");
					fwrite(s_pckt->data_buff, s_pckt->len_data, 1, fptr);
					bzero(c_pckt->data_buff, sizeof(c_pckt->data_buff));
					fclose(fptr);

					c_pckt->pckt_ack=exp_index;
					nbytes = sendto(sockfd, (struct_pckt*) c_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&serveraddr, serverlen);
					printf("Packet Size being sent to server: %d and ACK sent: %d\n",nbytes, c_pckt->pckt_ack);
					exp_index++;
				}
				else{
					c_pckt->pckt_ack=s_pckt->pckt_index;
					nbytes = sendto(sockfd, (struct_pckt*)c_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&serveraddr,serverlen);
					printf("Packet Size being sent to server: %d and ACK sent: %d\n",nbytes, c_pckt->pckt_ack);
				}
				if(s_pckt->len_data != BUFSIZE){
					break;
				}
			}while(1); 
			
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));
		}

		else if(strcmp("put", name_cmd) == 0)
		{
			printf("Put File: \"%s\" on the server.\n", fname);
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));
			c_pckt->pckt_index = 1;
			s_pckt->pckt_ack = 0;
			timeout.tv_sec = 0;
			timeout.tv_usec = 300000;
			setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

			FILE *fptr;
			fptr = fopen(fname, "rb");
			if(fptr == NULL){
				perror("File does nor exist or Error opening file\n");
				char msg[] = "Error";
				nbytes = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&serveraddr, serverlen);
				continue;
			}

			else
			{
				char msg[] = "Success";
				nbytes = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&serveraddr, serverlen);
			}
			do{
				bzero(c_pckt->data_buff, sizeof(c_pckt->data_buff));
				read_length = fread(c_pckt->data_buff, 1, BUFSIZE , fptr);
				c_pckt->len_data = read_length;

				data_encryption(c_pckt->data_buff, c_pckt->len_data, key1, key2); //encrypting data to be sent on the server
				nbytes = sendto(sockfd, (struct_pckt*) c_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&serveraddr, serverlen);
				printf("Packet Size sent to the server: %d\n",nbytes);

				bzero(s_pckt->data_buff, sizeof(s_pckt->data_buff));
				nbytes = recvfrom(sockfd, (struct_pckt*) s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&remote, &serverlen);
				if(nbytes < 0){
					printf("---------------------Timeout--------------------------\n");
					fseek(fptr, (-1)*read_length, SEEK_CUR);
					continue;
				}
				else
				{
					printf("Packet Size from server: %d and ack_index from server: %d \n", nbytes, s_pckt->pckt_ack);
					if(s_pckt->pckt_ack == c_pckt->pckt_index){
						c_pckt->pckt_index++;
				}
				else{
						fseek(fptr, (-1)*read_length, SEEK_CUR);
					
				}
				}
				if(read_length != BUFSIZE){
					break;
				}
			}while(1);
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));
		}
		else if(strcmp("ls", name_cmd) == 0)
		{

			printf("\nTo list all the files in the directory%s\n", val);
			bzero(val, sizeof(val));
			bytestot = recvfrom(sockfd, val, strlen(val), 0, (struct sockaddr*)&serveraddr, &(serverlen));
			printf("\nThe dircetories and files are \n");
			printf("%s\n", val);
		}
		else if(strcmp("delete", name_cmd) == 0)
		{
			
			printf("Deleting the file with name: %s\n",fname);
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
		else if(strcmp("md5", name_cmd) == 0)
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
			printf("This command is incorrect\n");
		        printf("Please Type an appropriate command\n");
		}
		bzero(val, sizeof(val));	
		bzero(buf,sizeof(buf));
		bzero(cmd, sizeof(cmd));
		serverlen = sizeof(serveraddr);
		    n = sendto(sockfd, cmd, strlen(cmd), 0, (struct sockaddr*)&serveraddr, serverlen);
		    if (n < 0) 
		      error("ERROR in sendto");
		    
		    /* print the server's reply */
		 n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serveraddr, &serverlen);
		    if (n < 0) 
		      error("ERROR in recvfrom");
	}
}
