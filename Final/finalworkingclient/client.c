#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <netdb.h>

#define MAXBUFSIZE (307200)
#define BUFSIZE (102400)
#define port (8270)
#define LENGTH (8250376)
 
void val_check (int value);
struct timeval timeout;

int main (int argc, char * argv[])
{
	  int sockfd, client_sock[256], clientlen = 0, serverlen = 0, val = 0, fork_child = 1, connection, new_val = 0, read_size = 0, old_val = 0,n=0;
	  uint8_t old_arr[BUFSIZE], new_arr[BUFSIZE];  
	  unsigned long int len_file=0;
	  char revbuf[len_file], fname1[100], cmd[100];  
          char* name_cmd;
          char* fname;                     
	  struct sockaddr_in server,client;    
	  struct hostent* server_hp; 
	  char arr[302400], valnow='1', rep[]= "Got it";
          FILE* fd;
          int size=0, recvsize=0, arrsize=0, readsize, writesize, data=1, num, check=0, arrfd, arrout;
          struct timeval time_out = {10,0};
          bzero(rep, sizeof(rep));
          bzero(arr,sizeof(arr));
		  
	  while(1)
	  {
           bzero(cmd, sizeof(cmd));
           bzero(fname1, sizeof(fname1));
	   if(argc < 3)
	   {
		printf("USAGE: <ip> <server_port>\n");
		exit(1);
	  }
	  server.sin_family = AF_INET;                   
	  server.sin_port = htons(atoi(argv[2]));        
	  //server.sin_addr.s_addr = INADDR_ANY; 
	  server.sin_addr.s_addr = inet_addr(argv[1]);
	  //inet_pton(AF_INET, "127.0.0.1", &server.sin_addr); 
	  bzero(&(server.sin_zero), 8);
	  serverlen = sizeof(server);
	  clientlen = sizeof(client);
		
	  server_hp = gethostbyname(argv[1]);
	  if(server_hp < 0)
	  {
		perror("Host unknown");
	  }
	  bcopy((char*)server_hp->h_addr,(char*)&server.sin_addr,server_hp->h_length);
	  //socket creation
	  sockfd = socket(AF_INET, SOCK_STREAM,0);
	  if ((sockfd) == 0)
	  {
	   	perror("Socket can't be created");
	  }
	  puts("Socket created");

	  if (connect(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	  {
		fprintf(stderr, "ERROR: Failed to connect to the host! (errno = %d)\n",errno);
		exit(1);
	  }
	  else 
	  {
		printf("[Client] Connected to server at port\n");
	  }
	  printf("Enter the [filename] to be obtained from the server\n");
          bzero(fname1,strlen(fname1));
          //fgets(fname1,98,stdin);
          gets(fname1);
	  printf(" Filename is %s\n", fname1);
          int optval;
          
          optval = write(sockfd,fname1, strlen(fname1));
	  if(optval <= 0)
	  {
		perror("error in sending filename to server\n");
	  }
          printf("%d", optval);
          //name_cmd = strdup(cmd);
          //strtok(name_cmd, " ");
          //fname = strtok(NULL, " ");
          printf("Filename:%s\n", fname1);
            printf("Get the file:%s from the server\n", fname1);
		/*read(sockfd, revbuf, sizeof(revbuf));
		if(!strcmp(revbuf, "Error"))
			{
				printf("File does not exist on the server. \n");
				continue;
		        }  */      

	  do
	  {
		num = read(sockfd, &size, sizeof(int));
	  } while(num < 0);
	  printf("packet size is %d\n",num);
	  printf("file size is %d\n", size);
          printf("\n");
	  do
	  {
		num = write(sockfd, &rep, sizeof(int));
	  } while( num < 0);
	  printf("\n");
	  fd = fopen(fname1,"wb");
	  if(fd == NULL)
	  {
		perror(" File is not found\n");
		exit(1);
	 }
	 fd_set fds;
         while(recvsize < size)
	 {
		FD_ZERO(&fds);
		FD_SET(sockfd, &fds);
	        arrfd = select(FD_SETSIZE, &fds, NULL, NULL, &time_out);
		if(arrfd < 0)
		{
			perror("file desc error\n");
                 }
		if(arrfd == 0)
		{
			perror(" buffer timeout has expired\n");
		}
		if(arrfd > 0)
		{
			do
			{
				readsize = read(sockfd, arr, 10241);
			} while( readsize < 0);
			printf("packetsize is %d\n", readsize);
			writesize = fwrite(arr,1,readsize,fd);
  			if(readsize != writesize)
			{
				perror("Error in the file written\n");
			}
                        //Increment the total number of bytes read
			recvsize = recvsize + readsize;
 			data++;
			printf("The size of file received in bytes is %d\n", recvsize);
			printf("\n");
			printf("\n");
		}
	}
	fclose(fd);
 	printf("file has been received\n");	

	}
          close(sockfd);
          return 0;
}
