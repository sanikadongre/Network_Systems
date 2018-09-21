
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>

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
void data_encrypt(uint8_t *temp_buf, int len_d, uint8_t key[], uint8_t key1[]);
void data_decrypt(uint8_t *temp_buf, int len_d, uint8_t key[], uint8_t key1[]);


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

/***** MAIN FUNCTION - ENTRY POINT *****/
int main (int argc, char * argv[] )
{
	setvbuf(stdout, NULL, _IONBF, 0); 		//to set stdout to not buffer printf -> stackoverflow.com
	                          	// this will be our socket
	int sockfd, portno, clientlen, optval, n, client_socket, check = 1, bytestot = 0, file_del, exit_recv, bytestot1 = 0;     	//"Internet socket address structure"
	unsigned int remote_length;         	// length of the sockaddr_in structure                       		// number of bytes to be sent or received
	char buf[BUFSIZE];             	// buffer for storing the data to be sent/recieved
	char *name_cmd;												// to extract the command from user input at client												// to store the filename(in any) from user input at client
	FILE *fptr;
	uint8_t  hash_buf[BUFSIZE], recv_buf[BUFSIZE];
	uint8_t cmd_out_exit, val[BUFSIZE] = "The server is exiting", cmd[70], fname[70], key[4] = {'A', 'B', '5', '9'}, key1[6] = {'D', 'E', '7', '8', '9', '3'};
 	bzero(cmd, sizeof(cmd));
	bzero(fname, sizeof(fname));
 	bzero(val, sizeof(val));
	struct sockaddr_in sin,remote;
  	Packet_Details* buf_pkt = malloc(sizeof(Packet_Details));
  	Packet_Details* pkt_ack = malloc(sizeof(Packet_Details));
  	struct timeval timeout;															// declaring file pointer to perform file read/write
	int read_length;											// to store the length of the read data
	

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


	/***** Causes the system to create a generic socket of type UDP (datagram)*****/
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("unable to create socket");
	}

	/******************
	  Once we've created a socket, we must bind that socket to the
	  local address and port we've supplied in the sockaddr_in struct
	 ******************/
	if (bind(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf("unable to bind socket\n");
	}

	remote_length = sizeof(struct sockaddr_in);

	//waits for an incoming message
	while(1){
		/* Initializing timeout struct */
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

		bzero(buf, sizeof(buf));

		/* Recieving the command from the client */
		bytestot = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&remote, &remote_length);

		/***** Split the command from the user to get the command and filename if any
		 Refernce: http://www.geeksforgeeks.org/how-to-split-a-string-in-cc-python-and-java/
		 *****/

		/***** Condition: To send file to the client (requested by the client)
		Given a valid file name by the user, which is available
		at the server *****/
		if(strcmp("get", cmd) == 0){
			/* Setting Timeout */
			timeout.tv_sec = 0;
			timeout.tv_usec = 300000;
			setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
			buf_pkt->packet_index = 1;
			pkt_ack->packet_ack = 0;
			FILE *fptr;
			fptr = fopen(fname, "rb");
			if(fptr == NULL){
				perror("File does nor exist or Error opening file \n");
				char msg[] = "Error";
				bytestot = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_length);
				continue;
			}
			else{
				char msg[] = "Success";
				bytestot = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&remote, remote_length);
			}
			//Starting the loop for continous sending of data to the server
			do{
				bzero(buf_pkt->packet_descp, sizeof(buf_pkt->packet_descp));
				read_length = fread(buf_pkt->packet_descp,1,BUFSIZE, fptr);
				buf_pkt->byte = read_length;
				data_encrypt(buf_pkt->packet_descp, buf_pkt->byte, key, key1);
				bytestot= sendto(sockfd, (Packet_Details*)buf_pkt, sizeof(Packet_Details), 0, (struct sockaddr*)&remote, remote_length);
				bzero(buf_pkt->packet_descp, sizeof(buf_pkt->packet_descp));
				bytestot = recvfrom(sockfd, (Packet_Details*)pkt_ack, sizeof(Packet_Details), 0, (struct sockaddr*)&remote, &remote_length);
				//Packet Authentication
				if(bytestot<0){
					printf("---------------------Timeout--------------------------\n");
					fseek(fptr, (-1)*read_length, SEEK_CUR);
					continue;
				}
				printf("Packet Size from client: %d and client ack_index: %d \n",bytestot, pkt_ack->packet_ack);
				//incrementing Server packet index if the expected acknowledgement is recieved
				if(pkt_ack->packet_ack == buf_pkt->packet_index){
					buf_pkt->packet_index++;
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
			memset(pkt_ack, 0, sizeof(Packet_Details));
			memset(buf_pkt, 0, sizeof(Packet_Details));
		}

		/***** Condition: To recieve a file from the client (sent by the client)
		Given a valid file name by the user, which is available
		at the client *****/
		/*else if(!strcmp(cname,"put")){
			//clear memory for packet structres
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));

			int exp_index=1;	// Initializing the expected packet with 1
			s_pckt->pckt_ack = 1;//Initializing packet structure acknowledgement

			bzero(buf, sizeof(buf));
			bytestot = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *) &remote, &remote_length);

			if(!strcmp(buf, "Error")){
				continue;
			}
			FILE *fp;
			fp = fopen(filename,"ab");
			if(fp == NULL){
				perror("Error opening file \n");
			}

			do{
				bzero(c_pckt->data_buff, sizeof(c_pckt->data_buff));
				nbytes = recvfrom(sockfd, (struct_pckt*) c_pckt, sizeof(struct_pckt), 0, (struct sockaddr *) &remote, &remote_length);
				printf("Packet Size from the client: %d \n",nbytes);

				data_decryption(c_pckt->data_buff, c_pckt->len_data, key1, key2);//decrypting data sent by the client

				if(c_pckt->pckt_index == exp_index){
					printf("Writing to the file \n" );
					fwrite(c_pckt->data_buff, c_pckt->len_data, 1, fp);
					bzero(c_pckt->data_buff, sizeof(c_pckt->data_buff));
					s_pckt->pckt_ack=exp_index;
					//Sending Acknowledgement if the expected packet is recieved
					nbytes = sendto(sockfd, (struct_pckt*) s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *) &remote, remote_length);
					printf("Packet Size being sent to client: %d and ACK from server: %d\n",nbytes, s_pckt->pckt_ack);
					exp_index++;
				}
				else{
					s_pckt->pckt_ack=c_pckt->pckt_index;
					//Sending Acknowledgement if the expected packet not received
					nbytes = sendto(sockfd, (struct_pckt*)s_pckt, sizeof(struct_pckt), 0, (struct sockaddr *) &remote, remote_length);
				}
				if(c_pckt->len_data != BUFSIZE){
					break;
				}
			}while(1); // check for remaining data to be recieved
			fclose(fp);

			// Clearing the memory for the packet structures
			memset(c_pckt, 0, sizeof(struct_pckt));
			memset(s_pckt, 0, sizeof(struct_pckt));
		}*/

		/***** Condition: To delete a file from the server (requested by the client)
		Given a valid file name by the user, which is available
		at the server *****/
		/*else if(!strcmp(cname,"delete")){
			if(remove(filename)){
				char msg[] = "unable to delete file";
				// Sending acknowledgement for error in deleting the file.
				nbytes = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *) &remote, remote_length);
				perror("Error");
			}
			else{
				//Sending acknowledgement for deleting the file.
				char msg[] = "Deleted File";
				nbytes = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *) &remote, remote_length);
			}
		}*/

		/***** Condition: Client's request to list all file from the server
		Thus, sending the file-names in the directory to the client
		Reference: http://www.geeksforgeeks.org/c-program-list-files-sub-directories-directory/ *****/
		/*else if(!strcmp(cname,"ls")){
			DIR *current_dir;
			struct dirent *struct_dir;

			current_dir = opendir(".");
			if(current_dir == NULL){
				char msg[] = "Unable to read Directory";
				nbytes = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *) &remote, remote_length);
				perror("Error");
			}
			else{
				bzero(buf, sizeof(buf));
				//Concatinating the file names in the buffer
				while((struct_dir = readdir(current_dir)) != NULL){
					strcat(buffer, struct_dir->d_name);
					strcat(buffer, "\n");
				}
			nbytes = sendto(sockfd, buffer, sizeof(buffer) , 0, (struct sockaddr *)&remote, remote_length);
			}
		}*/
		/***** Condition: Client's request to the server
		to release the connection *****/
		/*else if(!strcmp(cname,"exit")){
			char msg[] = "Connection Released by Server";
			nbytes = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *) &remote, remote_length);
			close(sockfd);
			exit(0);
		}*/

		else{
			printf("Unsupported Command \n");
		}

		bzero(buf, sizeof(buf));
		}

}
