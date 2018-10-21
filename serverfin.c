
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>

#define port 8349
#define BUFSIZE (1024)
#define MAXBUFSIZE (8*1048576)

uint8_t header[]= "www", incorrect_req[] =
"HTTP/1.1 500 Internal Server Error\r\n"
"Content-Type: text/html; charset = UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<body><center><h1>ERROR 500: Internal Server Error</h1><br>\r\n", max_data[MAXBUFSIZE], obtained_arr[BUFSIZE], obtained_data[50], response[256] = "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nConnection: %s\r\nContent-Length: %d\r\n\r\n";

uint8_t* ptr_data, size_f;
uint8_t req_method[256], req_url[256], req_version[256], req_connection[256];

void val_check(int value);
void shutdown_condition(int client_sock);


void get_request(int client_sock)
{
	strcat(header, req_url);
	printf(" The request url is: %s\n", header);
	FILE *fptr = fopen(header, "r");
	if (fptr == NULL)
	{
		perror("fopen failed");
	  	if (send(client_sock, incorrect_req,strlen(incorrect_req),0) < 0)
	  	{
	 	 	 perror("error in writing to the socket");
	  	}
		shutdown_condition(client_sock);
	}
	fseek(fptr,0,SEEK_END);
	int size_f = ftell(fptr);
	fseek(fptr,0,SEEK_SET);
	printf("\nThe size of the file is %d\n", size_f);
	if (fread(max_data,1,size_f,fptr)<0)
	{
		perror("Reading of file failed");
	}
	ptr_data = strchr(req_url, '.');
  	if (strcmp(ptr_data, ".html") == 0)
		strcpy(obtained_data, "text/html");
	else if (strcmp(ptr_data, ".txt") == 0)
		strcpy(obtained_data, "text/plain");
 	else if (strcmp(ptr_data, ".jpg") == 0)
		strcpy(obtained_data, "image/jpg");
	else if (strcmp(ptr_data, ".png") == 0)
		strcpy(obtained_data, "image/png");
	else if (strcmp(ptr_data, ".gif") == 0)
		strcpy(obtained_data, "image/gif");
	else if (strcmp(ptr_data, ".js") == 0)
		strcpy(obtained_data, "application/javascript");
	else strcpy(obtained_data, "text/css");
	printf("obtained data is %s\n", obtained_data);
	sprintf(obtained_arr,response, obtained_data, req_connection, size_f);
 	if (write(client_sock,obtained_arr,strlen(obtained_arr)) < 0)
	{
	 	perror(" writing to the socket failed");
	}
	printf("%s\n", obtained_arr);
	if (write(client_sock,max_data, size_f) < 0)
	{
		perror("writing to the socket failed");
	}
	 fclose(fptr);
	 shutdown_condition(client_sock);
	 printf("\nComplete\n");
}

void val_check(int value)
{
	value++;
	if(value % 100 == 0)
	{
		value == 0;
	}
}
void shutdown_condition(int client_sock)
{
	shutdown(client_sock,SHUT_RDWR);
    	close(client_sock);	
}

int main(int argc, char * argv[])
{

  	int val = 0, sockfd, client_sock[BUFSIZE], serverlen = 0, fork_child = 1, clientlen = 0, read_size = 0, write_size = 0, counter = 0;	
        uint8_t post_arr[BUFSIZE], old_arr[BUFSIZE], new_arr[BUFSIZE];
  	struct sockaddr_in server, client;
  	
	//Socket creation
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if((sockfd) == 0)
	{
		perror("Socket opening error");
	}
	puts("socket opened successsfully");
	
	server.sin_family = AF_INET;
  	server.sin_addr.s_addr = INADDR_ANY;
  	server.sin_port = htons(port);
	serverlen = sizeof(server);
	clientlen = sizeof(client);
	//Socket binding
	if (bind(sockfd, (struct sockaddr *) &server, serverlen) < 0)
	{
		perror("Socket binding error");
	}
	puts("bind is successfull");

	//Socket listening
	if(listen(sockfd,5) < 0)
	{
		perror("listen error");
	}
	puts("listening success");
	while(1)
	{
		//accepting connection
		 client_sock[val] = accept(sockfd, (struct sockaddr *) &client, &clientlen);
 		 if (client_sock[val]<0)
  		{
	 		perror("Connection accept error");
			close(client_sock[val]);
  		}
		puts("client connection accepted");
		read_size = read(client_sock[val],old_arr,sizeof(old_arr));
		if (read_size < 0)
		{
			perror("reading from socket error");
		}
		puts("socket read successfully");
		int j = 0;
		for(int i = 0; i<read_size; i++)
		{
			if(old_arr[i]!='\0')
			{
				new_arr[j] = old_arr[i];
				j++;
			}
		}
	  	uint8_t* req_char= strtok(new_arr, " \n");
		while( req_char != NULL ) 
		{
	  		counter++;
			switch(counter)
			{
	  			case 1:
				strcpy(req_method, req_char);
				break;
	  	 		
				case 2: 
				strcpy(req_url, req_char);
				break;

				case 3:				
				strcpy(req_version, req_char);
				break;
			}
	   		if (strcmp("Connection:", req_char) == 0)
			 {
			 	req_char = strtok(NULL, " \n");
		   		strcpy(req_connection, req_char);
			 	if (strcmp(req_connection, "keep-alive") != 0 | strcmp(req_connection, "Keepalive") != 0)
			 	{
				 	 strcpy(req_connection, "close");
				}

			 }

	  		req_char= strtok(NULL, " \n");
		 }
  		fork_child = fork();
		if (fork_child == 0)
		{
			if (strcmp(req_method,"GET") == 0)
			{
				get_request(client_sock[val]);
			}
    			else
			{
		        	write_size = send(client_sock[val], incorrect_req,strlen(incorrect_req),0);
				if (write_size < 0)
				{
					perror("writing to the socket error");
				}
				shutdown(client_sock[val],SHUT_RDWR);
				close(client_sock[val]);
			}
			exit(1);
		}
		val_check(val);
	}
  	close(sockfd);

	return 0;

}
