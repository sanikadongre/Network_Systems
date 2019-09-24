/*

 * servercode.c

 *

 *  Created on: Aug 19, 2019

 *      Author: sdongre

 */


#include <stdio.h>

#include <stdlib.h>

#include <stdint.h>

#include <string.h>

#include <sys/types.h>

#include <sys/wait.h>

#include <sys/time.h>

#include <errno.h>

#include <signal.h>

#include <fcntl.h>

#include <time.h>

#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#include <unistd.h>

#include <memory.h>


#define MAXBUFSIZE (102400)

#define BUFSIZE (10000)


char incorr_req[] =

"HTTP/1.1 500 internal server error\r\n"

"Content-Type: text/html; charset = UTF-8\r\n\r\n"

"<!DOCTYPE html>\r\n"

"<body><center><h1>ERROR 500:INTERNAL SERVER </h1><br>\r\n";



void req_func(int bytes_received)

{

	int i=0,j=0, obt_bytes=0, data_send=0, bytesout, bytestot, bytestot1;

	FILE* fptr;

	size_t opsize, read_file;

	char newbuffer[MAXBUFSIZE],oldbuffer[MAXBUFSIZE], url_op[20], url_header[MAXBUFSIZE], full_message[MAXBUFSIZE];

	char* comp_type;

	char* data1;

	char* data2;

	char* data3;

	char* post_data;

	char* post_data1;

	char* post_data2;

	char* post_data3;

	char *position;


  	bytesout= bytes_received;

    bzero(newbuffer,MAXBUFSIZE);

    obt_bytes = recv(bytesout, newbuffer,MAXBUFSIZE,0);


    printf("length of bytes obtained is %d\n",obt_bytes);


    for(i=0;i<obt_bytes;i++)

    {

      if(newbuffer[i]!= '\0')

      {

         oldbuffer[j]=newbuffer[i];

         j++;

      }

    }

    oldbuffer[j]='\0';


    if(obt_bytes < 0)

    {

        perror("\n incorrect data received \n");

        shutdown(bytesout,SHUT_RDWR); //shutting down the connection

        close(bytesout);

    }

    else if(obt_bytes == 0)

    {

        perror("\n incorrect data received \n");

        shutdown(bytesout,SHUT_RDWR); //shutting down the connection

        close(bytesout);

    }

    else if(obt_bytes > 0)

    {

    	printf("\n The request received from client is %s\n",newbuffer);

		strncpy(full_message,oldbuffer,obt_bytes);

		bzero(newbuffer,MAXBUFSIZE);

		data1 = strtok(full_message," \n");

		data2 =strtok(NULL," \n");

		data3 =strtok(NULL," \n");

		printf("Command is %s\n",data1);

		printf("Request is %s\n",data2);

		printf("Protocol is %s\n",data3);

   }


   //GET REQUEST


   if((strncmp(data1,"GET\0",4)==0)||(strncmp(data1,"POST\0",5)==0))

   {

	  printf("GET or the POST request is received\n");

   }

   else

   {


	  perror("\n The request isn't valid \n");

	  bytestot1 = send(bytesout,incorr_req, strlen(incorr_req), 0);

	  shutdown(bytesout, SHUT_RDWR);  //shutting down the connection

	  close(bytesout);  //close the connection

   }


   //HTTP request


   if((strncmp(data3,"HTTP/1.1",8)==0) || (strncmp(data3,"HTTP/1.0",8)==0))

   {

	  	printf("\n The request version 1.0 or the request version 1.1 is received \n");

   }

   else

   {

		printf("\n The HTTP version is invalid \n");

		bytestot1 = send(bytesout,incorr_req, strlen(incorr_req), 0);

		shutdown(bytesout, SHUT_RDWR); //shutting down the connection

		close(bytesout); // close the connection

   }


   //GET request



   if(strncmp(data1,"GET\0",4)==0)

   {

	    char url_loc[MAXBUFSIZE] = "/home/sanika/tcp_serv/PA2/www";

		strcat(url_loc,data2);

		printf("\n Request is %s\n",url_loc);

		fptr =fopen(url_loc,"r");


	    if(fptr==NULL)

	    {

			printf("\n The Request isn't valid \n");

			bytestot1 = send(bytesout,incorr_req, strlen(incorr_req), 0);

			shutdown(bytesout, SHUT_RDWR); //shutting down the connection

			close(bytesout); //close the connection

	    }

	    else

	    {

	   	printf("The file has been created \n");

	    }

	    fseek (fptr , 0 , SEEK_END);

		opsize = ftell(fptr);

		fseek (fptr, 0 , SEEK_SET);

		position = strrchr(data2,'.');

		strcpy(url_op,position);


		if(strcmp(url_op,".html")==0)

		{

			comp_type ="text/html";

		}

		else if(strcmp(url_op,".png")==0)

		{

			comp_type ="image/png";

		}

		else if(strcmp(url_op,".txt")==0)

		{

			comp_type ="text/plain";

		}

		else if(strcmp(url_op,".css")==0)

		{

			comp_type ="text/css";

		}

		else if(strcmp(url_op,".gif")==0)

		{

			comp_type ="image/gif";

	    }


	    else if(strcmp(url_op,".jpg")==0)

	    {

	     	comp_type ="image/jpg";

	    }

	    else if(strcmp(url_op,".js")==0)

	    {

	   	comp_type ="application/javascript";


	    }

	    else

	    {

			printf("\n The request isn't valid \n");

			bytestot1 = send(bytesout,incorr_req, strlen(incorr_req), 0);

			shutdown(bytesout, SHUT_RDWR); //shutting down the connection

			close(bytesout);

	    }

	    sprintf(url_header,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n",comp_type,opsize);

	    printf("Final Header is %s\n",url_header);

	    data_send =write(bytesout,url_header, strlen(url_header));

	    if(data_send <=0)

	    {

	   	printf("Header not Sent\n");

	   	shutdown(bytesout,SHUT_RDWR); //shutting down the connection

		    close(bytesout); //close the connection

	    }

		else

		{

			printf("Header Sent\n");

		}

	    bzero(url_header,sizeof(url_header));

	    read_file = fread(newbuffer,1,MAXBUFSIZE, fptr);//read the requested file

	    printf("file read is %ld\n", read_file);

	    bytestot =write(bytesout, newbuffer, read_file);//write the file over connection to client

	    printf("sent file as %d\n",bytestot);

	    bzero(newbuffer,sizeof(newbuffer));

	    fclose(fptr);

	    shutdown(bytesout,SHUT_RDWR);

	    close(bytesout);

   	}


   	if(strncmp(data1,"POST\0",5)==0)

     {

 		bzero(newbuffer,sizeof(newbuffer));

 		post_data =strtok(oldbuffer,"\n");

 		post_data1 =strtok(NULL,"\n\n");

 		post_data2 =strtok(NULL,"\r\n");

 		post_data3 =strtok(NULL,"\r\n");

 		printf("\nPost Data is:\n%s\n",post_data3);

 		char *content="text/html";

 		char header1[MAXBUFSIZE];

 		sprintf(header1,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n<html><body><pre><h1>%s</h1></pre>\r\n",content,strlen(post_data3),post_data3);

 		printf("\n Post Header: \n %s",header1);

 		char dir1[MAXBUFSIZE] = "/home/sanika/tcp_serv/PA2/www";

 		fptr =fopen(dir1,"r");

 		if (fptr == NULL)

 		{

 			printf("\n File request isn't valid \n");

 			bytestot1 = send(bytesout,incorr_req, strlen(incorr_req), 0);

 			shutdown(bytesout, SHUT_RDWR);

 			close(bytesout);
 		}

         else

         {

        	printf("Index File Created\n");

         }

		read_file = fread(newbuffer,1,MAXBUFSIZE, fptr);

		bytestot=write(bytesout, newbuffer, read_file);

		printf("Index File Sent");

		bzero(newbuffer,sizeof(newbuffer));

		fclose(fptr);

     }



}


int main (int argc, char* argv[])

{

  int sockfd, conn_acc[10];

  struct sockaddr_in sin,client_addr;

  socklen_t client_length;

  int child_thread;

  int connfd=0;


  if (argc != 2)

  {

      printf("Incorrect usage: USAGE:  <port>\n");

      exit(1);

  }

  bzero(&sin,sizeof(sin));

  sin.sin_family = AF_INET;

  sin.sin_port = htons(atoi(argv[1]));

  sin.sin_addr.s_addr = INADDR_ANY;


  //socket creation

  if ((sockfd = socket(AF_INET, SOCK_STREAM,0))< 0)

  {

	  perror("socket creation failed\n");

  }

  puts("socket created successfully\n");


  //socket bind

  if(bind(sockfd, (struct sockaddr *)&sin, sizeof(sin))< 0)

  {

	  perror("socket bind failed\n");

  }

  puts("socket binded successfully\n");


  //listen

  listen (sockfd, 8);

  printf("The server is waiting for connection\n");

  client_length = sizeof(client_addr);


  while(1)

  {

	//socket connection accept

    conn_acc[connfd] = accept (sockfd, (struct sockaddr *) &client_addr, &client_length);

    if (conn_acc[connfd] < 0)

    {

    	perror("The connection accept failed\n");

    }

    else

    {

      printf("The connection is being accepted by the server\n");


      //child thread via fork

      child_thread= fork();


      if(child_thread==0)

      {

        printf("The child thread has been created\n");

        req_func(conn_acc[connfd]);//calling the function1

        connfd++;

        exit(1);

      }

      if(connfd==10)

      {

    	  connfd=0;

      }

    }

  }

}



	

