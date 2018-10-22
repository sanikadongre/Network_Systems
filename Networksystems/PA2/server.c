/****************************************************
*TCP server handling multiple requests using fork
*Netsys Programming Assignment 2
*Date: 10/14/2018
*Author: Sanika Dongre
******************************************************/

#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>

#define MAXBUFSIZE (307200) //Maximum buffsize
#define BUFSIZE (102400) 
#define port (8233) //port

uint8_t url_val[MAXBUFSIZE] = "/home/sanika/Documents/Networksystems/PA2/www"; //url
uint8_t incorrect_req[] =
"HTTP/1.1 500 internal server error\r\n"
"Content-Type: text/html; charset = UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<body><center><h1>ERROR 500:INTERNAL SERVER </h1><br>\r\n", ok_response[] = "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n", ok_response_post[] = "HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n<html><body><pre><h1>%s</h1></pre>\r\n";

uint8_t* req_version; //request version

void val_check (int value);
struct timeval timeout; //structure for timeout testing

void shutdown_condition(int connect)  
{
    shutdown(connect, SHUT_RDWR); //shutdown
    close(connect); //closing socket
}

/*Function for 500 internal error handling*/
void internal_error(int sock_error)
{
	int err_val = 0;
	perror("invalid request leads to 500 internal error");
	err_val = send(sock_error, incorrect_req, strlen(incorrect_req), 0); //sending error 
	shutdown_condition(sock_error); //shutdown

}

/***********************************************************************************
*get data: To receive the request and split into header, url, version using strtok
* strtok reference provided in function
*and then to check which file is requested and provide files accordingly
***********************************************************************************/
void get_data(int sock_data, int data, uint8_t arr1[], uint8_t arr2[])
{
          uint8_t client_data[MAXBUFSIZE], header_val[20], buff[MAXBUFSIZE];
	  uint8_t* ptr_data;
	  char* req_method;
	  char* req_url;
          char* ptr2;
	  int file_write, header_bytes = 0, file_size, rw_f;
	  FILE *fptr;

	  if(data > 0)
	  {
	   	//strtok use reference: https://www.geeksforgeeks.org/strtok-strtok_r-functions-c-examples/
	    	printf("Data from client is %s\n", arr1);
	    	strncpy(client_data, arr1,data);
	    	req_method=strtok(client_data," \n"); //extracting method
	    	req_url =strtok(NULL," \n"); //extracting url
	    	req_version =strtok(NULL," \n"); //extracting version
	    	printf(" method: %s url: %s version: %s", req_method, req_url, req_version);
	  }
	  if((strncmp(req_version,"HTTP/1.1",8)==0) || (strncmp(req_version,"HTTP/1.0",8)==0)) //checking request version
	  {
	    	printf("Received the requested version %s\n", req_version);
	  }
	  else
	  {
	    	internal_error(sock_data); //control goes to the 500 internal error loop
	  }
	  if((strncmp(req_method,"GET\0",4)==0)||(strncmp(req_method,"POST\0",4)==0)) //checking request method
	  {
		printf("Received the req method %s\n", req_method);
	  }
	  else
	  {
	    	internal_error(sock_data); //control goes to the 500 internal error loop
	  }
	  if(strncmp(req_method,"GET\0",4)==0) //The condition where method is get
	  {
	  	
	  	strcat(url_val, req_url);
	  	printf("\n The url request is %s\n", url_val);
	  	fptr =fopen(url_val,"r");
	  	if (fptr==NULL)
	  	{
			printf("File can't be opened\n");
	   		internal_error(sock_data); //Indicating the file can't be opened
	  	}
	  	fseek (fptr , 0 , SEEK_END);
	        file_size = ftell (fptr);
	  	fseek (fptr , 0 , SEEK_SET);
		rw_f = fread(arr1,1,MAXBUFSIZE,fptr);
		if(rw_f < 0)
		{
			perror("Reading of the file failed"); //File reading failed
		}
	  	ptr2 = strrchr(req_url,'.');
		strcpy(header_val,ptr2);
	  	if (strcmp(header_val,".html")==0) //for .html file
	    		ptr_data ="text/html";
	    	else if(strcmp(header_val,".txt")==0) //for .txt file 
	   		 ptr_data ="text/plain";
	   	else if(strcmp(header_val,".png")==0) //for .png file 
	    		ptr_data ="image/png";
	   	else if(strcmp(header_val,".gif")==0) //for gif file
	  		  ptr_data ="image/gif";
	    	else if(strcmp(header_val,".jpg")==0) //for jpg file
	   		 ptr_data ="image/jpg";
	   	else if(strcmp(header_val,".css")==0) //for css file
	    	 	ptr_data ="text/css";
	  	else if(strcmp(header_val,".js")==0) // for js file
	   		ptr_data ="application/javascript";
	  	else
	   	        internal_error(sock_data);
		sprintf(buff,ok_response,ptr_data,file_size);
	  	printf("The final header is %s\n",buff);
	  	header_bytes=write(sock_data, buff, strlen(buff));
	 	if (header_bytes<=0)
	  	{
	   		printf("header not Sent\n");
	  		shutdown_condition(sock_data); //shutdown condition
	 	}
	 	bzero(buff,sizeof(buff));
	  	file_write=write(sock_data, arr1, rw_f);
		if(file_write < 0)
		{
			perror("socket write failed"); 
		}
	  	fclose(fptr);
		printf("Client disconnected\n"); //client disconnection for timeout
	    	shutdown_condition(sock_data); //shuting down
		
	}
}
int main (int argc, char * argv[])
{
	  int sockfd, client_sock[256], clientlen = 0, serverlen = 0, val = 0, fork_child = 1, connection, new_val = 0, read_size = 0, old_val = 0;
	  uint8_t old_arr[BUFSIZE], new_arr[BUFSIZE];                          
	  struct sockaddr_in server,client;     

	 //socket creation
	  sockfd = socket(AF_INET, SOCK_STREAM,0);
	  if ((sockfd) == 0)
	  {
	   	perror("Socket can't be created");
	  }
	  puts("Socket created");
	  server.sin_family = AF_INET;                   
	  server.sin_port = htons(port);  //port      
	  server.sin_addr.s_addr = INADDR_ANY; 
	  serverlen = sizeof(server);
	  clientlen = sizeof(client);

	  //socket binding
	  if (bind(sockfd, (struct sockaddr *)&server, serverlen) < 0)
	  {
	   	 perror("error in binding socket");
	  }
	  puts("socket binding successful");
	  
	  //socket listening
	  if(listen (sockfd, 8) < 0) 
	  {
		perror("socket listening error");
	  }
	  puts("socket listening successful");
	  while(1)
	  {
	    	//accept connection
	    	client_sock[val] = accept (sockfd, (struct sockaddr *) &client, &clientlen);
	   	if (client_sock[val] < 0)
	    	 {
	     		perror("connection accept failed");
	      		return 1;
	   	 }
	   	puts("connection accepted"); //accepted connection
                connection = client_sock[val];
		read_size = recv(connection,old_arr,sizeof(old_arr), 0);
		if (read_size < 0)
		{
			perror("reading from socket error"); //socket read error
		}
		puts("socket read successfully");
		for(old_val = 0; old_val<read_size; old_val++)
		{
			if(old_arr[old_val]!='\0')
			{
				new_arr[new_val] = old_arr[old_val];
				new_val++;
			}
		}
		new_arr[new_val] = '\0';
		if((read_size == 0) || (read_size < 0)) //if the bytes obtained are zero or less than zero shutdown the connection
		{
			shutdown_condition(connection);
		}
		timeout.tv_sec = 10; //timeout for 10 seconds
		setsockopt(client_sock[val], SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
	     	fork_child=fork(); //fork used for handling multiple connections
	      	if(fork_child == 0)
	      	{
			printf("The child thread from a parent has been created\n");
			get_data(client_sock[val], read_size, old_arr, new_arr); //To process the data received 
			val++;
	       		exit(1);
	      	}
	   	val_check(val); //val check condition loop
	}
}

void val_check(int value)
{
	if(value%100 == 0)
	{
	     value=0;
	}
}

