/******************************************
* PA4
* dfs.c
* Date:  08 December 2018
* Author: Sanika Dongre
******************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <dirent.h>
#define MAXBUFSIZE 1024
typedef struct
{
	char username[4][64];
	char password[4][64];
}data_parsing;

typedef struct
{
	char username[64];
	char password[64];
	char command[32];
	char filename[32];
}data_validation;

/*Function for parsing data*/

int check_data_parsing(data_parsing *match_data, int* data_val, char* config_data)
{
	char check_row[MAXBUFSIZE];	
	FILE *fptr;
	fptr = fopen(config_data, "r");
	if(fptr != 0)
	{
		while(fgets(check_row, sizeof(check_row), fptr) > 0)
		{
			sscanf(check_row, "%s %s", (*match_data).username[*data_val], (*match_data).password[*data_val]);
			printf("Getting the data from configuration file\n");
			printf(" The Username and the password is: %s and %s", (*match_data).username[*data_val],(*match_data).password[*data_val]);
			(*data_val)++;
			
    		}
		return 0;		
		
	}
	else
	{
		perror("Error in opening file");
    		return -1;
  	}
}
/*To find error condition*/
void condition_get_error(char data_array[], int bytes_tot, int socket2)
{
	bytes_tot = send(socket2,data_array, strlen(data_array), 0);
	if(bytes_tot < 0)
	{
		perror("Error in sending data");
	}
}
void error_condition(char data_array[], int bytes_tot, int socket2)
{
	bzero(data_array, MAXBUFSIZE);
	strcpy(data_array, "Recieved Iteration");
	bytes_tot = send(socket2, data_array, strlen(data_array), 0);
	if(bytes_tot < 0)
	{
		perror("Error in sending data");
	}
}
/*Function for get functionality*/

void get_file(char data_array[], int bytes_tot, int socket2, char file_name[])
{       FILE *fptr;
	fptr = fopen(file_name, "r");
	if(fptr!=0)
	{
		
		bzero(data_array, MAXBUFSIZE);
		strcpy(data_array, file_name);
		strcat(data_array, " YES");
		condition_get_error(data_array, bytes_tot, socket2);
		bzero(data_array, MAXBUFSIZE);
		bytes_tot = recv(socket2,data_array,MAXBUFSIZE, 0);
		printf("Data being sent is: %s\n", data_array);
		if(strstr(data_array, "SEND") != NULL)
		{
			while(1)
			{
				bzero(data_array, MAXBUFSIZE);
			      	int read_length = fread(data_array, 1, MAXBUFSIZE, fptr);
			      	printf("Read length of file requested:%d\n", read_length );
				bytes_tot = send(socket2,data_array, read_length, 0);
			      	if(bytes_tot < 0)
				{
			 		printf("Error in writing to the socket");
					fseek(fptr, (-1)*MAXBUFSIZE, SEEK_CUR);
			      	}
			     	if(read_length != MAXBUFSIZE)
				{
					break;
			     	}
				bzero(data_array, MAXBUFSIZE);
			      	bytes_tot = recv(socket2, data_array,MAXBUFSIZE, 0);
			      	printf("data recieved for parts iteration : %s\n", data_array);
			   }
		}
		fclose(fptr);
	}
	else
	{
		bzero(data_array, MAXBUFSIZE);
		strcpy(data_array, file_name);
		strcat(data_array, " NO");
		condition_get_error(data_array, bytes_tot, socket2);
	 }
}
/*Function for put functionality*/
int put_file(char destination_path[], char file_division[], int category_split, char data_array[], int socket2, int bytes_tot, unsigned long int part_size, int parts_iteration)
{
	char dfsdata[148];
	FILE* dfs_file;
	int temp_data=0;
	bzero(dfsdata, 148);
	sprintf(dfsdata, "%s/.%s.%d", destination_path,file_division, category_split);
	printf("%s\n", dfsdata);
	dfs_file = fopen(dfsdata, "ab");
	if(dfs_file==0)
	{
		printf("part number creation error: %d\n", category_split);
		return -1;
	}
	do
	{
		bzero(data_array, MAXBUFSIZE);
		bytes_tot = recv(socket2, data_array,MAXBUFSIZE, 0);
		fwrite(data_array, 1, bytes_tot, dfs_file);
		temp_data++;
		error_condition(data_array, bytes_tot, socket2);
		if(temp_data == (parts_iteration))
		{
			bzero(data_array, MAXBUFSIZE);
			bytes_tot = recv(socket2, data_array,MAXBUFSIZE, 0);
			fwrite(data_array, 1, bytes_tot, dfs_file);
			error_condition(data_array, bytes_tot, socket2);
		}
	}while(temp_data<parts_iteration);
	temp_data = 0;
	category_split = 0;
	part_size= 0;
	bzero(file_division, 128);
	fclose(dfs_file);
}
/*main function*/
int main(int argc, char * argv[])
{
	data_parsing data_extract;
	data_validation *verify = (data_validation*)malloc(sizeof(data_validation)); 
	struct sockaddr_in server_addr, client_addr;
	int data_flag, sock,socket2, category_split = 0, bytes_tot, verify_flag=0, child_fork, obtbytes, i=0, output_put;
	int* data_val = (int*)malloc(sizeof(int));
	unsigned int clientlen = sizeof(client_addr);
	char* config_data;
	unsigned long int part_size = 0;
	char data_array[MAXBUFSIZE], number[32]= {0}, file_division[128], dfsfull[140], destination_path[148], name_directory[148],file_name1[148], file_name2[148], file_name3[148], file_name4[148], filename[148];
	if (argc < 3)
	{
		printf("\nMention the: </DFSn> <portNo>\n");
		exit(1);
	}
	strcpy(number, argv[2]);
	strcpy(dfsfull, argv[1]);
	config_data = (char *)"dfs.conf";
	data_flag = check_data_parsing(&(data_extract), data_val, config_data);
	if(data_flag==-1)
	{
		perror("Error reading config file");
		exit(1);
	}
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if ((sock) < 0)
	{
		perror("Error in creating socket");
	}
	puts("socket successfully created");
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(number));
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("Error in binding socket");
		exit(-1);
	}
	puts("socket binding successfully");
	if(listen(sock, 1024) < 0)
	{
	   perror("socket listening error");
	}
	puts("socket listening successfull");
	while(1)
	{
		if((socket2 = accept(sock, (struct sockaddr *)&client_addr, &clientlen)) < 0)
		{
			perror("socket connection acceptance failed");
			exit(-1);
		}
		puts("connection accepted successfully");
		child_fork = fork();
		if(child_fork==0)
		{
			obtbytes = recv(socket2, verify, sizeof(*verify), 0);
			if(obtbytes == 0)
			{
				perror("client is disconnected");
				exit(0);
			}
			printf("Username and password recieved: %s\t %s Command: %s \n\n", &verify->username[0], &verify->password[0],&verify->command[0]);
	      		for(i=0; i<*data_val; i++)
			{
				if((strcmp((data_extract).username[i], &verify->username[0]))==0)
				{
		  			if((strcmp((data_extract).password[i], &verify->password[0]))!=0)
					{
		    				bzero(data_array, MAXBUFSIZE);
			    			strcpy(data_array, "password error");
						condition_get_error(data_array,bytes_tot, socket2);
			  			break;
		  			}
		  			else
		  			{
			    			
						bzero(data_array, MAXBUFSIZE);
		    				strcpy(data_array, "User Exists");
		    				verify_flag = 1;
						condition_get_error(data_array,bytes_tot,socket2);
		    				break;
		  			}
				}
				else if(i == *data_val-1)
				{
			  		bzero(data_array, MAXBUFSIZE);
			  		strcpy(data_array, "Invalid User: User Does not exist");
					condition_get_error(data_array,bytes_tot,socket2);
			  		break;
			 	}
	      		}
	      		if(verify_flag==1)
			{	
				if(strcmp(&verify->command[0],"get")==0)
				{
					bzero(data_array, MAXBUFSIZE);
					recv(socket2,data_array,MAXBUFSIZE, 0);
					printf("Buffer: %s \n", data_array);
					char filepath[148];
					bzero(filepath, sizeof(filepath));
					bzero(filename, sizeof(filename));
					bzero(file_name1, sizeof(file_name1));
					bzero(destination_path, sizeof(destination_path));
					sscanf(data_array, "%s %s", filename, filepath);
					sprintf(destination_path, ".%s/%s/", argv[1], filepath );
					printf("%s\n", destination_path);
					mkdir(destination_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		       		        sprintf(file_name1, "%s.%s.1",destination_path, filename);
					get_file(data_array, bytes_tot, socket2, file_name1);
					bzero(data_array, MAXBUFSIZE);
					bytes_tot = recv(socket2,data_array,MAXBUFSIZE, 0);
					printf("Receiving data 1:%s\n", data_array);
					bzero(file_name2, sizeof(file_name2));
					sprintf(file_name2, "%s.%s.2",destination_path, filename);
					get_file(data_array, bytes_tot, socket2, file_name2);
					bzero(data_array, MAXBUFSIZE);
					bytes_tot = recv(socket2, data_array,MAXBUFSIZE, 0);
					printf("Receiving data 2:%s\n", data_array);
					bzero(file_name3, sizeof(file_name3));
					sprintf(file_name3, "%s.%s.3",destination_path, filename);
					get_file(data_array,bytes_tot, socket2, file_name3);
					bzero(data_array, MAXBUFSIZE);
					bytes_tot = recv(socket2,data_array,MAXBUFSIZE, 0);
					printf("Receiving data 3:%s\n", data_array);
					bzero(file_name4, sizeof(file_name4));
					sprintf(file_name4, "%s.%s.4",destination_path, filename);
					get_file(data_array, bytes_tot, socket2, file_name4);
					bzero(data_array, MAXBUFSIZE);
					bytes_tot = recv(socket2,data_array,MAXBUFSIZE, 0);
					printf("Recieving data 4:%s\n", data_array);

				}
				else if(strcmp(&verify->command[0], "put")==0)
				{
					char filepath[148];
			  		bzero(data_array, MAXBUFSIZE);
			  		recv(socket2, data_array,MAXBUFSIZE, 0);
			  		printf("%s This is the data array\n", data_array);
			  		bzero(filepath, sizeof(filepath));
				 	sscanf(data_array, "%*[^:]%*c%d %s %lu %s", &category_split,file_division, &part_size, filepath);
					bzero(data_array, MAXBUFSIZE);
			  		strcpy(data_array, "Recieved Iteration");
					condition_get_error(data_array,bytes_tot,socket2);
					int parts_iteration = (part_size/MAXBUFSIZE);
			  		bzero(destination_path, sizeof(destination_path));
			  		sprintf(destination_path, ".%s/%s", argv[1], filepath );
			  		printf("THe destination path is:%s\n", destination_path);
			  		mkdir(destination_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			  		output_put = put_file(destination_path,file_division,category_split,data_array, socket2,bytes_tot,part_size, parts_iteration);
			  		bzero(data_array, MAXBUFSIZE);
			  		recv(socket2, data_array,MAXBUFSIZE, 0);
			 		printf("%s: The data array****\n", data_array);
					bzero(filepath, sizeof(filepath));
					sscanf(data_array, "%*[^:]%*c%d %s %lu %s", &category_split, file_division, &part_size, filepath);
					parts_iteration = (part_size/MAXBUFSIZE);
			  		printf("Total number of parts %d\n", parts_iteration);
					bzero(data_array, MAXBUFSIZE);
			  		strcpy(data_array, "Recieved Iteration");
					condition_get_error(data_array,bytes_tot,socket2);
					bzero(destination_path, sizeof(destination_path));
			  		sprintf(destination_path, ".%s/%s", argv[1], filepath );
			  		printf("The path directory is:%s\n", destination_path);
			  		mkdir(destination_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			  		output_put = put_file(destination_path,file_division,category_split,data_array, socket2,bytes_tot,part_size, parts_iteration);
					printf("\nTHe file is put on the server\n\n");
	       		   	}
				else if(strcmp(&verify->command[0],"list")==0) //for listing files
			   	{
				   	
					DIR *present_dir;
					char msg_val[] = "List synq";
					char filepath[148];
					bzero(data_array, MAXBUFSIZE);
					bzero(filepath, sizeof(filepath));
					recv(socket2, data_array,MAXBUFSIZE, 0);
					bytes_tot = send(socket2, msg_val, strlen(msg_val), 0);
		  			struct dirent *struct_dir;
					bzero(data_array, MAXBUFSIZE);
					recv(socket2, data_array,MAXBUFSIZE, 0);
					sscanf(data_array, "%s", filepath);
					bytes_tot = send(socket2, msg_val, strlen(msg_val), 0);
					bzero(destination_path, sizeof(destination_path));
					sprintf(destination_path, ".%s/%s", argv[1], filepath);
		       		        printf("The path is: %s\n", destination_path);
					mkdir(destination_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
					present_dir = opendir(destination_path);
		  			if(present_dir != NULL)
					{
						bzero(data_array,MAXBUFSIZE);
		  				while((struct_dir = readdir(present_dir)) != NULL)
						{
			    				if(!strcmp(struct_dir->d_name, ".") || !strcmp(struct_dir->d_name, "..") || (struct_dir->d_name[0] != '.'))
							{
			      					continue;
			    				}
			    				else
							{
			      					strcat(data_array, struct_dir->d_name);
		  						strcat(data_array, "\n");
			   				 }
		  				}
					   	if(strlen(data_array) == 0)
					   	{
		 			  	 	
							 bzero(data_array, MAXBUFSIZE);
			  				 sprintf(data_array, "files not present on server%s  \n", argv[1]);
			  				 printf("list sending:%s\n", data_array);
							 condition_get_error(data_array,bytes_tot,socket2);
	       				   	}
					   	else
					  	 {
			 			  	printf("List sending(NOT NULL):%s\n", data_array);
							condition_get_error(data_array,bytes_tot,socket2);
		       				  }
						   bzero(data_array, MAXBUFSIZE);
						   recv(socket2, data_array,MAXBUFSIZE, 0);
		  				
		  			}
		  			else
					{
		  				char msg_val[] = "Unable to read Directory";
			  			printf("%s\n", msg_val);
						bytes_tot = send(socket2, msg_val, strlen(msg_val), 0);
		  				perror("Error in reading the directory");
			  			continue;
	  				}
	  			}
	      			else if(!strcmp(&verify->command[0],"mkdir")) //for performing mkdir functionality
				{
					bzero(name_directory, sizeof(name_directory));
					bzero(data_array, MAXBUFSIZE);
					recv(socket2,data_array,MAXBUFSIZE, 0);
					printf("request for subdirectory made: %s \n", data_array);
					strcpy(name_directory, data_array);
					bzero(destination_path, sizeof(destination_path));
					sprintf(destination_path, ".%s/%s/%s", argv[1], &verify->username[0], name_directory);
				   	mkdir(destination_path, S_IRWXU | S_IRWXG | S_IROTH |S_IXOTH);
					printf("Sub directory is made %s\n", name_directory);
				}
		    	    }
	  		}
	   	 }
	    return 0;
}
