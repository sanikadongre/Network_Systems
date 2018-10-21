#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <stdint.h>

#define port 8650
#define BUFSIZE 100

uint8_t header[]= "www", incorrect_req[] =
"HTTP/1.1 500 Internal Server Error\r\n"
"Content-Type: text/html; charset = UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<body><center><h1>ERROR 500: Internal Server Error</h1><br>\r\n";

void get_request(int client_sock, char request_url[], char version[], char connection[])
{
	char data_buffer[1024*1024*4] = {};
	char data_content[1024] = {};
	char *content_type;
	char content[50] = {};
	strcat(header, request_url);
	printf(" The request url is: %s\n", header);
	FILE *fd = fopen(header, "r");
	if (fd == NULL)
	{
		perror("fopen failed");
		int write_var = send(client_sock, incorrect_req,strlen(incorrect_req),0);
	  if (write_var < 0)
	  {
	 	  perror("ERROR writing to socket");
	  }
		shutdown(client_sock,SHUT_RDWR);
    close(client_sock);
		return;
	}
	// Get file size
	printf("fopen done!\n");
	fseek(fd,0,SEEK_END);
	int file_size = ftell(fd);
	fseek(fd,0,SEEK_SET);
	printf("\nfile size %d\n",file_size);
	int fr = fread(data_buffer,1,file_size,fd);
	if (fr<0)
	{
		perror("fread failed");
	}
	printf("fread done fr = %d\n",fr);

	content_type = strchr(request_url, '.');
	printf("content_type %s\n", content_type);

  if (strcmp(content_type, ".html") == 0)
		strcpy(content, "text/html");
	else if (strcmp(content_type, ".txt") == 0)
		strcpy(content, "text/plain");
  else if (strcmp(content_type, ".jpg") == 0)
		strcpy(content, "image/jpg");
	else if (strcmp(content_type, ".png") == 0)
		strcpy(content, "image/png");
	else if (strcmp(content_type, ".gif") == 0)
		strcpy(content, "image/gif");
	else if (strcmp(content_type, ".js") == 0)
		strcpy(content, "application/javascript");
	else strcpy(content, "text/css");

	printf("content %s\n", content);
	sprintf(data_content,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nConnection: %s\r\nContent-Length: %d\r\n\r\n",content, connection, file_size);

 	int write_var = write(client_sock,data_content,strlen(data_content));
 	if (write_var < 0)
	{
	 	perror("ERROR writing to socket");
	}

	printf("%s\n", data_content);
	 write_var = write(client_sock,data_buffer,file_size);
	 if (write_var < 0)
	 {
		 perror("ERROR writing to socket");
	 }
	 else if(write_var > 0)
	 {
		 printf("write_var %d file_size %d\n",write_var,file_size );
	 }
	 else
	 {
		 printf("Write_var = 0\n" );
	 }
	 fclose(fd);
	 shutdown(client_sock,SHUT_RDWR);
	 close(client_sock);
	 printf("\nComplete\n");
}

/*
POST FUNCTION
*/
void post_request(int client_sock, char request_url[], char version[], char connection[], char new_arr[])
{
	printf("**************Received*****************\n");
	printf("%s\n", new_arr);
	printf("**************Received*****************\n");
	int write_var = write(client_sock,new_arr,strlen(new_arr));
 	if (write_var < 0)
	{
	 	perror("ERROR writing to socket");
	}

}

int main(int argc, char * argv[])
{

  	int val = 0, sockfd, client_sock[1024], serverlen = 0, child_thread, clientlen = 0, read_size = 0, write_size = 0;	
        uint8_t buffer[256], new_arr[1024], old_arr[1024];
  	struct sockaddr_in server, client;
  	
	//Socket creation
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if((sockfd) == 0)
	{
		perror("Socket opening error");
	}
	puts("socket opened successfully");

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
	puts("listening error");
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
			perror("error reading from socket");
		}
		puts("socket can be read successfully");
		char new_buffer[1024] = {};
		int j = 0;
	for(int i = 0; i<read_size; i++)
	{
		if(old_arr[i]!='\0')
		{
			new_buffer[j] = old_arr[i];
			j++;
		}
	}

	printf("***************************************\n");
	printf("%s\n", new_buffer);
	printf("***************************************\n");
	   /* get the first token */
	char* token = strtok(new_buffer, " \n");

	char request_method[100], request_url[100], request_version[100], connection[100];
	int token_count = 0;

  /* walk through other tokens */
	while( token != NULL ) {
	   token_count++;

	   if (token_count == 1)
			strcpy(request_method, token);
	   else if (token_count == 2)
		{
			 strcpy(request_url, token);
		}
	   else if (token_count == 3)
				strcpy(request_version, token);
	   else if (strcmp(token, "Connection:") == 0)
		 {
			 token = strtok(NULL, " \n");
		   strcpy(connection, token);
			 if (strcmp(connection, "keep-alive") != 0 | strcmp(connection, "Keepalive") != 0)
			 {
				  strcpy(connection, "close");
			 }
			 if (strcmp(request_method, "POST") == 0)
			 {
				 token = strtok(NULL, " \n");
				 token = strtok(NULL, " \n");
				 strcpy(new_arr, token);

				 while(token !=  "\0")
				 {
					  token = strtok(NULL, " ");
						if(token == NULL)
						{
							break;
						}
						strcat(new_arr, " ");
					 	strcat(new_arr, token);

				 }
				 printf("data %s\n", new_arr);

			 }

		 }

	   token = strtok(NULL, " \n");
	 }
  child_thread = fork();

	if (child_thread == 0)
	{
		if (strcmp(request_method,"GET") == 0)
		{
			get_request(client_sock[val],request_url,request_version,connection);
		}
		else if (strcmp(request_method, "POST") == 0)
		{
			post_request(client_sock[val],request_url,request_version,connection, new_arr);
		}
    else
		{
		        write_size = send(client_sock[val], incorrect_req,strlen(incorrect_req),0);
			if (write_size < 0)
			{
				perror("ERROR writing to socket");
			}
			shutdown(client_sock[val],SHUT_RDWR);
			close(client_sock[val]);
		}
		exit(1);
	}
  val++;
	val = val%99;
}
  close(sockfd);

	return 0;

}
