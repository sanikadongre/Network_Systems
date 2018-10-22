
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define port 8121
#define MAXBUFSIZE 102400
#define MAXBUFSIZE1 10000

char bad_request[] =
"HTTP/1.1 500 internal server error\r\n"
"Content-Type: text/html; charset = UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<body><center><h1>ERROR 500:INTERNAL SERVER </h1><br>\r\n";
//function declaration for function1
void function1(int func_var);


int main (int argc, char * argv[])
{
 	int sockfd, a, client_sock[256], n, serverlen = 0, clientlen = 0, val = 0, fork_child = 1;                           
 	struct sockaddr_in server,client;    
 	unsigned int remote_length;        

  

 	//Socket creation
 	sockfd = socket(AF_INET, SOCK_STREAM, 0);
 	if ((sockfd) == 0)
 	{
    		perror("Socket opening error");
 	}
  	puts("socket created successfully");

  	server.sin_family = AF_INET;   
  	server.sin_addr.s_addr = INADDR_ANY;               
  	server.sin_port = htons(port);        
  	serverlen = sizeof(server);
  	clientlen= sizeof(client);

  	//Socket binding
  	if (bind(sockfd, (struct sockaddr *)&server, serverlen) < 0)
  	{
   		 perror("Socket binding error");
  	}
  	puts("socket binding success");
 
  	if(listen(sockfd, 8) < 0)
  	{
		perror("listen error");
  	}
  	puts("listening success");
  
  	while(1)
  	{
   	 //accepting connection
    		client_sock[val] = accept (sockfd, (struct sockaddr *) &client, &clientlen);
 		if (client_sock[val] < 0)
	    	{
	     		 perror("connection accept error");
	     		close(client_sock[val]);
	    	}
		puts("connection accepted");
		fork_child = fork();
	      	if(fork_child == 0)
	      	{
			function1(client_sock[val]);
			val++;
	       		exit(1);
		}
	   		
	   	 if(val==10)
	    	{
	    		 val=0;
	    	}
  	}
}
//function1 is used to recieve command and send appropiate header and file content
void function1(int func_var)
{
  //local variable
  char buffer[MAXBUFSIZE],buffer0[MAXBUFSIZE];             //a buffer to store our received message
  size_t var1;
  char full_message[MAXBUFSIZE];
  char* first;
  char* second;
  char* third;
  char* postdata;
  char* postdata1;
  int conn;
  char* postdata2;
  char* postdata3;
  char* postdata4;

  int recv_bytes=0;
  FILE *pFile;
  int send_bytes=0;
  size_t lSize;
  char head[MAXBUFSIZE];
  char *position;
  char extension[20];
  char *exten;
  int nbytes,nbytes1;
  conn=func_var;                     //number of bytes we receive in our message
  //printf("conection in func%d\n",conn);
  bzero(buffer,MAXBUFSIZE);
  recv_bytes = recv(conn, buffer,MAXBUFSIZE,0);
  printf("length %d \n",recv_bytes);

  int j=0,i=0;
  for(i=0;i<recv_bytes;i++)
  {
    if(buffer[i] !='\0')
    {
      buffer0[j]=buffer[i];
      j++;
    }
  }
  buffer0[j]='\0';


  //printf("cn func%d\n",recv_bytes);
  if(recv_bytes < 0)
  {
    printf("Error recieving I \n");
    shutdown(conn,SHUT_RDWR);
    close(conn);
    return;
  }
  else if(recv_bytes == 0)
  {
    printf("Error Recieving II \n");
    shutdown(conn,SHUT_RDWR);
    close(conn);
    return;
  }
  else if(recv_bytes > 0)
  {
    printf("%s","String received from client:");
    printf("\n%s\n",buffer);//command recieved from client
    strncpy(full_message,buffer,recv_bytes);
    bzero(buffer,MAXBUFSIZE);
    first =strtok(full_message," \n");//saving command in first variable
    second =strtok(NULL," \n");//saving file type in second variable
    third =strtok(NULL," \n");//saving protocol in third variable
    printf("Command is %s\n",first);
    printf("Request is %s\n",second);
    printf("Protocol is %s\n",third);
  }
  if((strncmp(first,"GET\0",4)==0)||(strncmp(first,"POST\0",4)==0))
  {
      printf("GET Recieved\n");
  }
  else
  {//in case of bad command, 500 INTERNAL ERRORis sent to client and displayed on console
    printf("\nERROR: Invalid Command Request Recieved\n");
    nbytes1 = send(conn,bad_request, strlen(bad_request), 0);
    shutdown(conn, SHUT_RDWR);//shutdown the port
    close(conn);//close the listenning connection
    return;
  }
  if((strncmp(third,"HTTP/1.1",8)==0) || (strncmp(third,"HTTP/1.0",8)==0))
  {
    printf("Received %s\n",third);
  }
  else
  {
    printf("\nERROR: Invalid Protocol Request Recieved\n");
    nbytes1 = send(conn,bad_request, strlen(bad_request), 0);
    shutdown(conn, SHUT_RDWR);
    close(conn);
    return;
  }
  if(strncmp(first,"POST\0",5)==0)
  {
    int postdatalength=0;
    bzero(buffer,sizeof(buffer));
    //printf("\nPost wala data :\n%s\n",buffer0);//command recieved from client
    postdata =strtok(buffer0,"\n");//saving protocol in third variable
    //printf("\npost data isisisis :\n%s\n",postdata);//command recieved from client
    postdata1 =strtok(NULL,"\n\n");//saving protocol in third variable
    //printf("\npost data isisisis :\n%s\n",postdata1);//command recieved from client
    postdata2 =strtok(NULL,"\r\n");//saving protocol in third variable
    //printf("\npost data isisisis :\n%s\n",postdata2);//command recieved from client
    postdata3 =strtok(NULL,"\r\n");//saving protocol in third variable
    printf("\nPost Data is:\n%s\n",postdata3);//command recieved from client
    char *content="text/html";
    char header1[MAXBUFSIZE];
    //printf("\n%ld\n",strlen(postdata3));
    sprintf(header1,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n<html><body><pre><h1>%s</h1></pre>\r\n",content,strlen(postdata3),postdata3);
    printf("\nPost Header: \n %s",header1);
    char dir1[MAXBUFSIZE] = "www/index.html";
    pFile =fopen(dir1,"r");
    if (pFile==NULL)
    {
      printf("\nERROR: Invalid File Request Recieved\n");
      nbytes1 = send(conn,bad_request, strlen(bad_request), 0);
      shutdown(conn, SHUT_RDWR);
      close(conn);
      return;
    }
    else
    {
      printf("Index File Created\n");
    }
    var1 = fread(buffer,1,MAXBUFSIZE,pFile);
    //printf("file read is %ld\n",var1);
    nbytes=write(conn, buffer, var1);
    printf("Index File Sent");
    //printf("sent file as %d\n",nbytes);
    bzero(buffer,sizeof(buffer));
    fclose(pFile);
  }

  if(strncmp(first,"GET\0",4)==0)
  {
  char dir[MAXBUFSIZE] = "www";
  strcat(dir,second);
  printf("\nRequest is %s\n",dir);
  pFile =fopen(dir,"r");
  if (pFile==NULL)
  {
    printf("\nERROR: Invalid File Request Recieved\n");
    nbytes1 = send(conn,bad_request, strlen(bad_request), 0);
    shutdown(conn, SHUT_RDWR);
    close(conn);
    return;
  }
  else
  {
    printf("File Created\n");
  }
  fseek (pFile , 0 , SEEK_END);
  lSize = ftell (pFile);
  fseek (pFile , 0 , SEEK_SET);
  //printf("size of file sent is %ld\n",lSize);
  position = strrchr(second,'.');
  strcpy(extension,position);
  if (strcmp(extension,".html")==0)
  {
    exten ="text/html";
    //printf("text is %s",exten);
  }
  else if(strcmp(extension,".txt")==0)
  {
    exten ="text/plain";
    //printf("plain is %s",exten);
  }
  else if(strcmp(extension,".png")==0)
  {
    exten ="image/png";
    //printf("png is %s",exten);
  }
  else if(strcmp(extension,".gif")==0)
  {
    exten ="image/gif";
    //printf("gif is %s",exten);
  }
  else if(strcmp(extension,".jpg")==0)
  {
    exten ="image/jpg";
    //printf("jpg is %s",exten);
  }
  else if(strcmp(extension,".css")==0)
  {
    exten ="text/css";
    //printf("css is %s",exten);
  }
  else if(strcmp(extension,".js")==0)
  {
    exten ="application/javascript";
    //printf("java is %s",exten);
  }
  else
  {
    printf("\nERROR: Invalid Type Request Recieved\n");
    nbytes1 = send(conn,bad_request, strlen(bad_request), 0);
    shutdown(conn, SHUT_RDWR);
    close(conn);
    return;
  }
  sprintf(head,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n",exten,lSize);
  printf("Final Header is %s\n",head);
  //printf("con value is here %d\n",conn);
  send_bytes=write(conn, head, strlen(head));
  if (send_bytes<=0)
  {
    printf("Header not Sent\n");
    //printf("header is %d\n",send_bytes);
    shutdown(conn,SHUT_RDWR);
    close(conn);
    return;
  }
  else
  {
    printf("Header Sent\n");
    //printf("header is %d\n",send_bytes);
  }
  bzero(head,sizeof(head));
  var1 = fread(buffer,1,MAXBUFSIZE,pFile);
  //printf("file read is %ld\n",var1);
  nbytes=write(conn, buffer, var1);
  //printf("sent file as %d\n",nbytes);
  bzero(buffer,sizeof(buffer));
  fclose(pFile);
  shutdown(conn,SHUT_RDWR);
  close(conn);
}
}
