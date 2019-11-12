#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <dirent.h>
#include <memory.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include <netdb.h>

#define MAXBUFSIZE (307200)
#define BUFSIZE (102400)
#define LENGTH (8250376)

void val_check (int value);

int main (int argc, char * argv[])
{
	  int sockfd, client_sock, clientlen = 0, serverlen = 0, val = 0, fork_child = 1, connection, new_val = 0, read_size = 0, old_val = 0, optval;
	  uint8_t old_arr[BUFSIZE], new_arr[BUFSIZE];                          
	  struct sockaddr_in server,client;     
	  struct hostent* server_hp;
	  unsigned long int len_file=0;
   	  char bf[100] = " ";
          char* cname;
          char* filename;
          char cmd[100], fname[100];
	  FILE* fd;
   	  char s[302400], r[256];
   	  int sendsize, readsize, num, data;
   	  data=1;          
	  bzero(cmd, sizeof(cmd));
  	  bzero(fname, sizeof(fname));	  
	  if(argc < 2)
	  {
		printf("USAGE: <client_port>\n");
		exit(1);
	  }
	 
	  bzero((char *) &server, sizeof(server));	  
	  server.sin_family = AF_INET;                   
	  server.sin_port = htons(atoi(argv[1]));
	  server.sin_addr.s_addr = INADDR_ANY;        
	  //server.sin_addr.s_addr = inet_addr(argv[1]); 
	  serverlen = sizeof(server);
	  clientlen = sizeof(client);
		
 	/* server_hp = gethostbyname(argv[1]);
	if(server_hp < 0)
	{
		perror("Host unknown");
	}*/
	//bcopy((char*)server_hp->h_addr, (char*)&server.sin_addr, server_hp->h_length);
	
	//socket creation
	  sockfd = socket(AF_INET, SOCK_STREAM,0);
	  if ((sockfd) == 0)
	  {
	   	perror("Socket can't be created");
	  }
	  puts("Socket created");
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
	    	client_sock = accept (sockfd, (struct sockaddr *) &client, &clientlen);
	   	if (client_sock < 0)
	    	 {
	     		perror("connection accept failed");
	      		return 1;
	   	 }
	   	puts("connection accepted");
				
		bzero(fname, sizeof(fname));
		optval = read(client_sock,fname, 99);
		printf("%d is optval\n", optval);
                if(optval==0)
		{
			continue;
                }
                if(optval < 0)
		{
			printf("error reading from the socket\n");
		}
	          
                //cname = strdup(bf);
                //strtok(cname, " ");
                //filename = strtok(NULL, " ");
                //printf("command received:%s\n", cname);
                printf("filename:%s", fname);	
	
   //bzero(fname, sizeof(fname));
   		fd= fopen(fname,"rb");
   	        if(fd == NULL)
   		{
			//strcpy(msg_val, "Error");
			perror("file not found error\n");
			//write(socketdata,msg_val,strlen(msg_val));
   		}
   		fseek(fd, 0, SEEK_END);
   		sendsize = ftell(fd);
   		fseek(fd, 0, SEEK_SET);
   		printf("File size is being sent now");
   		write(client_sock, (void*)&sendsize, sizeof(int));
                //Sending the file as an byte array
   		do
   		{
			num=read(client_sock, &r, 255);
			printf("The number of bytes read %d\n",num);
   		} while(num < 0);
   		while(!feof(fd))
   		{
			readsize= fread(s,1,sizeof(s)-1, fd);
        		do
			{
				num=write(client_sock,s,readsize);
			} while (num < 0);
			printf("Size of packet sent %d\n", readsize);
			printf("\n");
			printf("\n");
			data++;
        		bzero(s, sizeof(s));
   		}
             	printf("Ok sent to client!\n");
		val++;
	       	//exit(1);
	   	val_check(val);
	}
}

void val_check(int value)
{
	if(value%100 == 0)
	{
	     value=0;
	}
}

