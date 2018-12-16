/******************************************
* PA4
* dfc.c
* Date: 08 December 2018
*Author: Sanika Dongre
******************************************/
#include <stdio.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <dirent.h>
#include <openssl/md5.h>
#define MAXBUFSIZE 1024
void encoding(char *buffer, int data_len, char key1[]);
void decoding(char *buffer, int data_len, char key1[]);

void encoding(char *buffer, int data_len, char key1[])
{
	int key1_length = strlen(key1);
	for(int i=0; i<data_len; i++)
	{
		buffer[i] ^= key1[i%(key1_length-1)];
	}
}
void decoding(char *buffer, int data_len, char key1[])
{
	int key1_length = strlen(key1);
	for(int i=0; i<data_len; i++)
	{
		buffer[i] ^= key1[i%(key1_length-1)];
	}
}
typedef struct
{
    int access_port[4][64];
	uint8_t dfs[4][64];
    uint8_t name_user[4][64];
	uint8_t config_dfs[4][64];
	uint8_t password[4][64];
}parsing_data;

typedef struct
{
	char name_user[64];
	char password[64];
	char command[32];
	char filename[32];
}login_details;

int parse_file(parsing_data *parse, char* conf)
{
	FILE *fp;
	char line[MAXBUFSIZE];
	char *c;
	fp = fopen(conf, "r");
	if(!fp)
	{
    		printf("Error reading Configuration file\n");
    		return -1;
  	}
  	else
	{
    		while(fgets(line, sizeof(line), fp) > 0)
		{
      			//printf("%s\n", line );
      			if((c = strstr(line, "DFS1")))
			{
        			sscanf(c, "%s %[^:]%*c%d", (*parse).dfs[0], (*parse).config_dfs[0], (*parse).access_port[0]);
        			printf("************\n");
        			printf("%d\n", *(*parse).access_port[0]);
        			printf("%s\n", (*parse).dfs[0]);
        			printf("%s\n", (*parse).config_dfs[0]);
			}
			else if((c = strstr(line, "DFS2")))
			{
        			sscanf(c, "%s %[^:]%*c%d", (*parse).dfs[1], (*parse).config_dfs[1], (*parse).access_port[1]);
        			printf("************\n");
        			printf("%d\n", *(*parse).access_port[1]);
        			printf("%s\n", (*parse).dfs[1]);
        			printf("%s\n", (*parse).config_dfs[1]);
      			}
			else if((c = strstr(line, "DFS3")))
			{
        			sscanf(c, "%s %[^:]%*c%d", (*parse).dfs[2], (*parse).config_dfs[2], (*parse).access_port[2]);
        			printf("************\n");
        			printf("%d\n", *(*parse).access_port[2]);
        			printf("%s\n", (*parse).dfs[2]);
        			printf("%s\n", (*parse).config_dfs[2]);
			}
			else if((c = strstr(line, "DFS4")))
			{
        			sscanf(c, "%s %[^:]%*c%d", (*parse).dfs[3], (*parse).config_dfs[3], (*parse).access_port[3]);
        			printf("************\n");
        			printf("%d\n", *(*parse).access_port[3]);
        			printf("%s\n", (*parse).dfs[3]);
        			printf("%s\n", (*parse).config_dfs[3]);
      			}
			else if((c = strstr(line, "name_user: ")))
			{
        			sscanf(line, "%*s %s", (*parse).name_user[0]);
        			printf("************\n");
        			printf("Username is: %s\n", (*parse).name_user[0]);
     		 	}
			else if((c = strstr(line, "Password: ")))
			{
        			sscanf(line, "%*s %s", (*parse).password[0]);
        			printf("The password is: %s\n", (*parse).password[0]);
      			}

    		}
    		return 0;
  	}
}
char* MD5sum(char *filename)
{
	unsigned char hash_hex[MD5_DIGEST_LENGTH];
	int i;
	char *md5string =(char*) malloc(sizeof(char)*256);
	FILE *fp = fopen(filename, "rb");
	MD5_CTX mdContext;
	int nbytes;
	unsigned char buffer[1024];
	if(fp == NULL)
	{
      		printf ("Error opening file for MD5SUM Calculation\n");
      		return 0;
	}
	MD5_Init (&mdContext);
	while ((nbytes = fread(buffer, 1, 1024, fp)) != 0)
	{
    		MD5_Update (&mdContext, buffer, nbytes);
  	}
  	MD5_Final (hash_hex,&mdContext);
  	for(i = 0; i < MD5_DIGEST_LENGTH; i++)
	{
    		snprintf(&md5string[i*2], 32, "%02x", hash_hex[i]);
 	}
  	fclose(fp);
  	return (char *)md5string;
}

int* intMD5sum(char * hash_value)
{
	int *hash_int = (int*)malloc(sizeof(int));
	*hash_int = strtol(hash_value+31, NULL, 16);
	return (int *)hash_int;
}



int main(int argc, char * argv[])
{
	int checking_parse, socket_fd[4], temp=0;
	parsing_data parse;
	login_details *auth = (login_details*)malloc(sizeof(login_details));
	char command[32], cname[32], filename[128] = "NONE", subfolder[128] = "NONE", buffer[MAXBUFSIZE];
  	char* conf;
	struct sockaddr_in server_addr;
	int nbytes,x, parts_iteration;
	unsigned long int file_length, len_part, len_part4, read_length;
	if (argc < 2)
  	{
  		printf ("\nParameters missing: <conf file>\n");
  		exit(1);
  	}
	conf = argv[1];
	checking_parse = parse_file(&(parse), conf);
	if(checking_parse==-1)
	{
        	printf("Error Parsing the Configuration File\n");
      		exit(1);
    	}
	while(1)
	{
    		int flag1 = 0;
    		int flag2 = 0;
    		int flag3 = 0;
    		int flag4 = 0;
    		char filename1[128];
    		char filename2[128];
    		char filename3[128];
    		char filename4[128];
		printf("\n\n*******************Files in CLIENT folder*********************\n");
    		system("ls");
		printf("\n\tEnter Command as under: \n");
    		printf("\t1. list <subfolder> \n" "\t2. get <filename> <subfolder>\n" "\t3. put <filename> <subfolder>\n" "\t4. mkdir <SubDirect-name>\n");
    		scanf(" %[^\n]s",command);	// store the command entered by the user
    		bzero(cname, sizeof(cname));
    		bzero(filename, sizeof(filename));
    		bzero(subfolder, sizeof(subfolder));
		strcpy(filename, "NONE");
    		strcpy(subfolder, "NONE");
    		sscanf(command, "%s %s %s", cname, filename, subfolder);
    		printf("%s %s %s\n", cname, filename, subfolder );
		strcpy(auth->name_user, *parse.name_user);
 		strcpy(auth->password, *parse.password);
    		strcpy(auth->command, cname);
		int key_length = strlen(*parse.password);
    		char key1[key_length];
    		strcpy(key1, *parse.password);
		printf("Username:%sPassword:%sCommand:%s\n", auth->name_user, auth->password, auth->command);
		for(int i=0; i<4; i++)
		{
      			struct timeval timeout = {1,0};
      			if((socket_fd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
        			printf("Error in creating a socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.access_port[i]);
      			}
      			memset(&server_addr, 0, sizeof(server_addr));
      			server_addr.sin_family = AF_INET;
      			server_addr.sin_port = htons(*parse.access_port[i]);
      			server_addr.sin_addr.s_addr = inet_addr(parse.config_dfs[i]);
      			printf("\n\nsocket_fd %d: %d\n", i, socket_fd[i]);
      			printf("%d\n", *(parse).access_port[i]);
      			printf("%s\n", (parse).dfs[i]);
      			printf("%s\n", (parse).config_dfs[i]);
      			if(connect(socket_fd[i],(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
      			{
        			perror("Error: \n");
				printf("Error in Connecting to socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.access_port[i]);
				continue;
      			}
    			timeout.tv_sec = 0;
	    		timeout.tv_usec = 0;
			if(setsockopt (socket_fd[i], SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
			{
	        		perror("SETSOCKOPT failed\n");
      			}
			if((nbytes = send(socket_fd[i], auth, sizeof(*auth), 0)) < 0)
			{
       				printf("%d\n", nbytes);
        			printf("Error in sending to socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.access_port[i]);
      			}
      			if(!strcmp(cname, "put"))
			{
        			printf("\n************IN PUT*********\n");
       				printf("Put File: \"%s\" on the server.\n", filename);
        			char *hash_value = MD5sum(filename);
        			if(hash_value == 0)
				{
          				printf("Error Opening file or it Does not exist\n");
          				continue;
        			}
				int *hash_int = intMD5sum(hash_value);
				x = (*hash_int)%4;
        			printf("\n************\n");
        			printf("MD5HASH%%4 value: %d\n", x );
				bzero(buffer, MAXBUFSIZE);
        			nbytes = 0;
        			if((nbytes = recv(socket_fd[i], buffer, sizeof(buffer), 0))<0)
				{
		  			perror("Error: \n");
				}
				else printf("\n/**********\n%s\n**********/\n", buffer);
				if(!(strcmp(buffer, "User Exists" )))
				{
          				printf("User Exists: Server Ready to Put File\n");
					FILE * fp_part;
          				fp_part = fopen(filename, "rb");
          				if(fp_part == NULL)
					{
           					 perror("Error Opening File for sending to DFS: \n");
          				}
					fseek(fp_part, 0, SEEK_END);
          				file_length = ftell(fp_part);
          				fseek(fp_part, 0, SEEK_SET);
					len_part = (file_length/4);
          				len_part4 = len_part + (file_length%4);
					printf("File length: %lu\n", file_length );
          				printf("first 3: %lu, last: %lu\n", len_part, len_part4 );
          				parts_iteration = (len_part/MAXBUFSIZE);
					printf("Iterations: %d\n", parts_iteration);
					int part_map[4][4][2] =
					{
           					{{1,2},{2,3},{3,4},{4,1}},
           					{{4,1},{1,2},{2,3},{3,4}},
            					{{3,4},{4,1},{1,2},{2,3}},
            					{{2,3},{3,4},{4,1},{1,2}},
         				};
					if(part_map[x][i][0]==1 || part_map[x][i][1]==1)
					{
						if(!strcmp(subfolder, "NONE"))
						{
              						char filepath[128];
              						bzero(filepath, sizeof(filepath));
             						sprintf(filepath, "Part:1 %s %lu %s", filename, len_part, *parse.name_user);
              						if((nbytes = send(socket_fd[i], filepath, strlen(filepath), 0)) < 0)
							{
								perror("Error: \n");
              						}
           					 }
            					 else
						 {
              						char filepath[128];
              						bzero(filepath, sizeof(filepath));
              						sprintf(filepath, "Part:1 %s %lu %s/%s", filename, len_part, *parse.name_user, subfolder);
             						if((nbytes = send(socket_fd[i], filepath, strlen(filepath), 0)) < 0)
							{
                						perror("Error: \n");
              						}
						}
						bzero(buffer, MAXBUFSIZE);
            					recv(socket_fd[i], buffer, sizeof(buffer), 0);
						fseek(fp_part, 0, SEEK_SET);
						do
						{
              						bzero(buffer, MAXBUFSIZE);
              						read_length = fread(buffer, 1, MAXBUFSIZE, fp_part);
              						encoding(buffer, read_length, key1); //encrypting data to be sent on the server
              						if((nbytes = send(socket_fd[i], buffer, read_length, 0)) < 0)
							{
                						printf("Sending to DFS1: %d bytes\n", nbytes);
                						printf("Error in sending to socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.access_port[i]);
              						}
              						bzero(buffer, MAXBUFSIZE);
              						recv(socket_fd[i], buffer, sizeof(buffer), 0);
							temp++;
							if(temp == (parts_iteration))
							{
                						bzero(buffer, MAXBUFSIZE);
                						read_length = fread(buffer, 1, (len_part%MAXBUFSIZE), fp_part);
                						encoding(buffer, read_length, key1);
                						if((nbytes = send(socket_fd[i], buffer, read_length, 0)) < 0)
								{
                  							printf("Sending to DFS1: %d bytes\n", nbytes);
                  							printf("Error in sending to socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.access_port[i]);
                						}
                						bzero(buffer, MAXBUFSIZE);
                						recv(socket_fd[i], buffer, sizeof(buffer), 0);
              							}
            					  }while(temp<parts_iteration);
            					  temp=0;
          				}
          				if(part_map[x][i][0]==2 || part_map[x][i][1]==2)
					{
            					if(!strcmp(subfolder, "NONE"))
						{
             						char filepath[128];
              						bzero(filepath, sizeof(filepath));
              						sprintf(filepath, "Part:2 %s %lu %s", filename, len_part, *parse.name_user);
              						if((nbytes = send(socket_fd[i], filepath, strlen(filepath), 0)) < 0)
							{
                					 	perror("Error: \n");
              						}
            					}
            					else
						{
             						 char filepath[128];
             						 bzero(filepath, sizeof(filepath));
              						 sprintf(filepath, "Part:2 %s %lu %s/%s", filename, len_part, *parse.name_user, subfolder);
              						 if((nbytes = send(socket_fd[i], filepath, strlen(filepath), 0)) < 0)
							 {
                					 	perror("Error: \n");
              						 }
            					}
						bzero(buffer, MAXBUFSIZE);
            					recv(socket_fd[i], buffer, sizeof(buffer), 0);
            					fseek(fp_part, len_part, SEEK_SET);
						do
						{
              						bzero(buffer, MAXBUFSIZE);
              						read_length = fread(buffer, 1, MAXBUFSIZE, fp_part);
              						encoding(buffer, read_length, key1);
              						if((nbytes = send(socket_fd[i], buffer, read_length, 0)) < 0)
							{
                						printf("Error in sending to socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.access_port[i]);
              						}
              						temp++;
              						bzero(buffer, MAXBUFSIZE);
              						recv(socket_fd[i], buffer, sizeof(buffer), 0);
							if(temp == (parts_iteration))
							{
                						bzero(buffer, MAXBUFSIZE);
                						read_length = fread(buffer, 1, (len_part%MAXBUFSIZE), fp_part);
                						encoding(buffer, read_length, key1);
                						if((nbytes = send(socket_fd[i], buffer, read_length, 0)) < 0)
								{
                  							printf("Error in sending to socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.access_port[i]);
                						}
                						bzero(buffer, MAXBUFSIZE);
                						recv(socket_fd[i], buffer, sizeof(buffer), 0);
              						}
           					 }while(temp<parts_iteration);
            					 temp=0;
          				}
          				if(part_map[x][i][0]==3 || part_map[x][i][1]==3)
					{
            					if(!strcmp(subfolder, "NONE"))
						{
              						char filepath[128];
              						bzero(filepath, sizeof(filepath));
             					        sprintf(filepath, "Part:3 %s %lu %s", filename, len_part, *parse.name_user);
              						if((nbytes = send(socket_fd[i], filepath, strlen(filepath), 0)) < 0)
							{
                						perror("Error: \n");
              						}
            					}
            					else
						{
              						char filepath[128];
              						bzero(filepath, sizeof(filepath));
             						sprintf(filepath, "Part:3 %s %lu %s/%s", filename, len_part, *parse.name_user, subfolder);
              						if((nbytes = send(socket_fd[i], filepath, strlen(filepath), 0)) < 0)
							{
                						perror("Error: \n");
              						}
            					}
						bzero(buffer, MAXBUFSIZE);
            					recv(socket_fd[i], buffer, sizeof(buffer), 0);
            					fseek(fp_part, (2*len_part), SEEK_SET);
						do
						{
              						bzero(buffer, MAXBUFSIZE);
              						read_length = fread(buffer, 1, MAXBUFSIZE, fp_part);
              						encoding(buffer, read_length, key1);
              						if((nbytes = send(socket_fd[i], buffer, read_length, 0)) < 0)
							{
                						printf("Sending to DFS2: %d bytes\n", nbytes);printf("Sending to DFS3: %d bytes\n", nbytes);
                						printf("Error in sending to socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.access_port[i]);
							}
							temp++;
							bzero(buffer, MAXBUFSIZE);
              						recv(socket_fd[i], buffer, sizeof(buffer), 0);
							if(temp == (parts_iteration))
							{
                						bzero(buffer, MAXBUFSIZE);
                						read_length = fread(buffer, 1, (len_part%MAXBUFSIZE), fp_part);
                						encoding(buffer, read_length, key1);
                						if((nbytes = send(socket_fd[i], buffer, read_length, 0)) < 0)
								{
                  							printf("Sending to DFS3: %d bytes\n", nbytes);
                  							printf("Error in sending to socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.access_port[i]);
                						}
                						bzero(buffer, MAXBUFSIZE);
                						recv(socket_fd[i], buffer, sizeof(buffer), 0);
              						}
            					}while(temp<parts_iteration);
            					temp=0;
          				}
          				if(part_map[x][i][0]==4 || part_map[x][i][1]==4)
					{
            					if(!strcmp(subfolder, "NONE"))
						{
              						char filepath[128];
              						bzero(filepath, sizeof(filepath));
              						sprintf(filepath, "Part:4 %s %lu %s", filename, len_part, *parse.name_user);
              						if((nbytes = send(socket_fd[i], filepath, strlen(filepath), 0)) < 0)
							{
                						perror("Error: \n");
              						}
           					 }
           					else
						{
              						char filepath[128];
              						bzero(filepath, sizeof(filepath));
              						sprintf(filepath, "Part:4 %s %lu %s/%s", filename, len_part, *parse.name_user, subfolder);
              						if((nbytes = send(socket_fd[i], filepath, strlen(filepath), 0)) < 0)
							{
                						perror("Error: \n");
              						}
            					}
						bzero(buffer, MAXBUFSIZE);
            					recv(socket_fd[i], buffer, sizeof(buffer), 0);
            					fseek(fp_part, (3*len_part), SEEK_SET);
						do
						{
              						bzero(buffer, MAXBUFSIZE);
              						read_length = fread(buffer, 1, MAXBUFSIZE, fp_part);
              						encoding(buffer, read_length, key1);
              						if((nbytes = send(socket_fd[i], buffer, read_length, 0)) < 0)
							{
                						printf("Sending to DFS4: %d bytes\n", nbytes);
                						printf("Error in sending to socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.access_port[i]);
              						}
							printf("Sending to DFS4: %d bytes\n", nbytes);
							temp++;
              						bzero(buffer, MAXBUFSIZE);
              						recv(socket_fd[i], buffer, sizeof(buffer), 0);
							if(temp == (parts_iteration))
							{
                						bzero(buffer, MAXBUFSIZE);
                						read_length = fread(buffer, 1, (len_part4%MAXBUFSIZE), fp_part);
                						encoding(buffer, read_length, key1);
                						if((nbytes = send(socket_fd[i], buffer, read_length, 0)) < 0)
								{
                  							printf("Sending to DFS4: %d bytes\n", nbytes);
                  							printf("Error in sending to socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.access_port[i]);
                						}
                						bzero(buffer, MAXBUFSIZE);
                						recv(socket_fd[i], buffer, sizeof(buffer), 0);
              						}
            					}while(temp<parts_iteration);
            					temp=0;
          				}
        			}
      			}
     			else if(!strcmp(cname, "list"))
			{
				bzero(buffer, MAXBUFSIZE);
        			nbytes = 0;
        			if((nbytes = recv(socket_fd[i], buffer, sizeof(buffer), 0))<0)
				{
          				perror("Error: \n");
        			}
				else printf("\n/**********\n %s \n**********/\n", buffer);
        			char msg[] = "Synq";
        			if((nbytes = send(socket_fd[i], msg, strlen(msg), 0)) < 0)
				{
         				 printf("In Synq Send()\n");
		  			 perror("Error: \n");
				}
        			if(!(strcmp(buffer, "User Exists" )))
				{
          				printf("User Exists: Server Ready to List File\n");
          				bzero(buffer, MAXBUFSIZE);
          				nbytes = 0;
          				if((nbytes = recv(socket_fd[i], buffer, sizeof(buffer), 0))<0)
					{
            					perror("Error: \n");
          				}
					if(!strcmp(filename, "NONE"))
					{
            					char filepath[128];
            					bzero(filepath, sizeof(filepath));
            					sprintf(filepath, "%s", *parse.name_user);
            					if((nbytes = send(socket_fd[i], filepath, strlen(filepath), 0)) < 0)
						{
							 perror("Error: \n");
            					}
           					bzero(buffer, MAXBUFSIZE);
            					nbytes = 0;
            					if((nbytes = recv(socket_fd[i], buffer, sizeof(buffer), 0))<0)
						{
              						perror("Error: \n");
           					}
          				}
          				else
					{
            					char filepath[128];
           					bzero(filepath, sizeof(filepath));
            					sprintf(filepath, "%s/%s", *parse.name_user, filename);
            					if((nbytes = send(socket_fd[i], filepath, strlen(filepath), 0)) < 0)
						{
              						perror("Error: \n");
           					}
            					bzero(buffer, MAXBUFSIZE);
            					nbytes = 0;
            					if((nbytes = recv(socket_fd[i], buffer, sizeof(buffer), 0))<0)
						{
              						perror("Error: \n");
						 }
					}
          				bzero(buffer, MAXBUFSIZE);
          				nbytes = 0;
          				if((nbytes = recv(socket_fd[i], buffer, sizeof(buffer), 0))<0)
					{
            					perror("Error: \n");
           					exit(1);
          				}
          				if((nbytes = send(socket_fd[i], buffer, strlen(buffer), 0)) < 0)
					{
           					printf("In Synq Send()\n");
            					perror("Error: \n");
          				}
					FILE *fp;
          				fp = fopen("list_file_temp", "ab");
          				if(!fp)
					{
            					printf("Error creating temporary list file\n");
            					return -1;
          				}
          				fwrite(buffer, 1, nbytes, fp);
          				fclose(fp);
        			}
      			}
      			else if(!strcmp(cname, "get"))
			{
       				char msg[] = "Synq message";
        			printf("Get File: \"%s\" from the server.\n", filename);
        			bzero(buffer, MAXBUFSIZE);
        			nbytes = 0;
        			if((nbytes = recv(socket_fd[i], buffer, sizeof(buffer), 0))<0)
				{
          				perror("Error: \n");
       				}
				else printf("\n/**********\n%s\n**********/\n", buffer);
				if(!(strcmp(buffer, "User Exists" )))
				{
          				printf("User Exists: Server Ready to SEND File\n");
					if(!strcmp(subfolder, "NONE"))
					{
            					char filepath[128];
           					bzero(filepath, sizeof(filepath));
            					sprintf(filepath, "%s %s", filename, *parse.name_user);
            					if((nbytes = send(socket_fd[i], filepath, strlen(filepath), 0)) < 0)
						{
              						perror("Error: \n");
            					}
          				}
          				else
					{
           					char filepath[128];
            					bzero(filepath, sizeof(filepath));
            					sprintf(filepath, "%s %s/%s", filename, *parse.name_user, subfolder);
            					if((nbytes = send(socket_fd[i], filepath, strlen(filepath), 0)) < 0)
						{
              						perror("Error: \n");
            					}
          				}
					bzero(filename1, sizeof(filename1));
          				sprintf(filename1, ".%s.1", filename);
					bzero(filename2, sizeof(filename2));
          				sprintf(filename2, ".%s.2", filename);
					bzero(filename3, sizeof(filename3));
          				sprintf(filename3, ".%s.3", filename);
					bzero(filename4, sizeof(filename4));
          				sprintf(filename4, ".%s.4", filename);
					bzero(buffer, MAXBUFSIZE);
          				recv(socket_fd[i], buffer, sizeof(buffer), 0);
          				printf("Part status: %s \n", buffer );
					if((strstr(buffer, "YES") != NULL) && (strstr(buffer, filename1) != NULL) && (flag1==0))
					{
            					if((nbytes = send(socket_fd[i], "SEND", strlen("SEND"), 0)) < 0)
						{
              						perror("Error In Part Send: \n");
					        }
            				 	FILE* fp;
            					fp = fopen(filename1, "ab");
            					if(!fp)
						{
              						printf("Error Opening : %s\n", filename1);
              						perror("Error \n");
            					}
            					else
						{
              						do
							{
                						bzero(buffer, MAXBUFSIZE);
                						if((nbytes = recv(socket_fd[i], buffer, sizeof(buffer), 0)) < 0)
								{
                  							printf("Error: Reading from the socket\n");
               							}
								printf("Read length %d\n", nbytes );
								decoding(buffer, nbytes, key1);
                						int write_length = fwrite(buffer, 1, nbytes, fp);
								if(write_length != MAXBUFSIZE)
								{
                  							break;
                						}
                						if((nbytes = send(socket_fd[i], msg, strlen(msg), 0)) < 0)
								{
                 							 printf("In Synq Send()\n");
                 							 perror("Error: \n");
                						}

             						 }while(1);
              						 flag1 = 1;
              						 fclose(fp);
						 }
          				  }
          				  else if((strstr(buffer, "YES") != NULL) && (strstr(buffer, filename1) != NULL) && (flag1==1))
					  {
            				  	if((nbytes = send(socket_fd[i], "DONT", strlen("DONT"), 0)) < 0)
						{
              						printf("In Part Send\n");
              						perror("Error: \n");
            					}
            					sleep(1);
          				}
					bzero(buffer, MAXBUFSIZE);
          				strcpy(buffer, "Last Synq message in part-1");
          				if((nbytes = send(socket_fd[i], buffer, strlen(buffer), 0)) < 0)
					{
            					printf("In Synq Send()\n");
            					perror("Error: \n");
         			        }
          				bzero(buffer, MAXBUFSIZE);
          				recv(socket_fd[i], buffer, sizeof(buffer), 0);
          				printf("Part status: %s \n", buffer );
					if((strstr(buffer, "YES") != NULL) && (strstr(buffer, filename2) != NULL) && (flag2==0))
					{
            					if((nbytes = send(socket_fd[i], "SEND", strlen("SEND"), 0)) < 0)
						{
              						printf("In Part Send\n");
              						perror("Error: \n");
						}
            					FILE* fp;
            					fp = fopen(filename2, "ab");
            					if(!fp)
						{
              						printf("Error Opening : %s\n", filename2);
              						perror("Error \n");
            					}
            					else
						{
              						do
							{
                						bzero(buffer, MAXBUFSIZE);
									if((nbytes = recv(socket_fd[i], buffer, sizeof(buffer), 0)) < 0)
									{
                  								printf("Error: Reading from the socket\n");
									}
									printf("Read length %d\n", nbytes );
									decoding(buffer, nbytes, key1);
									int write_length = fwrite(buffer, 1, nbytes, fp);
									if(write_length != MAXBUFSIZE)
									{
                  								break;
                							}
                							if((nbytes = send(socket_fd[i], msg, strlen(msg), 0)) < 0)
									{
                  								printf("In Synq Send()\n");
                  								perror("Error: \n");
                							}

              						}while(1);
              						flag2 = 1;
              						fclose(fp);
            					}
         				}
          				else if((strstr(buffer, "YES") != NULL) && (strstr(buffer, filename2) != NULL) && (flag2==1))
					{
            					if((nbytes = send(socket_fd[i], "DONT", strlen("DONT"), 0)) < 0)
						{
              						printf("In Part Send\n");
              						perror("Error: \n");
            					}
            					sleep(1);
          				}
          				bzero(buffer, MAXBUFSIZE);
          				strcpy(buffer, "Last Synq message in part2");
          				if((nbytes = send(socket_fd[i], buffer, strlen(buffer), 0)) < 0)
					{
            					printf("In Synq Send()\n");
            					perror("Error: \n");
          				}
          				bzero(buffer, MAXBUFSIZE);
          				recv(socket_fd[i], buffer, sizeof(buffer), 0);
          				printf("Part Status: %s \n", buffer );
					if((strstr(buffer, "YES") != NULL) && (strstr(buffer, filename3) != NULL) && (flag3==0))
					{
           					 if((nbytes = send(socket_fd[i], "SEND", strlen("SEND"), 0)) < 0)
						 {
              					 	printf("In Part Send\n");
              						perror("Error: \n");
            					 }
            					 FILE* fp;
            					 fp = fopen(filename3, "ab");
            					 if(!fp)
						 {
              					 	printf("Error Opening : %s\n", filename3);
              						perror("Error \n");
            					 }
            					 else
						 {
              						do
							{
               							 bzero(buffer, MAXBUFSIZE);
                						 if((nbytes = recv(socket_fd[i], buffer, sizeof(buffer), 0)) < 0)
								 {
                  						  	printf("Error: Reading from the socket\n");
                						 }
								 printf("Read length %d\n", nbytes );
								 decoding(buffer, nbytes, key1);
                						 int write_length = fwrite(buffer, 1, nbytes, fp);
								 if(write_length != MAXBUFSIZE)
								 {
                  						  	break;
               							 }
                						 if((nbytes = send(socket_fd[i], msg, strlen(msg), 0)) < 0)
								{
                  							printf("In Synq Send()\n");
                  							perror("Error: \n");
                						}

              						}while(1);
              						flag3 = 1;
              						fclose(fp);
            					}
         				}
          				else if((strstr(buffer, "YES") != NULL) && (strstr(buffer, filename3) != NULL) && (flag3==1))
					{
            					if((nbytes = send(socket_fd[i], "DONT", strlen("DONT"), 0)) < 0)
						{
              						printf("In Part Send\n");
              						perror("Error: \n");
            					}
            					sleep(1);
          				}
          				bzero(buffer, MAXBUFSIZE);
          				strcpy(buffer, "Last Synq message in part3");
          				if((nbytes = send(socket_fd[i], buffer, strlen(buffer), 0)) < 0)
					{
            					printf("In Synq Send()\n");
            					perror("Error: \n");
         				}
				        bzero(buffer, MAXBUFSIZE);
          				recv(socket_fd[i], buffer, sizeof(buffer), 0);
          				printf("Part Status: %s \n", buffer );
					if((strstr(buffer, "YES") != NULL) && (strstr(buffer, filename4) != NULL) && (flag4==0))
					{
            					if((nbytes = send(socket_fd[i], "SEND", strlen("SEND"), 0)) < 0)
						{
              						printf("In Part Send\n");
              						perror("Error: \n");
           					 }
            					FILE* fp;
            					fp = fopen(filename4, "ab");
            					if(!fp)
						{
              						printf("Error Opening : %s\n", filename4);
              						perror("Error \n");
            					}
            					else
						{
              						do
							{
                						bzero(buffer, MAXBUFSIZE);
								if((nbytes = recv(socket_fd[i], buffer, sizeof(buffer), 0)) < 0)
								{
                  							printf("Error: Reading from the socket\n");
								}
								printf("Read length %d\n", nbytes );
                						decoding(buffer, nbytes, key1);
								int write_length = fwrite(buffer, 1, nbytes, fp);
								if(write_length != MAXBUFSIZE)
								{
                  							break;
                						}
                						if((nbytes = send(socket_fd[i], msg, strlen(msg), 0)) < 0)
								{
                  							printf("In Synq Send()\n");
                  							perror("Error: \n");
                						}

              						}while(1);
              						flag4 = 1;
              						fclose(fp);
            					}
         				}
          				else if((strstr(buffer, "YES") != NULL) && (strstr(buffer, filename4) != NULL) && (flag4==1))
					{
            					if((nbytes = send(socket_fd[i], "DONT", strlen("DONT"), 0)) < 0)
						{
              						printf("In Part Send\n");
              						perror("Error: \n");
            					}
           					sleep(1);
          				}
          				bzero(buffer, MAXBUFSIZE);
          				strcpy(buffer, "Last Synq message in part4");
          				if((nbytes = send(socket_fd[i], buffer, strlen(buffer), 0)) < 0)
					{
            					printf("In Synq Send()\n");
            					perror("Error: \n");
          				}
        			}
        			else
				{
         				 printf("Check For Credentials\n");
        			}
      			}
			else if(!strcmp(cname, "mkdir"))
			{
       				printf("\n************IN MKDIR*********\n");
				printf("Make Subfolder: \"%s\" on the server.\n", filename);
        			bzero(buffer, MAXBUFSIZE);
        			nbytes = 0;
       				if((nbytes = recv(socket_fd[i], buffer, sizeof(buffer), 0))<0)
				{
         				perror("Error: \n");
        			}
				else printf("\n/**********\n%s\n**********/\n", buffer);
				if(!(strcmp(buffer, "User Exists" )))
				{
          				printf("User Exists: Server Ready to MAKE subfolder\n");
					if((nbytes = send(socket_fd[i], filename, strlen(filename), 0)) < 0)
					{
           					printf("Sending to DFS%d: %d bytes\n", i+1, nbytes);
            					printf("Error in sending to socket for the Server:%s at Port: %d\n", parse.dfs[i], *parse.access_port[i]);
          				}

        			}
     			}
    		}
		if(!strcmp(cname, "list"))
		{
      			system("sort list_file_temp | uniq > list_file");
			FILE *fp;
      			FILE *fp_dup;
      			char line[MAXBUFSIZE];
      			char line_dup[MAXBUFSIZE];
      			char * c;
      			char list_filename[128];
      			char list_filename_dup[128];
      			int count = 0;
			fp = fopen("list_file", "r");
      			if(!fp)
			{
        			printf("Error reading list file\n");
        			return -1;
      			}
      			else
			{
        			printf("\n\n*****List from Servers ****** \n");
        			fgets(line, sizeof(line), fp);
        			if((c = strstr((char *)line, "."))){
          			bzero(list_filename, sizeof(list_filename));
          			strncpy(list_filename, line+strlen("."), strlen(line)-4);
				fp_dup = fp;
          			while(fgets(line_dup, sizeof(line_dup), fp_dup))
				{
            				if((c = strstr((char *)line_dup, "."))){
              				bzero(list_filename_dup, sizeof(list_filename_dup));
              				strncpy(list_filename_dup, line_dup+strlen("."), strlen(line_dup)-4);
              				if(!strcmp(list_filename_dup, list_filename))
					{
                				count = count + 1;
					}
              				else
					{
                				if(count==3)
						{
                  					printf("\t%s [complete]\n",list_filename);
               					}
                				else
						{
                  					printf("\t%s [incomplete]\n",list_filename);
                				}
                				count = 0;
                				bzero(list_filename, sizeof(list_filename));
                				strcpy(list_filename, list_filename_dup);
              				}
            			}
          		}
          		if(count==3)
			{
            			printf("\t%s [complete]\n",list_filename);
          		}
          		else
			{
            			printf("\t%s [incomplete]\n",list_filename);
			}
			count = 0;
          		bzero(list_filename, sizeof(list_filename));
          		strcpy(list_filename, list_filename_dup);
        	}
	}
      	remove("list_file");
      	remove("list_file_temp");
 }
 else if(!strcmp(cname, "get"))
{
      if((flag1==1) && (flag2==1) && (flag3==1) && (flag4==1))
	{
        	char concat_parts_rm[264];
        	char concat_command[264];
        	bzero(concat_parts_rm, sizeof(concat_parts_rm));
        	bzero(concat_command, sizeof(concat_command));
        	sprintf(concat_parts_rm, "rm %s %s %s %s", filename1, filename2, filename3, filename4);
		sprintf(concat_command, "cat %s %s %s %s > %s", filename1, filename2, filename3, filename4, filename);
		system(concat_command);
        	system(concat_parts_rm);
       }
       else
	{
        	printf("\n\n\n\n************File is Incomplete in Get*************\n\n\n");
      	}
    }
  }
return 0;
}
