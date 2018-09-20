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
	uint32_t packet_index;
}Packet_Details;

void file_trans(int udp_sock, uint8_t* nm_file, struct sockaddr_in rem_addr)
{
	int bytes_received =0, bytes_file_received = 0, values=0, val2=0;
	ssize_t encrypted_size, conff_size = 0;
	uint8_t temp_file[BUFSIZE],key[4] = {'A', 'B', '5', '9'}, nbuf[BUFSIZE];
	FILE *fptr;
	fptr = fopen(temp_file,"w");
	   
		if(fptr != NULL)
		{
			printf("File open successful\n");
			while(conff_size < encrypted_size)
			{
				Packet_Details *buf_pkt = malloc(sizeof(Packet_Details));  
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
						printf("Acknowledgement sent %d\n",buf_pkt->packet_index);
						encrypted_msg = ntohl(buf_pkt->packet_index);
						info_send = sendto(udp_sock, &encrypted_msg, sizeof(encrypted_msg), 0, (struct sockaddr*)&rem_addr, sizeof(rem_addr));
						bytes_received++;
						conff_size = conff_size + sizeof(buf_pkt->packet_descp);
					}

					if(bytes_received != buf_pkt->packet_index)
					{
						printf("Acknowledgement sent %d\n",buf_pkt->packet_index);
						encrypted_msg = ntohl(buf_pkt->packet_index);
						info_send = sendto(udp_sock, &encrypted_msg, sizeof(encrypted_msg), 0, (struct sockaddr*)&rem_addr, sizeof(rem_addr));

					}
					free(buf_pkt);
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


struct timeval time_vals, time_val1, time_val2, time_done;
/* cmd_get
 * error - wrapper for perror
 */
void error(char *msg) {
    perror(msg);
    exit(0);
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
			get_file(sockfd, fname, serveraddr);
			printf("\nThe file get is done\n");
		}

		else if(strcmp("put", name_cmd) == 0)
		{
			printf("\nTo put the file by the client %s\n", fname);
			//put_file(sockfd, fname, serveraddr);
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
		      error("ERROR in sendto");
		    
		    /* print the server's reply */
		 n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serveraddr, &serverlen);
		    if (n < 0) 
		      error("ERROR in recvfrom");
	    printf("Echo from server: %s\n", buf);
		    n = recvfrom(sockfd, cmd, strlen(cmd), 0, (struct sockaddr*)&serveraddr, &serverlen);
		    if (n < 0) 
		      error("ERROR in recvfrom");
	    printf("Echo from server: %s\n", cmd);
	}
    	    return 0;
}
