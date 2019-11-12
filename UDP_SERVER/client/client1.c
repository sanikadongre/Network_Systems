
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
/*Data encryption for relaibility*/
void encode(uint8_t *temp_buf, int size_len, uint8_t key)
{
	for(int i=0; i<size_len; i++)
	{
		temp_buf[i] ^= key;
	}
}

/*Structure for Packet details*/
typedef struct{
	int pckt_index;						
	int pckt_ack;														
	int packet_length;
	uint8_t data_descp[BUFSIZE];												
}Packet_Details;

int main (int argc, char * argv[])
{
		 			
	int sockfd, nbytes, portno, bytestot, bytestot1, n, read_length;                            
       	int serverlen;																								
       	uint8_t buffer[BUFSIZE], buf[BUFSIZE], val[BUFSIZE], fname1[70],  cmd[100], hash_buf[BUFSIZE], key=10;								
       	struct sockaddr_in serveraddr, remote;     
      	struct hostent *server_hp;							
      	uint8_t *name_cmd;														
      	uint8_t *fname;													
       	FILE *fptr;																			
        struct timeval timeout; 								
        bzero(cmd, sizeof(cmd));
        bzero(fname1, sizeof(fname1));
        bzero(val, sizeof(val));
	Packet_Details* a = malloc(sizeof(Packet_Details));
	Packet_Details* s = malloc(sizeof(Packet_Details));
	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	/* build the server's Internet address */
	bzero(&serveraddr,sizeof(serveraddr));               //zero the struct
	serveraddr.sin_family = AF_INET;                
	serveraddr.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address

	server_hp = gethostbyname(argv[1]);					 
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
		
		timeout.tv_sec = 0; //Timeout for ack 
		timeout.tv_usec = 0;
		setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

		bzero(buffer, sizeof(buffer)); 

		
		printf("Enter the command to be performed and type it: get [filename], put[filename], delete[filename], exit\n");
		gets(cmd);	
		nbytes = sendto(sockfd, cmd, sizeof(cmd) , 0, (struct sockaddr *)&serveraddr, serverlen);
		name_cmd = strdup(cmd); //Splitting string
		strtok(name_cmd, " ");
		fname = strtok(NULL, " ");
		printf("Filename: %s\n", fname);
		if(strcmp(name_cmd, "get") == 0)
		   {
			printf("Get the file: %s from the server.\n", fname);
			int packet_id=1; 	

			nbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &remote, &serverlen);
			if(!strcmp(buffer, "Error")){
				printf("File does not exist on the server. \n");
				continue;
		     }
			while(1)
			{
				bzero(s->data_descp, sizeof(s->data_descp));
				nbytes = recvfrom(sockfd, (Packet_Details*) s, sizeof(Packet_Details), 0, (struct sockaddr *) &remote, &serverlen);
				printf("Packet Size from the server: %d \n",nbytes);
				encode(s->data_descp, s->packet_length, key);
				if(s->pckt_index != packet_id) //if the packet index doesn't match the index that is expected then send again
				{
					a->pckt_ack=s->pckt_index;
					nbytes = sendto(sockfd, (Packet_Details*)a, sizeof(Packet_Details), 0, (struct sockaddr *)&serveraddr,serverlen);
					printf("Packet Size  that is being sent to server: %d and ACK sent: %d\n",nbytes, a->pckt_ack);
					
				}
				else if(s->pckt_index == packet_id){  //if they are equal then write to file
					printf(" Now write to the file\n" );
					FILE *fptr;
					fptr = fopen(fname,"ab");
					fwrite(s->data_descp, s->packet_length, 1, fptr);
					bzero(a->data_descp, sizeof(a->data_descp));
					fclose(fptr);
					a->pckt_ack=packet_id;
					nbytes = sendto(sockfd, (Packet_Details*) a, sizeof(Packet_Details), 0, (struct sockaddr *)&serveraddr, serverlen);
					printf("Packet Size being sent to server: %d and ACK sent: %d\n",nbytes, a->pckt_ack);
					packet_id++;
	
				}
				if(s->packet_length != BUFSIZE) 
				{
					break;
				}
			} 
			
			memset(a, 0, sizeof(Packet_Details)); //Free memory
			memset(s, 0, sizeof(Packet_Details));
		}
		else if(strcmp("put", name_cmd) == 0)
		{
			printf("Put the File: \"%s\" on the server.\n", fname);
			memset(a, 0, sizeof(Packet_Details));
			memset(s, 0, sizeof(Packet_Details));
			a->pckt_index = 1;
			s->pckt_ack = 0;
			timeout.tv_sec = 0;
			timeout.tv_usec = 300000;
			setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
			bzero(a->data_descp, sizeof(a->data_descp));
			FILE *fptr;
			fptr = fopen(fname, "rb");
			if(fptr != NULL)
			{
				char msg[] = "Filefound";
				nbytes = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&serveraddr, serverlen);
				
			}

			else if(fptr == NULL)
			{
				perror("File does nor exist or Error opening file\n");
				char msg[] = "Fileerror";
				nbytes = sendto(sockfd, msg, sizeof(msg), 0, (struct sockaddr *)&serveraddr, serverlen);
				continue;
				
			}
			
			while(1)
			{
				
				read_length = fread(a->data_descp, 1, BUFSIZE , fptr);
				a->packet_length = read_length;
				encode(a->data_descp, a->packet_length, key); //encrypting data to be sent on the server
				nbytes = sendto(sockfd, (Packet_Details*) a, sizeof(Packet_Details), 0, (struct sockaddr *)&serveraddr, serverlen);
				printf("Packet Size sent to the server: %d\n",nbytes);
				bzero(s->data_descp, sizeof(s->data_descp));
				nbytes = recvfrom(sockfd, (Packet_Details*) s, sizeof(Packet_Details), 0, (struct sockaddr *)&remote, &serverlen);
				if(nbytes < 0) //condition for timeout
				{
					printf("Timeout has occurred\n");
					fseek(fptr, (-1)*read_length, SEEK_CUR);
					continue;
				}
				else
				{
					printf("Packet Size from server: %d and ack_index from server: %d \n", nbytes, s->pckt_ack);
					if(s->pckt_ack == a->pckt_index){  
						a->pckt_index++; //incremeting the index 
				 }
				else{
						fseek(fptr, (-1)*read_length, SEEK_CUR); //Time out has ocurred
					
				}
				}
				if(a->packet_length != BUFSIZE)
				{
					break;
				}
			}
			memset(a, 0, sizeof(Packet_Details)); //free the memory
			memset(s, 0, sizeof(Packet_Details));
		}
		else if(strcmp("delete", name_cmd) == 0)
		{
			
			printf("Deleting the file with name: %s\n",fname);
			bytestot = sendto(sockfd, fname, strlen(fname), 0, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
		}

		else if(strcmp("exit", name_cmd) == 0)
		{
			printf("Exiting the server\n");
			bzero(buf, sizeof(buf));
			bytestot = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serveraddr, &(serverlen));
			printf("%s\n", buf);
			if(strcmp(buf, "exit") ==0)
			{
				printf("The server has exited successfully\n");
			}
			else
			{
				printf("Exiting the server\n");
			}
		}
		else
		{
			printf("This command is incorrect\n");
		        printf("Please Type an appropriate command\n");
		}
		bzero(val, sizeof(val));	
		bzero(buf,sizeof(buf));
		bzero(cmd, sizeof(cmd));
	}
}
