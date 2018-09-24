
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

void data_encryption(char *buffer, int data_len, char key1[], char key2[]);
void data_decryption(char *buffer, int data_len, char key1[], char key2[]);

/***** Packet Structure *****/
typedef struct{
	int pckt_index;										// to store the packet index
	int pckt_ack;											// to store the packet acknowledgement
	char data_buff[BUFSIZE];				// to store packet data
	int len_data;											// to store packet data length
}struct_pckt;

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
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

/***** DATA Encryption Function *****/
void data_encryption(char *buffer, int data_len, char key1[], char key2[]){
	for(int i=0; i<data_len; i++){
		buffer[i] ^= key1[i%38];
	}
	for(int j=0; j<data_len; j++){
		buffer[j] ^= key2[j%58];
	}
}

/***** MAIN FUNCTION - ENTRY POINT *****/
int main (int argc, char * argv[] )
{
			//to set stdout to not buffer printf -> stackoverflow.com
	int sockfd, portno, clientlen, optval, n, bytestot =0;                           	// this will be our socket
	struct sockaddr_in sin, clientaddr;     	//"Internet socket address structure
	int nbytes;                        		// number of bytes to be sent or received
	char buffer[BUFSIZE], buf[BUFSIZE], hash_buf[BUFSIZE], recv_buf[BUFSIZE];             	// buffer for storing the data to be sent/recieved
	char *cname;													// to extract the command from user input at client
	char *filename;												// to store the filename(in any) from user input at client
	struct_pckt* c_pckt = malloc(sizeof(struct_pckt));// defining packet structure for client packet
	struct_pckt* s_pckt = malloc(sizeof(struct_pckt));// defining packet structure for server packet
	FILE *fptr;															// declaring file pointer to perform file read/write
	int read_length;	
	uint8_t cmd_out_exit, val[BUFSIZE] = "The server is exiting", cmd[70], fname[70], check = 1;
  	bzero(cmd, sizeof(cmd));
  	bzero(fname, sizeof(fname));
  	bzero(val, sizeof(val));
  	uint8_t msg_conf[] = " ";// to store the length of the read data
	struct timeval timeout;								// defining timeout structure
	char key1[39] = "MyMethodOfEncrptingTheDataIIntendToSend"; // Data encryption key-1
	char key2[59] = "ThisIsMySecondKeyForMyMethodOfEncrptingTheDataIIntendToSend";// Data encryption key-2


/***** Check for required number of command line arguments
to start the server *****/
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

	//waits for an incoming message
	while(1){
		/* Initializing timeout struct */
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

		bzero(buffer, sizeof(buffer));

		/* Recieving the command from the client */
		nbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientaddr, &clientlen);

		/***** Split the command from the user to get the command and filename if any
		 Refernce: http://www.geeksforgeeks.org/how-to-split-a-string-in-cc-python-and-java/
		 *****/
		cname = strdup(buffer);
		strtok(cname," ");
		filename = strtok(NULL, " ");

		printf("Command Recieved: %s \n", cname);
		printf("File name:%s\n", filename );

		/***** Condition: To send file to the client (requested by the client)
		Given a valid file name by the user, which is available
		at the server *****/
		if(!strcmp(cname, "get")){
			/* Setting Timeout */
			timeout.tv_sec = 0;
			timeout.tv_usec = 300000;
			setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

			s_pckt->pckt_index = 1;	// Initializing the server packet index with 1
			c_pckt->pckt_ack = 0;		// Initializing the packet acknowledgement with 0

			FILE *fptr;
			fptr = fopen(filename, "rb");
			if(fptr == NULL){
				perror("File does nor exist or Error opening file \n");
				char msg[] = "Error";
				nbytes = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&clientaddr, clientlen);
				continue;
			}
			else{
				char msg[] = "Success";
				nbytes = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&clientaddr, clientlen);
			}
			//Starting the loop for continous sending of data to the server
			do{
				bzero(s_pckt->data_buff, sizeof(s_pckt->data_buff));
				read_length = fread(s_pckt->data_buff, 1, BUFSIZE , fptr);
				s_pckt->len_data = read_length;
				printf("Read length%d\n", read_length );

				data_encryption(s_pckt->data_buff, s_pckt->len_data, key1, key2);//Encrypting data to be sent to the client
				nbytes = sendto(sockfd, (struct_pckt*) s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&clientaddr, clientlen);
				printf("Packet Size send to client: %d\n",nbytes);

				bzero(s_pckt->data_buff, sizeof(s_pckt->data_buff));
				nbytes = recvfrom(sockfd, (struct_pckt*) c_pckt, sizeof(struct_pckt), 0, (struct sockaddr *)&clientaddr, &clientlen);

				//Packet Authentication
				if(nbytes<0){
					printf("---------------------Timeout--------------------------\n");
					fseek(fptr, (-1)*read_length, SEEK_CUR);
					continue;
				}
				printf("Packet Size from client: %d and client ack_index: %d \n",nbytes, c_pckt->pckt_ack);

				//incrementing Server packet index if the expected acknowledgement is recieved
				if(c_pckt->pckt_ack == s_pckt->pckt_index){
					s_pckt->pckt_index++;
				}
				//retaining Server packet index if the expected acknowledgement is not recieved
				else{
					fseek(fptr, (-1)*read_length, SEEK_CUR);
				}
				if(read_length != BUFSIZE){
					break;
				}
			}while(1);

			// Clearing the memory for the packet structures
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));
		}

		/***** Condition: To recieve a file from the client (sent by the client)
		Given a valid file name by the user, which is available
		at the client *****/
		else if(!strcmp(cname,"put")){
			//clear memory for packet structres
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));

			int exp_index=1;	// Initializing the expected packet with 1
			s_pckt->pckt_ack = 1;//Initializing packet structure acknowledgement

			bzero(buffer, sizeof(buffer));
			nbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &clientaddr, &clientlen);

			if(!strcmp(buffer, "Error")){
				continue;
			}
			FILE *fptr;
			fptr = fopen(filename,"ab");
			if(fptr == NULL){
				perror("Error opening file \n");
			}

			/* Loop for continous receiving of valid packets */
			do{
				bzero(c_pckt->data_buff, sizeof(c_pckt->data_buff));
				nbytes = recvfrom(sockfd, (struct_pckt*) c_pckt, sizeof(struct_pckt), 0, (struct sockaddr *) &clientaddr, &clientlen);
				printf("Packet Size from the client: %d \n",nbytes);

				data_decryption(c_pckt->data_buff, c_pckt->len_data, key1, key2);//decrypting data sent by the client

				/* Packet Validation */
				if(c_pckt->pckt_index == exp_index){
					printf("Writing to the file \n" );
					fwrite(c_pckt->data_buff, c_pckt->len_data, 1, fptr);
					bzero(c_pckt->data_buff, sizeof(c_pckt->data_buff));
					s_pckt->pckt_ack=exp_index;
					//Sending Acknowledgement if the expected packet is recieved
					nbytes = sendto(sockfd, (struct_pckt*) s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *) &clientaddr, clientlen);
					printf("Packet Size being sent to client: %d and ACK from server: %d\n",nbytes, s_pckt->pckt_ack);
					exp_index++;
				}
				else{
					s_pckt->pckt_ack=c_pckt->pckt_index;
					//Sending Acknowledgement if the expected packet not received
					nbytes = sendto(sockfd, (struct_pckt*)s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *) &clientaddr, clientlen);
				if(c_pckt->len_data != BUFSIZE){
					break;
				}
			}while(1); // check for remaining data to be recieved
			fclose(fptr);

			// Clearing the memory for the packet structures
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));
		}

		/***** Condition: To delete a file from the server (requested by the client)
		Given a valid file name by the user, which is available
		at the server *****/
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

		
		else if(!strcmp(cname,"ls")){
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
				while((dir_ls == readdir(direct_ls)) != NULL)
				{
					strcat(fname, dir_ls->d_name);
					strcat(fname, "\n");
				}
				bytestot = sendto(sockfd, fname, sizeof(fname), 0, (struct sockaddr*)&clientaddr, clientlen);
			}
			}
			else{
				bzero(buffer, sizeof(buffer));
				//Concatinating the file names in the buffer
				while((struct_dir = readdir(current_dir)) != NULL){
					strcat(buffer, struct_dir->d_name);
					strcat(buffer, "\n");
				}
			/* Sending the file-names to the client*/
			nbytes = sendto(sockfd, buffer, sizeof(buffer) , 0, (struct sockaddr *)&clientaddr, clientlen);
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
