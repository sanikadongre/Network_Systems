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
typedef struct
{
	char dfs[4][64];
	int port_num[4][64];
	char dfs_ip[4][64];
	char username[4][64];
	char password[4][64];
}parsing_data;

typedef struct
{
	char username[64];
	char password[64];
	char command[32];
	char filename[32];
}login_details;

/*function for encryption decryption*/
void encoding(char *temp_buf, int size_len, char key[])
{
	int len, i=0;
	len =  strlen(key);
	for(i=0; i<size_len; i++)
	{
		temp_buf[i] ^= key[i%(len-1)];
	}
}
/*function for parsing file*/

int output_parse(parsing_data *parse_type, char* config_data)
{
	char data_output[MAXBUFSIZE];
	char *compare;
	FILE *fptr;
	fptr = fopen(config_data, "r");
	if(fptr!=0)
	{
		while(fgets(data_output,MAXBUFSIZE, fptr) > 0)
		{
			if((compare = strstr(data_output, "DFS1")))
			{
        			sscanf(compare, "%s %[^:]%*c%d", (*parse_type).dfs[0], (*parse_type).dfs_ip[0], (*parse_type).port_num[0]);
        			printf("%d\n", *(*parse_type).port_num[0]);
        			printf("%s\n", (*parse_type).dfs[0]);
        			printf("%s\n", (*parse_type).dfs_ip[0]);
			}
			else if((compare = strstr(data_output, "DFS2")))
			{
        			sscanf(compare, "%s %[^:]%*c%d", (*parse_type).dfs[1], (*parse_type).dfs_ip[1], (*parse_type).port_num[1]);
        			printf("%d\n", *(*parse_type).port_num[1]);
        			printf("%s\n", (*parse_type).dfs[1]);
        			printf("%s\n", (*parse_type).dfs_ip[1]);
      			}
			else if((compare = strstr(data_output, "DFS3")))
			{
        			sscanf(compare, "%s %[^:]%*c%d", (*parse_type).dfs[2], (*parse_type).dfs_ip[2], (*parse_type).port_num[2]);
        			printf("%d\n", *(*parse_type).port_num[2]);
        			printf("%s\n", (*parse_type).dfs[2]);
        			printf("%s\n", (*parse_type).dfs_ip[2]);
			}
			else if((compare = strstr(data_output, "DFS4")))
			{
        			sscanf(compare, "%s %[^:]%*c%d", (*parse_type).dfs[3], (*parse_type).dfs_ip[3], (*parse_type).port_num[3]);
        			printf("%d\n", *(*parse_type).port_num[3]);
        			printf("%s\n", (*parse_type).dfs[3]);
        			printf("%s\n", (*parse_type).dfs_ip[3]);
      			}
			else if((compare = strstr(data_output, "Username: ")))
			{
        			sscanf(data_output, "%*s %s", (*parse_type).username[0]);
        			printf("Username is: %s\n", (*parse_type).username[0]);
     		 	}
			else if((compare = strstr(data_output, "Password: ")))
			{
        			sscanf(data_output, "%*s %s", (*parse_type).password[0]);
        			printf("The password is: %s\n", (*parse_type).password[0]);
      			}

    		}
    		return 0;
    		
  	}
  	else
	{
    	 	perror("error in reading the configuration file");
    		return -1;
  	}
}
/*md5 calcualte function*/
char* calculate_md5sum(char *file_name)
{
	unsigned char md5_value[MD5_DIGEST_LENGTH];
	int i, bytes_tot;
	FILE *fptr;
	fptr = fopen(file_name, "rb");
	char *md5string =(char*) malloc(sizeof(char)*256);
	MD5_CTX Context;
	unsigned char data_buff[MAXBUFSIZE];
	if(fptr!= NULL) 
	{	MD5_Init (&Context);
		while ((bytes_tot = fread(data_buff, 1, 1024, fptr))!= 0)
		{
	    		MD5_Update (&Context, data_buff, bytes_tot);
	  	}
	  	MD5_Final (md5_value,&Context);
	  	for(i = 0; i < MD5_DIGEST_LENGTH; i++)
		{
	    		snprintf(&md5string[i*2], 32, "%02x", md5_value[i]);
	 	}
	  	fclose(fptr);
  		return (char *)md5string;
	}
	else
	{
		printf ("Error opening file for MD5SUM Calculation\n");
      		return 0;
	
	}
}
/*function that defines error condition*/
void sock_error_condition(int bytes_tot, char file_error[], int socketfd[], int i)
{
	if((bytes_tot = send(socketfd[i], file_error, strlen(file_error), 0)) < 0)
	{
              perror("error in sending socket");
        }
}
/*md5sum value*/

int* md5sum_int_val(char* hash_calc)
{
	int *inthash = (int*)malloc(sizeof(int));
	*inthash = strtol(hash_calc+31, NULL, 16);
	return (int *)inthash;
}
/*Main function*/

int main(int argc, char * argv[])
{
	int flag_parse, bytes_tot, socketfd[4], hash_calc, sum_parts, temp_data=0;
	parsing_data parse;
	login_details *verify = (login_details*)malloc(sizeof(login_details));
	char command[32], name_command[32], filename[148] = "NONE", minifolder[148] = "NONE", data_array[MAXBUFSIZE];
  	char* data_config;
	struct sockaddr_in server_addr;
	unsigned long int len_file, part_len, part4_len, len_read;
	if (argc < 2)
  	{
  		perror("include the conf file");
  		exit(1);
  	}
	data_config = argv[1];
	flag_parse = output_parse(&(parse), data_config);
	if(flag_parse==-1)
	{
        	perror("error parsing the configuration file");
      		exit(1);
    	}
	while(1)
	{
    		int flag_1 = 0, flag_2 = 0, flag_3 = 0, flag_4 = 0;
    		char file_name1[128], file_name2[128], file_name3[128], file_name4[128];
		printf("\n\n**Files-client folder******\n");
    		system("ls");
		printf("\n\tChoose command of choice: \n");
    		printf("\n\t1. get <filename> <subfolder> \n" "\t2. put <filename> <subfolder>\n" "\t3. list <subfolder>\n" "\t4. mkdir <SubDirect-name>\n");
    		scanf(" %[^\n]s",command);	
    		bzero(name_command, sizeof(name_command));
    		bzero(filename, sizeof(filename));
    		bzero(minifolder, sizeof(minifolder));
		strcpy(filename, "NONE");
    		strcpy(minifolder, "NONE");
    		sscanf(command, "%s %s %s",name_command, filename,minifolder);
    		printf("%s %s %s\n",name_command, filename,minifolder);
		strcpy(verify->username, *parse.username);
 		strcpy(verify->password, *parse.password);
    		strcpy(verify->command, name_command);
		int key_length = strlen(*parse.password);
    		char key1[key_length];
    		strcpy(key1, *parse.password);
		printf("Username:%sPassword:%sCommand:%s\n", verify->username, verify->password, verify->command);
		for(int i=0; i<4; i++)
		{
      			struct timeval timeout = {1,0};
      			if((socketfd[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
        			printf("socket creation error for the server:%s for port: %d\n", parse.dfs[i], *parse.port_num[i]);
      			}
      			memset(&server_addr, 0, sizeof(server_addr));
      			server_addr.sin_family = AF_INET;
      			server_addr.sin_port = htons(*parse.port_num[i]);
      			server_addr.sin_addr.s_addr = inet_addr(parse.dfs_ip[i]);
      			printf("\n\n the sockfd is %d: %d\n", i, socketfd[i]);
      			printf("%d\n", *(parse).port_num[i]);
      			printf("%s\n", (parse).dfs[i]);
      			printf("%s\n", (parse).dfs_ip[i]);
      			if(connect(socketfd[i],(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
      			{
        			perror("error in connection");
				printf("error in connecting to socket for the server:%s for port: %d\n", parse.dfs[i], *parse.port_num[i]);
				continue;
      			}
    			timeout.tv_sec = 0;
	    		timeout.tv_usec = 0;
			if(setsockopt (socketfd[i], SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
			{
	        		perror("timeout failed");
      			}
			if((bytes_tot = send(socketfd[i],verify, sizeof(*verify), 0)) < 0)
			{
       				printf("%d\n", bytes_tot);
        			printf("error in sending to socket for the server:%s for port: %d\n", parse.dfs[i], *parse.port_num[i]);
      			}
      			if(strcmp(name_command, "put")==0)
			{
       				printf("putting file: \"%s\" on the server.\n", filename);
        			char *hash_value = calculate_md5sum(filename);
        			if(hash_value == 0)
				{
          				printf("file doesn't exist or can't be opened\n");
          				continue;
        			}
				int *hash_int = md5sum_int_val(hash_value);
				hash_calc = (*hash_int)%4;
        			printf("\n************\n");
        			printf(" md5hash value MD5HASH%%4 value: %d\n", hash_calc);
				bzero(data_array, MAXBUFSIZE);
        			bytes_tot = 0;
        			if((bytes_tot = recv(socketfd[i],data_array, sizeof(data_array), 0))<0)
				{
		  			perror("error in socket receiving");
				}
				else printf("\n/**********\n%s\n**********/\n", data_array);
				if((strcmp(data_array, "User Exists"))==0)
				{
          				printf("User Exists: server is ready to put file\n");
					FILE * fp_part;
          				fp_part = fopen(filename, "rb");
          				if(fp_part == NULL)
					{
           					 perror("file opening error");
          				}
					fseek(fp_part, 0, SEEK_END);
          				len_file = ftell(fp_part);
          				fseek(fp_part, 0, SEEK_SET);
					part_len = (len_file/4);
          				part4_len = part_len + (len_file%4);
					printf("File length: %lu\n", len_file);
          				printf("first 3: %lu, last: %lu\n",part_len, part4_len);
          				sum_parts = (part_len/MAXBUFSIZE);
					printf("Iterations: %d\n", sum_parts);
					int data_map[4][4][2] = 
					{
           					{{1,2},{2,3},{3,4},{4,1}},
           					{{4,1},{1,2},{2,3},{3,4}},
            					{{3,4},{4,1},{1,2},{2,3}},
            					{{2,3},{3,4},{4,1},{1,2}},
         				};
					if(data_map[hash_calc][i][0]==1 || data_map[hash_calc][i][1]==1)
					{
						if(strcmp(minifolder, "NONE")==0)
						{
              						char file_path[128];
              						bzero(file_path, sizeof(file_path));
             						sprintf(file_path, "Part:1 %s %lu %s", filename,part_len, *parse.username);
              						sock_error_condition(bytes_tot,file_path,socketfd,i);
           					 }
            					 else
						 {
              						char file_path[128];
              						bzero(file_path, sizeof(file_path));
              						sprintf(file_path, "Part:1 %s %lu %s/%s", filename,part_len, *parse.username,minifolder);
             						sock_error_condition(bytes_tot,file_path,socketfd,i);
						}
						bzero(data_array, MAXBUFSIZE);
            					recv(socketfd[i],data_array, sizeof(data_array), 0);
						fseek(fp_part, 0, SEEK_SET);
						do
						{
              						bzero(data_array, MAXBUFSIZE);
              						len_read = fread(data_array, 1, MAXBUFSIZE, fp_part);
              						encoding(data_array,len_read, key1); //encrypting data to be sent on the server
              						if((bytes_tot = send(socketfd[i],data_array,len_read, 0)) < 0)
							{
                						printf("Sending to DFS1: %d bytes\n", bytes_tot);
                						printf("error in sending to socket for the server:%s for Port: %d\n", parse.dfs[i], *parse.port_num[i]);
              						}
              						bzero(data_array, MAXBUFSIZE);
              						recv(socketfd[i],data_array, sizeof(data_array), 0);
							temp_data++;
							if(temp_data == (sum_parts))
							{
                						bzero(data_array, MAXBUFSIZE);
                						len_read = fread(data_array, 1, (part_len%MAXBUFSIZE), fp_part);
                						encoding(data_array,len_read, key1);
                						if((bytes_tot = send(socketfd[i],data_array,len_read, 0)) < 0)
								{
                  							printf("Sending to DFS1: %d bytes\n", bytes_tot);
                  							printf("error in sending to socket for the server:%s for port: %d\n", parse.dfs[i], *parse.port_num[i]);
                						}
                						bzero(data_array, MAXBUFSIZE);
                						recv(socketfd[i],data_array, sizeof(data_array), 0);
              							}
            					  }while(temp_data <sum_parts);
            					  temp_data=0;
          				}
          				if(data_map[hash_calc][i][0]==2 || data_map[hash_calc][i][1]==2)
					{
            					if(!strcmp(minifolder, "NONE"))
						{
             						char file_path[128];
              						bzero(file_path, sizeof(file_path));
              						sprintf(file_path, "Part:2 %s %lu %s", filename,part_len, *parse.username);
              						sock_error_condition(bytes_tot,file_path,socketfd,i);
            					}
            					else
						{
             						 char file_path[128];
             						 bzero(file_path, sizeof(file_path));
              						 sprintf(file_path, "Part:2 %s %lu %s/%s", filename,part_len, *parse.username,minifolder);
              						 sock_error_condition(bytes_tot,file_path,socketfd,i);
            					}
						bzero(data_array, MAXBUFSIZE); 
            					recv(socketfd[i],data_array, sizeof(data_array), 0);
            					fseek(fp_part,part_len, SEEK_SET);
						do
						{
              						bzero(data_array, MAXBUFSIZE);
              						len_read = fread(data_array, 1, MAXBUFSIZE, fp_part);
              						encoding(data_array,len_read, key1);
              						if((bytes_tot = send(socketfd[i],data_array,len_read, 0)) < 0)
							{
                						printf("Error in sending to socket for the Server:%s for port: %d\n", parse.dfs[i], *parse.port_num[i]);
              						}
              						temp_data++;
              						bzero(data_array, MAXBUFSIZE);
              						recv(socketfd[i],data_array, sizeof(data_array), 0);
							if(temp_data == (sum_parts))
							{
                						bzero(data_array, MAXBUFSIZE);
                						len_read = fread(data_array, 1, (part_len%MAXBUFSIZE), fp_part);
                						encoding(data_array,len_read, key1);
                						if((bytes_tot = send(socketfd[i],data_array,len_read, 0)) < 0)
								{
                  							printf("Error in sending to socket for the Server:%s for port: %d\n", parse.dfs[i], *parse.port_num[i]);
                						}
                						bzero(data_array, MAXBUFSIZE);
                						recv(socketfd[i],data_array, sizeof(data_array), 0);
              						}
           					 }while(temp_data <sum_parts);
            					 temp_data=0;
          				}
          				if(data_map[hash_calc][i][0]==3 || data_map[hash_calc][i][1]==3)
					{
            					if(!strcmp(minifolder, "NONE"))
						{
              						char file_path[128];
              						bzero(file_path, sizeof(file_path));
             					        sprintf(file_path, "Part:3 %s %lu %s", filename,part_len, *parse.username);
              						sock_error_condition(bytes_tot,file_path,socketfd,i);
            					}
            					else
						{
              						char file_path[128];
              						bzero(file_path, sizeof(file_path));
             						sprintf(file_path, "Part:3 %s %lu %s/%s", filename,part_len, *parse.username,minifolder);
              						sock_error_condition(bytes_tot,file_path,socketfd,i);
            					}
						bzero(data_array, MAXBUFSIZE);
            					recv(socketfd[i],data_array, sizeof(data_array), 0);
            					fseek(fp_part, (2*part_len), SEEK_SET);
						do
						{
              						bzero(data_array, MAXBUFSIZE);
              						len_read = fread(data_array, 1, MAXBUFSIZE, fp_part);
              						encoding(data_array,len_read, key1);
              						if((bytes_tot = send(socketfd[i],data_array,len_read, 0)) < 0)
							{
                						printf("Sending to DFS2: %d bytes\n", bytes_tot);
                						printf("error in sending to socket for the server:%s for port: %d\n", parse.dfs[i], *parse.port_num[i]);
							}
							temp_data++;
							bzero(data_array, MAXBUFSIZE);
              						recv(socketfd[i],data_array, sizeof(data_array), 0);
							if(temp_data == (sum_parts))
							{
                						bzero(data_array, MAXBUFSIZE);
                						len_read = fread(data_array, 1, (part_len%MAXBUFSIZE), fp_part);
                						encoding(data_array,len_read, key1);
                						if((bytes_tot = send(socketfd[i],data_array,len_read, 0)) < 0)
								{
                  							printf("Sending to DFS3: %d bytes\n", bytes_tot);
                  							printf("error in sending to socket for the server:%s for port: %d\n", parse.dfs[i], *parse.port_num[i]);
                						}
                						bzero(data_array, MAXBUFSIZE);
                						recv(socketfd[i],data_array, sizeof(data_array), 0);
              						}
            					}while(temp_data<sum_parts);
            					temp_data=0;
          				}
          				if(data_map[hash_calc][i][0]==4 || data_map[hash_calc][i][1]==4)
					{
            					if(strcmp(minifolder, "NONE")==0)
						{
              						char file_path[128];
              						bzero(file_path, sizeof(file_path));
              						sprintf(file_path, "Part:4 %s %lu %s", filename,part_len, *parse.username);
              						sock_error_condition(bytes_tot,file_path,socketfd,i);
           					 }
           					else
						{
              						char file_path[128];
              						bzero(file_path, sizeof(file_path));
              						sprintf(file_path, "Part:4 %s %lu %s/%s", filename,part_len, *parse.username,minifolder);
              						sock_error_condition(bytes_tot,file_path,socketfd,i);
            					}
						bzero(data_array, MAXBUFSIZE);
            					recv(socketfd[i],data_array, sizeof(data_array), 0);
            					fseek(fp_part, (3*part_len), SEEK_SET);
						do
						{
              						bzero(data_array, MAXBUFSIZE);
              						len_read = fread(data_array, 1, MAXBUFSIZE, fp_part);
              						encoding(data_array,len_read, key1);
              						if((bytes_tot = send(socketfd[i],data_array,len_read, 0)) < 0)
							{
                						printf("Sending to DFS4: %d bytes\n", bytes_tot);
                						printf("error in sending to socket for the server:%s for port: %d\n", parse.dfs[i], *parse.port_num[i]);
              						}
							printf("Sending to DFS4: %d bytes\n", bytes_tot);
							temp_data++;
              						bzero(data_array, MAXBUFSIZE);
              						recv(socketfd[i],data_array, sizeof(data_array), 0);
							if(temp_data == (sum_parts))
							{
                						bzero(data_array, MAXBUFSIZE);
                						len_read = fread(data_array, 1, (part4_len%MAXBUFSIZE), fp_part);
                						encoding(data_array,len_read, key1);
                						if((bytes_tot = send(socketfd[i],data_array,len_read, 0)) < 0)
								{
                  							printf("Sending to DFS4: %d bytes\n", bytes_tot);
                  							printf("error in sending to socket for the server:%s for port: %d\n", parse.dfs[i], *parse.port_num[i]);
                						}
                						bzero(data_array, MAXBUFSIZE);
                						recv(socketfd[i],data_array, sizeof(data_array), 0);
              						}
            					}while(temp_data<sum_parts);
            					temp_data=0;
          				}
        			}
      			}
     			else if(strcmp(name_command, "list")==0)
			{
				bzero(data_array, MAXBUFSIZE);
        			bytes_tot = 0;
        			if((bytes_tot = recv(socketfd[i],data_array, sizeof(data_array), 0))<0)
				{
          				perror("Error: \n");
        			}
				else printf("\n/**********\n %s \n**********/\n",data_array);
        			char msg[] = "Synq";
        			if((bytes_tot = send(socketfd[i], msg, strlen(msg), 0)) < 0)
				{
		  			 perror("error in sending");
				}
        			if(!(strcmp(data_array, "User Exists")))
				{
          				printf("User Exists: server can list file\n");
          				bzero(data_array, MAXBUFSIZE);
          				bytes_tot = 0;
          				if((bytes_tot = recv(socketfd[i],data_array, sizeof(data_array), 0))<0)
					{
            					perror("Error: \n");
          				}
					if(!strcmp(filename, "NONE"))
					{
            					char file_path[128];
            					bzero(file_path, sizeof(file_path));
            					sprintf(file_path, "%s", *parse.username);
            					sock_error_condition(bytes_tot,file_path,socketfd,i);
           					bzero(data_array, MAXBUFSIZE);
            					bytes_tot = 0;
            					if((bytes_tot = recv(socketfd[i],data_array, sizeof(data_array), 0))<0)
						{
              						perror("error in receiving");
           					}
          				}
          				else
					{
            					char file_path[128];
           					bzero(file_path, sizeof(file_path));
            					sprintf(file_path, "%s/%s", *parse.username, filename);
            					sock_error_condition(bytes_tot,file_path,socketfd,i);
            					bzero(data_array, MAXBUFSIZE);
            					bytes_tot = 0;
            					if((bytes_tot = recv(socketfd[i],data_array, sizeof(data_array), 0))<0)
						{
              						perror("error in receiving");
						 }
					}
          				bzero(data_array, MAXBUFSIZE);
          				bytes_tot = 0;
          				if((bytes_tot = recv(socketfd[i],data_array, sizeof(data_array), 0))<0)
					{
            					perror("Error: \n");
           					exit(1);
          				}
          				sock_error_condition(bytes_tot,data_array,socketfd,i);
					FILE *fptr;
          				fptr = fopen("list_file_temp", "ab");
          				if(fptr==0)
					{
            					printf("temporary list file creation error\n");
            					return -1;
          				}
          				fwrite(data_array, 1,bytes_tot, fptr);
          				fclose(fptr);
        			}
      			}
      			else if(strcmp(name_command, "get")==0)
			{
       				char msg[] = "Synq message";
        			printf("get the file: \"%s\" from the server.\n", filename);
        			bzero(data_array, MAXBUFSIZE);
        			bytes_tot = 0;
        			if((bytes_tot = recv(socketfd[i],data_array, sizeof(data_array), 0))<0)
				{
          				perror("error in receiving");
       				}
				else printf("\n/**********\n%s\n**********/\n",data_array);
				if(!(strcmp(data_array, "User Exists")))
				{
          				printf("User Exists: Server  is ready to SEND File\n");
					if(strcmp(minifolder, "NONE")!=0)
					{
            					char file_path[128];
            					bzero(file_path, sizeof(file_path));
            					sprintf(file_path, "%s %s/%s", filename, *parse.username,minifolder);
            					sock_error_condition(bytes_tot,file_path,socketfd,i);
						
          				}
          				else
					{
						char file_path[128];
           					bzero(file_path, sizeof(file_path));
            					sprintf(file_path, "%s %s", filename, *parse.username);
            					sock_error_condition(bytes_tot,file_path,socketfd,i);
           					
          				}
					bzero(file_name1, sizeof(file_name1));
          				sprintf(file_name1, ".%s.1", filename);
					bzero(file_name2, sizeof(file_name2));
          				sprintf(file_name2, ".%s.2", filename);
					bzero(file_name3, sizeof(file_name3));
          				sprintf(file_name3, ".%s.3", filename);
					bzero(file_name4, sizeof(file_name4));
          				sprintf(file_name4, ".%s.4", filename);
					bzero(data_array, MAXBUFSIZE);
          				recv(socketfd[i],data_array, sizeof(data_array), 0);
          				printf("Part status: %s \n", data_array);
					if((strstr(data_array, "YES") != NULL) && (strstr(data_array, file_name1) != NULL) && (flag_1==0))
					{
            					if((bytes_tot = send(socketfd[i], "SEND", strlen("SEND"), 0)) < 0)
						{
              						perror("error in sending part: \n");
					        }
            				 	FILE* fp;
            					fp = fopen(file_name1, "ab");
            					if(fp!=0)
						{
              						
							while(1)
							{
                						bzero(data_array, MAXBUFSIZE);
                						if((bytes_tot = recv(socketfd[i],data_array, sizeof(data_array), 0)) < 0)
								{
                  							printf("Can't reading from the socket\n");
               							}
								printf("Read data length %d\n", bytes_tot);
								encoding(data_array,bytes_tot, key1);
                						int write_length = fwrite(data_array, 1,bytes_tot, fp);
								if(write_length != MAXBUFSIZE)
								{
                  							break;
                						}
                						if((bytes_tot = send(socketfd[i], msg, strlen(msg), 0)) < 0)
								{
                 							 perror("error in sending");
                						}

             						 }
              						 flag_1 = 1;
              						 fclose(fp);
							
            					}
            					else
						{
							printf("Error Opening : %s\n", file_name1);
              						perror("error data");
              						
						 }
          				  }
          				  else if((strstr(data_array, "YES") != NULL) && (strstr(data_array, file_name1) != NULL) && (flag_1==1))
					  {
            				  	if((bytes_tot = send(socketfd[i], "DONT", strlen("DONT"), 0)) < 0)
						{
              						printf("In Part Send\n");
              						perror("error");
            					}
            					sleep(1);
          				}
					bzero(data_array, MAXBUFSIZE);
          				strcpy(data_array, "Last Synq message in part-1");
          				sock_error_condition(bytes_tot,data_array,socketfd,i);
          				bzero(data_array, MAXBUFSIZE);
          				recv(socketfd[i],data_array, sizeof(data_array), 0);
          				printf("Part status: %s \n", data_array);
					if((strstr(data_array, "YES") != NULL) && (strstr(data_array, file_name2) != NULL) && (flag_2==0))
					{
            					if((bytes_tot = send(socketfd[i], "SEND", strlen("SEND"), 0)) < 0)
						{
              						printf("In Part Send\n");
              						perror("error in sending");
						}
            					FILE* fp;
            					fp = fopen(file_name2, "ab");
            					if(fp!=0)
						{
              						while(1)
							{
                						bzero(data_array, MAXBUFSIZE);
									if((bytes_tot = recv(socketfd[i],data_array, sizeof(data_array), 0)) < 0)
									{
                  								printf("Can't read from the socket\n");
									}
									printf("Read data length %d\n", bytes_tot);
									encoding(data_array,bytes_tot, key1);
									int write_length = fwrite(data_array, 1,bytes_tot, fp);
									if(write_length != MAXBUFSIZE)
									{
                  								break;
                							}
                							if((bytes_tot = send(socketfd[i], msg, strlen(msg), 0)) < 0)
									{
                  								perror("error in reading");
                							}

              						}
              						flag_2 = 1;
              						fclose(fp);
							
            					}
            					else
						{
              						printf("Error Opening : %s\n", file_name2);
              						perror("error in opening file \n");
            					}
         				}
          				else if((strstr(data_array, "YES") != NULL) && (strstr(data_array, file_name2) != NULL) && (flag_2==1))
					{
            					if((bytes_tot = send(socketfd[i], "DONT", strlen("DONT"), 0)) < 0)
						{
              						printf("In Part Send\n");
              						perror("error in sending");
            					}
            					sleep(1);
          				}
          				bzero(data_array, MAXBUFSIZE);
          				strcpy(data_array, "Last Synq message in part2");
          				sock_error_condition(bytes_tot,data_array,socketfd,i);
          				bzero(data_array, MAXBUFSIZE);
          				recv(socketfd[i],data_array, sizeof(data_array), 0);
          				printf("Part Status: %s \n", data_array);
					if((strstr(data_array, "YES") != NULL) && (strstr(data_array, file_name3) != NULL) && (flag_3==0))
					{
           					 if((bytes_tot = send(socketfd[i], "SEND", strlen("SEND"), 0)) < 0)
						 {
              					 	printf("In Part Send\n");
              						perror("error in sening");
            					 }
            					 FILE* fp;
            					 fp = fopen(file_name3, "ab");
            					 if(fp!=0)
						 {
              					 	while(1)
							{
               							 bzero(data_array, MAXBUFSIZE);
                						 if((bytes_tot = recv(socketfd[i],data_array, sizeof(data_array), 0)) < 0)
								 {
                  						  	printf("Cant read from the socket\n");
                						 }
								 printf("Read length %d\n", bytes_tot);
								 encoding(data_array,bytes_tot, key1);
                						 int write_length = fwrite(data_array, 1,bytes_tot, fp);
								 if(write_length != MAXBUFSIZE)
								 {
                  						  	break;
               							 }
                						 if((bytes_tot = send(socketfd[i], msg, strlen(msg), 0)) < 0)
								{
                  							perror("error sending");
                						}

              						}
              						flag_3 = 1;
              						fclose(fp);
							
            					 }
            					 else
						 {
							printf("Error Opening : %s\n", file_name3);
              						perror("error in file");
              						
            					}
         				}
          				else if((strstr(data_array, "YES") != NULL) && (strstr(data_array, file_name3) != NULL) && (flag_3==1))
					{
            					if((bytes_tot = send(socketfd[i], "DONT", strlen("DONT"), 0)) < 0)
						{
              						perror("error sending");
            					}
            					sleep(1);
          				}
          				bzero(data_array, MAXBUFSIZE);
          				strcpy(data_array, "Last Synq message in part3");
          				sock_error_condition(bytes_tot,data_array,socketfd,i);
				        bzero(data_array, MAXBUFSIZE);
          				recv(socketfd[i],data_array, sizeof(data_array), 0);
          				printf("Part Status: %s \n", data_array);
					if((strstr(data_array, "YES") != NULL) && (strstr(data_array, file_name4) != NULL) && (flag_4==0))
					{
            					if((bytes_tot = send(socketfd[i], "SEND", strlen("SEND"), 0)) < 0)
						{
              						perror("error sending \n");
           					 }
            					FILE* fp;
            					fp = fopen(file_name4, "ab");
            					if(fp!=0)
						{
              						while(1)
							{
                						bzero(data_array, MAXBUFSIZE);
								if((bytes_tot = recv(socketfd[i],data_array, sizeof(data_array), 0)) < 0)
								{
                  							printf("Cant read from the socket\n");
								}
								printf("Read data length %d\n", bytes_tot);
                						encoding(data_array,bytes_tot, key1);
								int write_length = fwrite(data_array, 1,bytes_tot, fp);
								if(write_length != MAXBUFSIZE)
								{
                  							break;
                						}
                						if((bytes_tot = send(socketfd[i], msg, strlen(msg), 0)) < 0)
								{
                  							perror("sending error");
                						}

              						}
              						flag_4 = 1;
              						fclose(fp);
							
            					}
            					else
						{
              						printf("Error Opening : %s\n", file_name4);
              						perror("error sending");
            					}
         				}
          				else if((strstr(data_array, "YES") != NULL) && (strstr(data_array, file_name4) != NULL) && (flag_4==1))
					{
            					if((bytes_tot = send(socketfd[i], "DONT", strlen("DONT"), 0)) < 0)
						{
              						perror("sending error");
            					}
           					sleep(1);
          				}
          				bzero(data_array, MAXBUFSIZE);
          				strcpy(data_array, "Last Synq message in part4");
          				sock_error_condition(bytes_tot,data_array,socketfd,i);
        			}
        			else
				{
         				 printf("username and password confirm");
        			}
      			}
			else if(strcmp(name_command, "mkdir")==0)
			{
       				printf("\n**directory listing**\n");
				printf("making subfolder: \"%s\" on the server.\n", filename);
        			bzero(data_array, MAXBUFSIZE);
        			bytes_tot = 0;
       				if((bytes_tot = recv(socketfd[i],data_array, sizeof(data_array), 0))<0)
				{
         				perror("error in receiving");
        			}
				else printf("\n/**********\n%s\n**********/\n", data_array);
				if((strcmp(data_array, "User Exists"))==0)
				{
          				printf("User Exists: subfolder can be made\n");
					if((bytes_tot = send(socketfd[i], filename, strlen(filename), 0)) < 0)
					{
           					printf("Sending to DFS%d: %d bytes\n", i+1, bytes_tot);
            					printf("error in sending  data to socket for the server:%s for port: %d\n", parse.dfs[i], *parse.port_num[i]);
          				}

        			}
     			}
    		}
		if(strcmp(name_command, "list")==0)
		{
      			system("sort list_file_temp | uniq > list_file");
			int tracker = 0;
			FILE *fp_duplicate;
      			char get_line[MAXBUFSIZE], get_line_duplicate[MAXBUFSIZE];
      			char *c_get;
      			char list_filename[128], list_filename_duplicate[128];
      			FILE *fp;
			fp = fopen("list_file", "r");
      			if(fp==0)
			{
        			printf("error reading file\n");
        			return -1;
      			}
      			if(fp!=0)
			{
        			printf("\n\n*****Output of list from servers is****** \n");
        			fgets(get_line, sizeof(get_line), fp);
        			if((c_get = strstr((char *)get_line, ".")))
				{
		  			bzero(list_filename, sizeof(list_filename));
		  			strncpy(list_filename, get_line+strlen("."), strlen(get_line)-4);
					fp_duplicate = fp;
		  			while(fgets(get_line_duplicate, sizeof(get_line_duplicate), fp_duplicate))
					{
		    				if((c_get = strstr((char *)get_line_duplicate, ".")))
						{
		      					bzero(list_filename_duplicate, sizeof(list_filename_duplicate));
		      					strncpy(list_filename_duplicate, get_line_duplicate+strlen("."), strlen(get_line_duplicate)-4);
			      				if(strcmp(list_filename_duplicate, list_filename)!=0)
							{
								if(tracker!=3)
								{
									printf("\t%s [incomplete]\n",list_filename);
				  					
			       					}
								else
								{
				  					printf("\t%s [complete]\n",list_filename);
								}
								tracker = 0;
								bzero(list_filename, sizeof(list_filename));
								strcpy(list_filename, list_filename_duplicate);
								
							}
			      				else
							{
								tracker = tracker + 1;
			      				}
		    				}
		  			}
		  			if(tracker==3)
					{
		    				printf("\t%s [complete]\n",list_filename);
		  			}
		  			else
					{
		    				printf("\t%s [incomplete]\n",list_filename);
					}
					tracker = 0;
		  			bzero(list_filename, sizeof(list_filename));
		  			strcpy(list_filename, list_filename_duplicate);
        				}
			}
      			remove("list_file");
      			remove("list_file_temp");
 }
 else if(strcmp(name_command, "get")==0)
{
      if((flag_1!=1) && (flag_2!=1) && (flag_3!=1) && (flag_4!=1))
	{
        	printf("\n\n\n\n*Incomplete file in get*\n\n\n");
        }
       else
	{
        	
		char concat_parts_rm[264], concat_command[264];
        	bzero(concat_parts_rm, sizeof(concat_parts_rm));
        	bzero(concat_command, sizeof(concat_command));
        	sprintf(concat_parts_rm, "rm %s %s %s %s", file_name1, file_name2, file_name3, file_name4);
		sprintf(concat_command, "cat %s %s %s %s > %s", file_name1, file_name2, file_name3, file_name4, filename);
		system(concat_command);
        	system(concat_parts_rm);
      	}
    }
  }
return 0;
}
