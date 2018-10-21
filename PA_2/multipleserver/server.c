/*
    C socket server example, handles multiple clients using threads
*/
 
#include <stdio.h>
#include <string.h>    //strlen
#include <stdlib.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h> //for threading , link with lpthread
#include <stdint.h>
#include <stdint.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define portno (8772)
#define BUFSIZE (100)
uint8_t incorrect_req[] =
"HTTP/1.1 500 Internal Server Error\r\n"
"Content-Type: text/html; charset = UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<body><center><h1>ERROR 500: Internal Server Error</h1><br>\r\n";
//the thread function
void *connection_handler(void *);
int fork_condition = 1, val = 0;
 
int main(int argc , char *argv[])
{
    int socket_desc , client_sock[256] , c , *new_sock;
    pid_t fork_child;
    struct sockaddr_in server , client;
    uint8_t buffer_msg[256], buffer_obt[1024];
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( portno );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
   if(listen(socket_desc , 5) < 0)
   {
	perror("Error listening");

   }
   puts("listening successful");
   while(1) 
  { 
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    client_sock[val] = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    {
        puts("Connection accepted");
        if (client_sock[val]<0)
  	{
	  perror("ERROR on accept");
		close(client_sock[val]);
 	}

	 
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock[val];
         
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }
     
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    }
    return 0;
}
 
/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
        //Send the message back to client
        write(sock , client_message , strlen(client_message));
	
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    //Free the socket pointer
    free(socket_desc);
     
    return 0;
}
