/***** INCLUDES *****/
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

/***** Declaring the data encryption/decryption functions *****/
void data_encryption(char *buffer, int data_len, char key1[], char key2[]);
void data_decryption(char *buffer, int data_len, char key1[], char key2[]);

/***** Packet Structure *****/
typedef struct{
	int pckt_index; 						// to store the packet index
	int pckt_ack;								// to store the packet acknowledgement
	char data_buff[BUFSIZE];	// to store packet data
	int len_data;								// to store packet data length
}struct_pckt;

/***** DATA Encryption Function *****/
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


/***** MAIN FUNCTION - ENTRY POINT *****/
int main (int argc, char * argv[])
{
		 			
	int nbytes;                            
	int serverlen;															
	int sockfd;                              
	char command[100];											
	char buffer[BUFSIZE];								
	struct sockaddr_in serveraddr, remote;     
	struct hostent *server_hp;							
	char *cname;														
	char *filename;													
	FILE *fp;															
	struct_pckt* c_pckt = malloc(sizeof(struct_pckt)); 				
	struct_pckt* s_pckt = malloc(sizeof(struct_pckt)); 				
	struct timeval timeout; 								
	int read_length;												
	char key1[39] = "MyMethodOfEncrptingTheDataIIntendToSend";
	char key2[59] = "ThisIsMySecondKeyForMyMethodOfEncrptingTheDataIIntendToSend";

/***** Check for required number of command line arguments
to connect to server *****/
	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	/******************
	  Here we populate a sockaddr_in struct with
	  information regarding where we'd like to send our packet
	  i.e the Server.
	 ******************/
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

	/******************
	  sendto() sends immediately.
	  it will report an error if the message fails to leave the computer
	  however, with UDP, there is no error if the message is lost in the network once it leaves the computer.
	 ******************/
	serverlen = sizeof(struct sockaddr_in);

	while(1){
		
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

		bzero(buffer, sizeof(buffer)); // clear buffer

		
		printf("\n\tEnter Command as under: \n");
		printf("\t1. get <filename> \n" "\t2. put <filename>\n" "\t3. delete <filename>\n"
		"\t4. ls\n" "\t5. exit \n" );
		gets(command);	
		nbytes = sendto(sockfd, command, sizeof(command) , 0, (struct sockaddr *)&serveraddr, serverlen)
		cname = strdup(command);
		strtok(cname, " ");
		filename = strtok(NULL, " ");
		printf("Filename: %s\n", filename);
		if(!strcmp(cname, "get")){
			printf("Get File: %s from the server.\n", filename);
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
					FILE *fp;
					fp = fopen(filename,"ab");
					fwrite(s_pckt->data_buff, s_pckt->len_data, 1, fp);
					bzero(c_pckt->data_buff, sizeof(c_pckt->data_buff));
					fclose(fp);

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

		else if(!strcmp(cname, "put")){
			printf("Put File: \"%s\" on the server.\n", filename);
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));
			c_pckt->pckt_index = 1;
			s_pckt->pckt_ack = 0;
			timeout.tv_sec = 0;
			timeout.tv_usec = 300000;
			setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

			FILE *fp;
			fp = fopen(filename, "rb");
			if(fp == NULL){
				perror("File does nor exist or Error opening file\n");
				char msg[] = "Error";
				nbytes = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&serveraddr, serverlen);
				continue;
			}

			else{
				char msg[] = "Success";
				nbytes = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&serveraddr, serverlen);
			}
			do{
				bzero(c_pckt->data_buff, sizeof(c_pckt->data_buff));
				read_length = fread(c_pckt->data_buff, 1, BUFSIZE , fp);
				c_pckt->len_data = read_length;

				data_encryption(c_pckt->data_buff, c_pckt->len_data, key1, key2); //encrypting data to be sent on the server
				nbytes = sendto(sockfd, (struct_pckt*) c_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&serveraddr, serverlen);
				printf("Packet Size sent to the server: %d\n",nbytes);

				bzero(s_pckt->data_buff, sizeof(s_pckt->data_buff));
				nbytes = recvfrom(sockfd, (struct_pckt*) s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&remote, &serverlen);
				if(nbytes < 0){
					printf("---------------------Timeout--------------------------\n");
					fseek(fp, (-1)*read_length, SEEK_CUR);
					continue;
				}
				else{
					printf("Packet Size from server: %d and ack_index from server: %d \n", nbytes, s_pckt->pckt_ack);
					if(s_pckt->pckt_ack == c_pckt->pckt_index){
						c_pckt->pckt_index++;
					}
					else{
						fseek(fp, (-1)*read_length, SEEK_CUR);
					}
				}
				if(read_length != BUFSIZE){
					break;
				}
			}while(1);
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));
		}
		else if(!strcmp(cname, "delete")){

			nbytes = recvfrom(sockfd, buffer, BUFSIZE, 0, (struct sockaddr *)&remote, &serverlen);
			printf(" %s \n", buffer );
		}
		else if(!strcmp(cname, "ls")){
			printf("\nListing Files from the server.\n");
			bzero(buffer, sizeof(buffer));
			nbytes = recvfrom(sockfd, buffer, BUFSIZE, 0, (struct sockaddr *)&remote, &serverlen);
			printf("%s\n", buffer ); //Printing data recieved from the buffer
		}

		else if(!strcmp(cname, "exit")){
			printf("Request server to release the connection.\n");
			nbytes = recvfrom(sockfd, buffer, BUFSIZE, 0, (struct sockaddr *)&remote, &serverlen);
			printf(" %s \n", buffer );
		}

		else{
			printf("Unsupported Command\n");
		}

		bzero(buffer,sizeof(buffer)); 
	}
}
