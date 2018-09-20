/*
    C socket server example
*/
 
#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <stdlib.h>
#include <dirent.h>


int check_identity(int client_sock){
	char client_message[20];
	recv(client_sock , client_message , 20 , 0);	
	printf("client username: %s\n",client_message );
	recv(client_sock , client_message , 20 , 0);	
	printf("client password%s\n",client_message );
	return 0;

}

int findfilesize(char file_name[20]){
    int lsize=0;
    char path[100];
    printf("%s\n",file_name );
   FILE *filename;
   strcpy(path,"/home/sahana/NetworkSystems/DFS/Server4/");
   strcat(path,file_name);
   printf("%s\n",path );
   filename=fopen(path,"r+");
   if(filename==NULL){
   	perror("fopen failure\n");
   }
   if((fseek (filename,0,SEEK_END))){
   	perror("fseek error\n");
   }
   printf("seek file\n");
   lsize = ftell(filename);
   printf("size\n");
   rewind(filename);
   printf("rewind\n");
   printf("full size%d\n",lsize );
   fclose(filename);
   return lsize;
}

int main(int argc , char *argv[])
{
    int socket_desc , client_sock , c , read_size,packet_size,count=0;
    char size[10];
    struct sockaddr_in server , client;
    char client_message[22133];
    //FILE *fp;
     
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
    server.sin_port = htons( atoi(argv[1]) );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
        
    //Listen
    while(1){
    	FILE *file;
	    listen(socket_desc , 3);
	     
	    //Accept and incoming connection
	    puts("Waiting for incoming connections...");
	    c = sizeof(struct sockaddr_in);
	     
	    //accept connection from an incoming client
	    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	    if (client_sock < 0)
	    {
	        perror("accept failed");
	        return 1;
	    }
	    puts("Connection accepted");
	    /*******************Check Usernaem and Password*********************/
	    int ret = check_identity(client_sock);
	    read_size=recv(client_sock,client_message,20,0);
	    printf("command %s\n",client_message );
	    /******************PUT command**************************************/
	    if(strcmp(client_message,"PUT")==0){
		    //file = fopen("recieved1","w+");
		    while(count<2){
		    	if(count==0){
		    		char filename[20],path[20];
		    		strcpy(path,"/home/sahana/NetworkSystems/DFS/Server4/");
		    		read_size = recv(client_sock,filename,20,0);
		    		printf("filename is 4%s\n", filename);
		    		strcat(path,filename);
		    		file = fopen(path,"w+");
		    		if(read_size = recv(client_sock , size ,10, 0)<0){
				    	printf("recv error\n");
				    	exit(0);
			    	}
				    printf("size recieved%s\n",size );
				    count++;
				    printf("count %d\n",count );
				    packet_size=atoi(size);
				    read_size = recv(client_sock , client_message , packet_size , 0);
				   	printf("The actual data recieved in server 4 %d\n",read_size);
				    if(read_size=fwrite(client_message,packet_size,1,file)==0){
				    	printf("error write\n");
				    }
				    fclose(file);
			    }
		    	if(count == 1){
		    		char filename[20],path[20];
		    		strcpy(path,"/home/sahana/NetworkSystems/DFS/Server4/");
		    		read_size = recv(client_sock,filename,20,0);
		    		printf("filename 1 %s\n",filename );
		    		strcat(path,filename);
		    		file = fopen(path,"w+");
		    		if(read_size = recv(client_sock , size ,10, 0)<0){
				    	printf("recv error\n");
				    	exit(0);
				    }
				    printf("size recieved%s\n",size );
				    count++;
				    printf("count %d\n",count );
				    packet_size=atoi(size);
				    read_size = recv(client_sock , client_message , packet_size , 0);
				   	printf("The actual data recieved in server 4 %d\n",read_size);
				    fwrite(client_message,packet_size,1,file);
				    fclose(file);
		    	}
			    
			}
		}
		if(strcmp(client_message,"LIST")==0){
			/*Send the file names */
			printf("In list\n");
			int j=0;
			char sendserver[20];
			char *sendlist[20];
			char *filelist,*ret1,*ret2,*ret3,*ret4;
			DIR *dir;
			struct dirent *fir;
			dir=opendir("/home/sahana/NetworkSystems/DFS/Server4/");

			if(!dir){printf("opendir error");}
			while ((fir=readdir(dir))!=NULL){
				printf("%s\n",fir->d_name );
				strcpy(sendserver,fir->d_name);
				printf("%s\n",sendserver );
				ret1=strstr(sendserver,".1");
				ret2=strstr(sendserver,".2");
				ret3=strstr(sendserver,".3");
				ret4=strstr(sendserver,".4");
				if(ret1||ret2||ret3||ret4){
					printf("error4\n");
					if(send(client_sock,sendserver,20,0)<0){
						printf("Send error in filename\n");
					}
				}
				//read_size=send(client_sock,filelist,20,0);
			}
		}
	    
	    if(strcmp(client_message,"GET")==0){
			printf("HAve to send data\n");
			int size;
			DIR *dir;
			char sendserver[20],recvpart[20],filesize[20],path[100];
			char *ret1,*ret2,*ret3,*ret4;
			struct dirent *fir;
			dir=opendir("/home/sahana/NetworkSystems/DFS/Server4/");
			if(!dir){printf("opendir error\n");}
			while((fir=readdir(dir))!=NULL){
				strcpy(sendserver,fir->d_name);
				ret1=strstr(sendserver,".1");
				ret2=strstr(sendserver,".2");
				ret3=strstr(sendserver,".3");
				ret4=strstr(sendserver,".4");
				if(ret1||ret2||ret3||ret4){
					if(send(client_sock,sendserver,20,0)<0){
						printf("Send error in filename\n");
					}
					printf("%s\n",sendserver );
				}
			}
			if((recv(client_sock,recvpart,20,0))<0){
				printf("Send error in file\n");
			}
			printf("The needed file Part is %s\n",recvpart);
			size = findfilesize(recvpart);
			printf("size of the %d\n",size );
			sprintf(filesize,"%d",size);
			printf("%s\n",filesize );
			if(send(client_sock,filesize,20,0)<0){
				printf("Error in sending the size\n");
			}
			FILE *filecnt;
		    strcpy(path,"/home/sahana/NetworkSystems/DFS/Server4/");
		    strcat(path,recvpart);
		    printf("%s\n",path );
		    filecnt=fopen(path,"r+");
		    if(filecnt==NULL){
		   		perror("fopen failure\n");
		    }
			fread(sendserver,size,1,filecnt);
			if(send(client_sock,sendserver,size,0)<0){
				printf("Error in sending part\n");
			}
			fclose(filecnt);
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
    }
    return 0;
}
