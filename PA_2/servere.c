#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<stdbool.h>
#include<signal.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>

#define PORTNUMBER 9889
#define MAXBUFFSIZE 1000

char *root_dir = "/home/Network_Systems/PA_2/www";
struct sockaddr_in client_address;

int listen_fd = 0;

char bad_request[] = 
"HTTP/1.1 500 internal server error\r\n"
"Content-Type: text/html; charset = UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<body><center><h1>ERROR 500: internal server error: %s </h1><br>\r\n";
 
void recieve_function(int result_client);

int main()
{

  int result_client[5];
  int addr_length;
  struct sockaddr_in server;
  bzero((char *)&server, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(PORTNUMBER);
  
  if((listen_fd = socket(AF_INET,SOCK_STREAM, 0)) < 0)
  {
    printf("ERROR: CREATING SOCKET\n");
    exit(1);
  }

  /*check if bind is success*/
  if((bind(listen_fd,(struct sockaddr *)&server,sizeof(server))) < 0)
  {
    printf("ERROR : BINDING\n");
    exit(1);
  }

  /*listen for incoming connections*/
  if(listen(listen_fd,1000000) != 0)
  {
    perror("ERROR: listen()");
    exit(1);
  }

  addr_length = sizeof(client_address);
  int i = 0;
  int child_pid = 0;
  while(1)
  {
    printf("\nWaiting for accept command\n");
    result_client[i] = accept(listen_fd,(struct sockaddr*)&client_address , &addr_length);
    if(result_client[i] < 0)
    {
      printf("\nError in accept\n");
      exit(1);
    }
    else
    {
      child_pid = fork();
      if(child_pid == 0)
      {
        recieve_function(result_client[i]);
        exit(1);
      }
    }
    i++;
    i = i%4;
  }

}

void recieve_function(int result_client)
{
  char send_buffer[MAXBUFFSIZE];
  char recieve_buffer[MAXBUFFSIZE];
  int nbytes_recieve = 0, nbytes_send = 0;
  char complete_message[MAXBUFFSIZE];
  char *command;
  char *protocol;
  char *protocol_type,*connection_status;
  char *test1,*test2;
  char *recieve;
  char root_directory[100];
  char file_path[MAXBUFFSIZE];
  char header[MAXBUFFSIZE];
  FILE *file_ptr;
  char content_type[MAXBUFFSIZE];
  int file_size = 0;
  char test_buffer[MAXBUFFSIZE];
  strcpy(root_directory,root_dir); 
  bzero(send_buffer,MAXBUFFSIZE);
  bzero(recieve_buffer,MAXBUFFSIZE);
  bzero(complete_message,MAXBUFFSIZE);
  nbytes_recieve = read(result_client,recieve_buffer,MAXBUFFSIZE);
  printf("\n:%d\n",nbytes_recieve);
  if(nbytes_recieve < 0)
  {
    printf("\nError while recieving command\n");
    shutdown(result_client, SHUT_RDWR);
    close(result_client);
    return;
  }
  else if(nbytes_recieve == 0)
  {
    printf("\nNo command recieved\n");
    shutdown(result_client, SHUT_RDWR);
    close(result_client);
    return;
  }
  else
  {
    int j = 0;
    for(int i = 0; i<nbytes_recieve; i++)
    {
      if(recieve_buffer[i] != '\0')
      {
        test_buffer[j]=recieve_buffer[i];
        j++;
      }
    }
    test_buffer[j] = '\0';
    printf("\nrecieve message is: %s\n",recieve_buffer);
    memcpy(complete_message,recieve_buffer,nbytes_recieve);
    printf("complete message is: %s\n", test_buffer);
    command = strtok(complete_message," \n");
    protocol_type = strtok(NULL," \n");
    protocol = strtok(NULL, " \n");
    test1 = strstr(test_buffer,"Connection:");
    connection_status=strtok(test1, "\n");
    printf("Test 1 value is: %s\n",connection_status);
  }
  if(strncmp(command,"GET\0",4)==0 || strncmp(command,"POST\0",5)==0)
  {
    printf("Command recieved is: %s\n",command);
    //printf("protocol_value value is: %s\n",protocol_type); 
  }
  else
  {
    printf("\nInvalid command recieved\n");
    sprintf(send_buffer,bad_request,"Invalid request");
    nbytes_send = send(result_client, send_buffer, strlen(send_buffer), 0);
    shutdown(result_client, SHUT_RDWR);
    close(result_client);
    return;
  }
  if((strncmp(protocol,"HTTP/1.1",8)==0) || (strncmp(protocol,"HTTP/1.0",8)==0))
  { 
    printf("protocol value is: %s\n",protocol);
  }
  else
  {
    printf("\nInvalid command recieved\n");
    sprintf(send_buffer,bad_request,"Invalid protocol");
    nbytes_send = send(result_client, send_buffer, strlen(send_buffer), 0);
    shutdown(result_client, SHUT_RDWR);
    close(result_client);
    return;
  }
  if(strncmp(protocol_type,"/index.html",11)==0 || strncmp(protocol_type,"/images/",8)==0 || strncmp(protocol_type,"/fancybox",9)==0 || strncmp(protocol_type,"/css",4)==0 || strncmp(protocol_type,"/files",5)==0 || strncmp(protocol_type,"/graphics",9)==0) 
  {
    printf("protoco_type value is:%s\n",protocol_type);
    if(strncmp(protocol_type,"/fancybox",9)==0)
    {
      strcpy(content_type,"text/css");
      printf("\nentered fancybox folder\n");
    }
    else if(strncmp(protocol_type,"/index.html",11)==0)
    {
      strcpy(content_type,"text/html");
      printf("\nentered html folder\n");
    }
    else if(strncmp(protocol_type,"/images/",8)==0)
    {
      char *content=strchr(protocol_type,'.');
      strcpy(content_type,"image/");
      strcat(content_type,&content[1]);
      printf("entered image loop:%s",content_type);
    }
    else if(strncmp(protocol_type,"/graphics",9)==0)
    {
      char *content=strchr(protocol_type,'.');
      strcpy(content_type,"graphics/");
      strcat(content_type,&content[1]);
      printf("entered graphics loop:%s",content_type);
    }
    else if(strncmp(protocol_type,"/files",6)==0)
    {
      char *content=strchr(protocol_type,'.');
      strcpy(content_type,"files/");
      strcat(content_type,&content[1]);
      printf("entered files loop:%s",content);
      if(strncmp(content,".txt",4)==0)
      {
        strcpy(content_type,"text/plain");
      }
    }
    else
    {
      strcpy(content_type,".html");
      printf("\nentered else loop\n");
    }
  }
  else
  {
    printf("\nInvalid command recieved\n");
    sprintf(send_buffer,bad_request,"Invalid protocol type");
    nbytes_send = send(result_client, send_buffer, strlen(send_buffer), 0);
    shutdown(result_client, SHUT_RDWR);
    close(result_client);
    return;
  }
  if(strncmp(command,"POST\0",5)==0)
  {
    printf("\nEntered post condition\n");
    char *data_string;
    int forward_value=strlen(connection_status);
    //data_string = strtok(test1,"\n");
    data_string = test1 + (forward_value+4);
    //data_string = strtok(NULL,"A");
    //data_string = strtok(NULL,"\n");
    //data_string = data_string + 1;
    printf("\nData string value is:%s\n",data_string);
    file_size = strlen(data_string);
    sprintf(header,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n<html><body><pre><h1>%s</h1></pre>\r\n", content_type,file_size,(data_string));
     printf("Header value sent is:%s\n",header);
     printf("\nData string length is:%d\n",file_size); 
     send(result_client,header,strlen(header),0);
  }

  strcpy(file_path, root_directory);
  int path_size = strlen(file_path);
  strcpy(&file_path[path_size],protocol_type);
  printf("\nFile path is: %s\n",file_path);
  if((file_ptr = fopen(file_path,"r")) != NULL)
  {
    fseek(file_ptr,0,SEEK_END);
    file_size = ftell(file_ptr);
    fseek(file_ptr,0,SEEK_SET);
    if(strncmp(command,"GET\0",4)==0)
    {
      sprintf(header,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %d\r\n\r\n", content_type,file_size);
      printf("\nHeader value is : %s\n",header);
      send(result_client,header,strlen(header),0);
    }
    bzero(send_buffer,MAXBUFFSIZE);
    while((nbytes_recieve = fread(send_buffer,1,MAXBUFFSIZE,file_ptr)) > 0)
        {
          nbytes_send = write(result_client,send_buffer,nbytes_recieve);
          printf("Number of bytes sent: %d\n",nbytes_send);
        }
    fclose(file_ptr);
    printf("\nFile closed\n");
    shutdown(result_client, SHUT_RDWR);
    close(result_client);
    return;
  }
  else
  {
    printf("\nFile not found\n");
    sprintf(send_buffer,bad_request,"File not found");
    nbytes_send = send(result_client, send_buffer, strlen(send_buffer), 0);
    shutdown(result_client, SHUT_RDWR);
    close(result_client);
    return;
  }

}
