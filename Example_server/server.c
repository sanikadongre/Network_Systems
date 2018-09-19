/* Headers section */
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
#include <string.h>
#include <server.h>
#include <time.h>
#include <stdint.h>

#define BUFSIZE 100

struct timeval time_vals,time_val1,time_val2,time_done;

typedef struct
{
	uint32_t packet_index;
	uint8_t packet_descp[BUFSIZE];
	uint32_t packet_len;

}Packet_Details;
/* Main Function definition */
int main(int argc, char *argv[])
{
	int udp_sock,client_socket, bytestot = 0, file_del, exit_recv, bytestot1 = 0; 
	uint8_t hash_buf[100], recv_buf[BUFSIZE];
	uint8_t* fname;
	uint8_t* name_cmd;
	uint8_t cmd_out_exit, val[BUFSIZE] = "The server is exiting", cmd[70], fname1[70];                       
	struct sockaddr_in sin, remote_opt;     
	uint32_t remote_length; 
	bzero(&sin,sizeof(sin));  
	bzero(cmd, sizeof(cmd));
	bzero(fname1, sizeof(fname1));
	bzero(val, sizeof(val));
		
	/* Check for input paramaters during execution */
	if (argc != 2)
	{
		printf ("USAGE:  <port>\n");
		exit(1);
	}
	/* Assingn the address family */
	sin.sin_family = AF_INET;                  
	/* Set the input port number to network byte order using htons() function */	
	sin.sin_port = htons(atoi(argv[1]));        
	/* Supplies the IP address of the local machine */
	sin.sin_addr.s_addr = INADDR_ANY;          
	if ((udp_sock =socket(PF_INET,SOCK_DGRAM,0)) < 0)
	{
		printf("unable to create socket\n");
	}
	if (bind(udp_sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		printf("unable to bind socket\n");
	}
	remote_length = sizeof(remote_opt);
	while(1)
	{
		/* Initialising the timeout to be infinite for 'recvfrom' function */
		time_val1.tv_sec = 0;
		time_val1.tv_usec = 0;
		(setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO,&time_val1,sizeof(time_val1)) < 0); 
	
		bytestot = recvfrom(udp_sock, cmd, strlen(cmd), 0, (struct sockaddr*)&remote_opt, &remote_length);
		bytestot1 = recvfrom(udp_sock, fname1, strlen(fname1), 0, (struct sockaddr*)&remote_opt, &remote_length);
		printf("The command received from the client is : %s\n", cmd);
		name_cmd = strdup(cmd);
		fname= strdup(fname1);
		strtok(name_cmd, " ");
		printf("The name of the command is: %s\n", name_cmd);
		fname = strtok(fname, " ");
		printf("The file name is %s\n", fname);
		if(strcmp("get", name_cmd) == 0)
		{
		}

		else if(strcmp("put", name_cmd) == 0)
		{

		 }
		 else if(strcmp("ls", name_cmd) == 0)
		{
			
		}
					
		else if(strcmp("delete", name_cmd) == 0)
		{	
			FILE *f;
			bytestot = recvfrom(udp_sock, recv_buf, strlen(recv_buf), 0, (struct sockaddr*)&remote_opt, &(remote_length));
			bytestot1 = recvfrom(udp_sock, recv_buf, strlen(recv_buf), 0, (struct sockaddr*)&remote_opt, &(remote_length));
			f = fopen(recv_buf,"r");			
			if(f != NULL)
			{
				file_del = remove(recv_buf);
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
		else if(strcmp("md5sum", name_cmd) == 0)
		{
			strcpy(hash_buf, "md5sum");
			printf("To get the hash value of the file: %s\n", fname);
			strncat(hash_buf,fname,strlen(fname));
			printf("**************************\n");
			system(hash_buf);
			printf("***************************\n");
		}
		else if(strcmp("exit", name_cmd) == 0)
		{
	 		strcat(recv_buf, "Exit");
			printf("The recv_buf message is %s\n", recv_buf);
			bzero(recv_buf, sizeof(recv_buf));
			exit_recv = sendto(udp_sock, recv_buf, BUFSIZE, 0, (struct sockaddr*)&remote_opt, sizeof(remote_opt));
			printf("The exit command sent is %d\n", exit_recv);
        	 }
	 	else
		{
			printf("The entered command isn't appropriate\n");

	  	}
		bzero(cmd,sizeof(cmd));
		bzero(fname1, sizeof(fname1));
		bzero(val,sizeof(val));

	}

}	
