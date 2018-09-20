/*
    C ECHO client example using sockets
*/
#include <stdio.h> //printf
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include "client.h"

//global//
char *parts[100];
char *broken;
char store[8][MAXLIST];
int array[20];
int j=0,n=1;
int create_sock(int n,char* port){
    printf("value of port connecting%s\n",port );
    int sock[4];
    struct sockaddr_in server;
    
     
    sock[n] = socket(AF_INET , SOCK_STREAM , 0);
    if (sock[n] == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( atoi(port) );
   
    //Connect to remote server
    if (connect(sock[n] , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        printf("%s\n",port );
        return 0;
    }
    puts("Connected\n");
    printf("port that was connected %s\n",port);
    return sock[n];

}

int findfilesize(char file_name[20]){
    int lsize=0;
   FILE *filename=fopen(file_name,"r");
   fseek (filename,0,SEEK_END);
   lsize = ftell(filename);
   rewind(filename);
   printf("full size%d\n",lsize );
   return lsize;
}

check_t getcommand(char filename[20],char command[20]){
    printf("Please enter the command\n");
    scanf("%s",command);
    int ret=strcmp(command,"PUT");
    if(ret == 0){
        scanf("%s",filename);
        printf("%s\n",filename );
    }
    ret = strcmp(command,"GET");
    if(ret == 0){
        scanf("%s",filename);
        printf("%s\n",filename );
    }
    return SUCCESS;
}

check_t check_identity(int sock){
    char conf_file[200],username[20],password[20];
    int size,i=0;
    FILE* conf=fopen("dfc.conf","r");
    size=findfilesize("dfc.conf");
    fread(conf_file,size,1,conf);
    broken=strtok(conf_file,"\n");
    parts[0]=broken;
    while(broken != NULL){
        broken=strtok(NULL,"\n");
        printf("%s\n",parts[i] );
        i++; 
        parts[i]=broken;

    }
    parts[4]=strtok(parts[4]," ");
    printf("%s\n",parts[4] );
    while(parts[4]!=NULL){
        parts[4]=strtok(NULL," \n");  
        parts[i]=parts[4]; 
        i++;
    }
    parts[5]=strtok(parts[5]," ");
    while(parts[5]!=NULL){
        parts[5]=strtok(NULL," \n");
        parts[i]=parts[5];
        i++;
    }
    printf("%s\n",parts[7] );
    printf("%s\n",parts[9] );
    fclose(conf); 
    if(send(sock,parts[7],20,0)<0){
        perror("Error in sending username\n");
    }
    if(send(sock,parts[9],20,0)<0){
        perror("Error in sending username\n");
    }
    return SUCCESS;
}

check_t send_command(int sock,char command[20]){
    printf("command is%s.\n",command );
    if(send(sock,command,20,0)<0){
        perror("Error in sending command\n");
    }
    return SUCCESS;
}

check_t send_filename(int sock, char filename[20],char number[2]){
    char serverfilename[20];
    strcpy(serverfilename,filename);
    strcat(serverfilename,number);
    printf("%s\n",serverfilename);
    if(send(sock,serverfilename,20,0)<0){
        printf("Error in sending the name\n");
    }
    return SUCCESS;
}



check_t enquire_filelist(int sock){
    char server_message[20];
    int recv_byte,i=0,send_byte;
    printf("%d\n",sock );
    while(i<2){
        usleep(2);
        if(recv_byte=recv(sock,server_message,20,0)<0){
            printf("recv error in file parts name\n");
        }
        i++;
        printf("%s\n",server_message );
        if(strstr(server_message,".1")){
            array[j]=1;
            j++;
        }
        if(strstr(server_message,".2")){
            array[j]=2;
            j++;
        }
        if(strstr(server_message,".3")){
            array[j]=3;
            j++;
        }
        if(strstr(server_message,".4")){
            array[j]=4;
            j++;
        }
    }
    return SUCCESS;
}

check_t store_list(int sock){
    char recvlist[20];
    int recv_byte;
    int loop=0;
    while(loop<2){
        if(recv_byte=recv(sock,recvlist,20,0)<0){
            printf("recv error in file parts name\n");
        }
        if(loop==0){
            strcpy(store[n-1],recvlist);
            printf("%d %s\n",n-1,store[n-1] );
        }
        if(loop==1){
            strcpy(store[n],recvlist);
            printf("%d %s\n",n,store[n] );
        }

        loop++;
    }
    
    //printf("%d\n",n );
    printf("2nd %s\n",store[0] );
    printf("1st%s\n",store[n-1] );
    n=n+2;
}

int main(int argc , char *argv[])
{
    int sock[4];
    int count1=0,count2=0,count3=0,count4=0;
    char message[100000] , server_reply[2000],command[20],filename[20],size[10];
    FILE *fp; 
    int size_divide,count=0,fullsize,packet_size,lsize=0,sizesent;

    char* port1=argv[1];
    char* port2=argv[2];
    char* port3= argv[3];
    char* port4= argv[4];
    sock[0]=create_sock(0,port1);    
    sock[1]=create_sock(1,port2);
    sock[2]=create_sock(2,port3);    
    sock[3]=create_sock(3,port4);
    
    int ret=check_identity(sock[0]);
    ret=check_identity(sock[1]);
    ret=check_identity(sock[2]);
    ret=check_identity(sock[3]);

    //printf("%d\n",ret );
    printf("got in main\n");
    getcommand(filename,command);

    send_command(sock[0],command);
    send_command(sock[1],command);
    send_command(sock[2],command);
    send_command(sock[3],command);
    
    if(strcmp(command,"PUT")==0){
        fp = fopen(filename,"r");
        if(fp==NULL){
        	perror("file open error\n");
        	exit(0);
        }
        lsize=findfilesize(filename);
        size_divide=lsize/4;
        printf("%d\n",size_divide );
        packet_size=size_divide;
        
        while(fullsize<lsize){
        	printf("I sent %d\n",count );
        	if(count>2){
            	size_divide=lsize-3*(size_divide);
                packet_size=size_divide;
                printf("%d\n",size_divide);
            }
        	fread(message,packet_size,1,fp);
        //Send some data
            sizesent=size_divide;
            sprintf(size,"%d",sizesent);

            if(count==0||count ==1){
                //Send filename
                if(count == 0){
                    send_filename(sock[0],filename,".1");
                }
                if(count == 1){
                    send_filename(sock[0],filename,".2");
                }
                if(send(sock[0] ,size ,10 , 0)<0){
                    printf("Error in send in 0 nd 1\n");
                    exit(0);
                }
                printf("size sent %s\n",size );   
                if( send(sock[0] , message ,size_divide , 0) < 0)
                {
                    puts("Send failed");
                    return 1;
                }
            }

            if(count==3||count==0){
                if(count == 3){
                    send_filename(sock[3],filename,".4");
                }
                if(count == 0){
                    send_filename(sock[3],filename,".1");
                }
               if(send(sock[3] ,size ,10 , 0)<0){
                printf("Error in send in 3 and 0\n");
                exit(0);
                }
                printf("size sent %s\n",size );
                
                
                if( send(sock[3] , message ,size_divide , 0) < 0)
                {
                    puts("Send failed");
                    return 1;
                } 
            }

            if(count==1||count==2){
                if(count == 1){
                    send_filename(sock[1],filename,".2");
                }
                if(count == 2){
                    send_filename(sock[1],filename,".3");
                }
                if(send(sock[1] ,size ,10 , 0)<0){
                printf("Error in send in 1 nd 2\n");
                exit(0);
                }
                printf("size sent %s\n",size );
                
                
                if( send(sock[1] , message ,size_divide , 0) < 0)
                {
                    puts("Send failed");
                    return 1;
                }    
            }
            if(count==2||count==3){
                if(count == 2){
                    send_filename(sock[2],filename,".3");
                }
                if(count == 3){
                    send_filename(sock[2],filename,".4");
                }
               if(send(sock[2] ,size ,10 , 0)<0){
                printf("Error in send in 3 nd 2\n");
                exit(0);
                }
                printf("size sent %s\n",size );
                
                
                if( send(sock[2] , message ,size_divide , 0) < 0)
                {
                    puts("Send failed");
                    return 1;
                } 
            }
            
            //fprintf(stdout, "%s\n",message );
            fullsize+=packet_size;
            printf("I sent %d\n",fullsize );
            count +=1;
        }
    }



    if(strcmp(command,"GET")==0){
        printf("requesting servers.................\n");
        int k=0,count1=0,count2=0,count3=0,count4=0,k1,k2,k3;
        FILE *recreate;
        char serversize[20],serverfile[1000];
        printf("%d\n",sock[0] );
        store_list(sock[0]);
        printf("%d\n",sock[0] );
        printf("Server 1 done\n");
        store_list(sock[1]);
        printf("Server 2 done\n");
        store_list(sock[2]);
        printf("Server 3 done\n");
        store_list(sock[3]);
        printf("Server 4 done\n");
        recreate=fopen("recieved","w+");
        for(int i=0;i<8;i++){
            printf("in loop %s\n",store[i] );
            if(i<2){
               k=0; 
            }
                
            else if(1<i<4){
                k=1;
            }
            else if(3<i<6){
                k=2;
            }
            else if(5<i<8){
                k=3;
            }
            if(strstr(store[i],".1")){
                count1++;
                printf("right servers%d\n",k );
                printf("filename %s\n",store[i] );
                printf("The number %d\n",sock[0] );
                printf("%d\n",k );
                if((send(sock[k],store[i],100,0))<0){
                    puts("Send failed");
                    return 1;   
                }
                printf("Finished send\n");
                printf("%d\n",k );
                if((recv(sock[k],serversize,20,0))<0){
                    printf("Send error in file\n");
                }
                printf("server size%s\n",serversize );
                int partsize=atoi(serversize);
                if((recv(sock[k],serverfile,partsize,0))<0){
                    printf("Send error in file\n");
                }
                printf("%s\n",serverfile );
                fwrite(serverfile,partsize,1,recreate);

            }
            if(count1>0){
                break;
            }
        }

        for(int i2=0;i2<8;i2++){
            printf("in loop %s\n",store[i2] );
            if(i2<2){
               k1=0; 
            }
                
            else if(1<i2<4){
                k1=1;
            }
            else if(3<i2<6){
                k1=2;
            }
            else if(5<i2<8){
                k1=3;
            }
            if(k1==k){
                printf("OLD server\n");
                i2++;
            }
            if(strstr(store[i2],".2")){
                count2++;
                printf("right servers%d\n",k );
                printf("right servers%d\n",k1 );
                printf("filename %s\n",store[i2] );
                printf("The number %d\n",sock[0] );
                printf("%d\n",k1 );
                if((send(sock[k1],store[i2],100,0))<0){
                    puts("Send failed");
                    return 1;   
                }
                printf("Finished send\n");
                printf("%d\n",k1 );
                if((recv(sock[k1],serversize,20,0))<0){
                    printf("Send error in file\n");
                }
                printf("server size%s\n",serversize );
                int partsize=atoi(serversize);
                if((recv(sock[k1],serverfile,partsize,0))<0){
                    printf("Send error in file\n");
                }
                printf("%s\n",serverfile );
                fwrite(serverfile,partsize,1,recreate);

            }
            if(count2>0){
                break;
            }
        }

        for(int i3=0;i3<8;i3++){
            printf("in loop %s\n",store[i3] );
            if(i3<2){
               k2=0; 
            }
            //printf("K value%d\n",k2 );   
            else if(1<i3 && i3<4){
                k2=1;
            }
            //printf("K value%d\n",k2 );
            else if(3<i3 && i3<6){
                k2=2;
            }
            //printf("K value%d\n",k2 );
            else if(5<i3 && i3<8){
                k2=3;
            }
            printf("K value%d\n",k2 );
            if(k2!=k&&k2!=k1){
                printf("OLD server\n");
                if(strstr(store[i3],".3")){
                    count3++;
                    printf("right servers%d\n",k );
                    printf("right servers%d\n",k2 );
                    printf("filename %s\n",store[i3] );
                    printf("The number %d\n",sock[0] );
                    printf("%d\n",k2 );
                    if((send(sock[k2],store[i3],100,0))<0){
                        puts("Send failed");
                        return 1;   
                    }
                    printf("Finished send\n");
                    printf("%d\n",k1 );
                    if((recv(sock[k2],serversize,20,0))<0){
                        printf("Send error in file\n");
                    }
                    printf("server size%s\n",serversize );
                    int partsize=atoi(serversize);
                    if((recv(sock[k2],serverfile,partsize,0))<0){
                        printf("Send error in file\n");
                    }
                    printf("%s\n",serverfile );
                    fwrite(serverfile,partsize,1,recreate);

                }
            }
            if(count3>0){
                break;
            }
        }

        for(int i4=0;i4<8;i4++){
            printf("in loop %s\n",store[i4] );
            if(i4<2){
               k3=0; 
            }
            //printf("K value%d\n",k2 );   
            else if(1<i4 && i4<4){
                k3=1;
            }
            //printf("K value%d\n",k2 );
            else if(3<i4 && i4<6){
                k3=2;
            }
            //printf("K value%d\n",k2 );
            else if(5<i4 && i4<8){
                k3=3;
            }
            printf("K value%d\n",k3 );
            if(k3!=k && k3!=k1 && k3!=k2){
                printf("OLD server\n");
                if(strstr(store[i4],".4")){
                    count4++;
                    printf("right servers%d\n",k );
                    printf("right servers%d\n",k2 );
                    printf("filename %s\n",store[i4] );
                    printf("The number %d\n",sock[0] );
                    printf("%d\n",k3 );
                    if((send(sock[k3],store[i4],100,0))<0){
                        puts("Send failed");
                        return 1;   
                    }
                    printf("Finished send\n");
                    printf("%d\n",k1 );
                    if((recv(sock[k3],serversize,20,0))<0){
                        printf("Send error in file\n");
                    }
                    printf("server size%s\n",serversize );
                    int partsize=atoi(serversize);
                    if((recv(sock[k3],serverfile,partsize,0))<0){
                        printf("Send error in file\n");
                    }
                    printf("%s\n",serverfile );
                    fwrite(serverfile,partsize,1,recreate);

                }
            }
            if(count4>0){
                break;
            }
        }
        

    }

    if(strcmp(command,"LIST")==0){
        printf("List command\n");
        enquire_filelist(sock[0]);
        enquire_filelist(sock[1]);
        enquire_filelist(sock[2]);
        enquire_filelist(sock[3]);
        for(int i=0;i<8;i++){
            if(array[i]==1){
                if(count1>0){
                    count1++;
                }
                else{
                    printf("1.txt\n");
                    count1++;
                }
            }
        }
        for(int i=0;i<8;i++){
            if(array[i]==2){
                if(count2>0){
                    count2++;
                }
                else{
                    printf("2.txt\n");
                    count2++;
                }
            }
        }
        for(int i=0;i<8;i++){
            if(array[i]==3){
                if(count3>0){
                    count3++;
                }
                else{
                    printf("3.txt\n");
                    count3++;
                }
            }
        }
        for(int i=0;i<8;i++){
            if(array[i]==4){
                if(count4>0){
                    count4++;
                }
                else{
                    printf("4.txt\n");
                    count4++;
                }
            }
        }
        if(count1==0){
            printf("1.txt incomplete\n" );
        }
        if(count2==0){
            printf("2.txt incomplete\n" );
        }
        if(count3==0){
            printf("3.txt incomplete\n" );
        }
        if(count4==0){
            printf("4.txt incomplete\n" );
        }


    }
     
    close(sock[0]);
    close(sock[1]);
    close(sock[2]);
    close(sock[3]);

    return 0;
}
