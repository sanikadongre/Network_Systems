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
int main(int argc, char * argv[])
{
	data_parsing data_extract;
	data_validation *verify = (data_validation*)malloc(sizeof(data_validation)); 
	struct sockaddr_in server_addr, client_addr;
	int data_flag, sock,socket2, category_split = 0, bytes_tot, verify_flag=0, child_fork, obtbytes, i=0;
	int* data_val = (int*)malloc(sizeof(int));
	unsigned int clientlen = sizeof(client_addr);
	char* config_data;
	unsigned long int part_size = 0;
	char data_array[MAXBUFSIZE], number[32]= {0}, file_division[128], dfsfull[140], dfsdata[148], destination_path[148];
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
						bytes_tot = send(socket2, data_array, strlen(data_array), 0);
			    			if(bytes_tot < 0)
						{
			      				printf("n bytes error%d\n", bytes_tot);
			      				
			   		        }
			  			break;
		  			}
		  			else
		  			{
			    			
						bzero(data_array, MAXBUFSIZE);
		    				strcpy(data_array, "User Exists");
		    				verify_flag = 1;
						bytes_tot = send(socket2, data_array, strlen(data_array), 0);
		    				if(bytes_tot < 0)
						{
		      					printf("existing %d\n", bytes_tot);
		      					
						}
		    				break;
		  			}
				}
				else if(i == *data_val-1)
				{
			  		bzero(data_array, MAXBUFSIZE);
			  		strcpy(data_array, "Invalid User: User Does not exist");
					bytes_tot = send(socket2, data_array, strlen(data_array), 0);
			  		if(bytes_tot < 0)
					{
			    			printf("invalid user error%d\n", bytes_tot);
			    			
			  		}
			  		break;
			 	}
	      		}
	      		if(verify_flag==1)
			{	
				if(strcmp(&verify->command[0], "put")==0)
				{
			  		bzero(data_array, MAXBUFSIZE);
			  		recv(socket2, data_array, sizeof(data_array), 0);
			  		printf("%s This is the data array\n", data_array);
					char filepath[128];
			  		bzero(filepath, sizeof(filepath));
				 	sscanf(data_array, "%*[^:]%*c%d %s %lu %s", &category_split,file_division, &part_size, filepath);
					bzero(data_array, MAXBUFSIZE);
			  		strcpy(data_array, "Recieved Iteration");
					bytes_tot = send(socket2, data_array, strlen(data_array), 0);
			 	        if(bytes_tot < 0)
					{
			   			printf("In Synq Send()\n");
			    			perror("output error");
			  		}
					int parts_iteration = (part_size/MAXBUFSIZE);
			  		int temp = 0;
			  		bzero(destination_path, sizeof(destination_path));
			  		sprintf(destination_path, ".%s/%s", argv[1], filepath );
			  		printf("THe destination path is:%s\n", destination_path);
			  		mkdir(destination_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
			  		bzero(dfsdata, sizeof(dfsdata));
			  		sprintf(dfsdata, "%s/.%s.%d", destination_path,file_division, category_split);
			  		printf("%s \n", dfsdata );
			  		FILE* dfs_file;
			  		dfs_file = fopen(dfsdata, "ab");
			  		if(dfs_file==0)
			  		{
						printf("Error creating part number: %d\n", category_split);
			  			return -1;
			 	    	}
		  			do
					{
			    			bzero(data_array, MAXBUFSIZE);
			    			bytes_tot = recv(socket2, data_array, sizeof(data_array), 0);
			    			fwrite(data_array, 1, bytes_tot, dfs_file);
			    			temp++;
			    			bzero(data_array, MAXBUFSIZE);
			    			strcpy(data_array, "Recieved Iteration");
						bytes_tot = send(socket2, data_array, strlen(data_array), 0);
			    			if(bytes_tot < 0)
						{
			      				printf("In Synq Send()\n");
			      				perror("Error: \n");
			   			 }
			   			 if(temp == (parts_iteration))
						{
			      				bzero(data_array, MAXBUFSIZE);
			      				bytes_tot = recv(socket2, data_array, sizeof(data_array), 0);
			      				fwrite(data_array, 1, bytes_tot, dfs_file);
							bzero(data_array, MAXBUFSIZE);
			      				strcpy(data_array, "Recieved Iteration");
							bytes_tot = send(socket2, data_array, strlen(data_array), 0);
			      				if(bytes_tot < 0)
							{
								printf("In Synq Send()\n");
								perror("Error: \n");
			     			 	}
		    		 	  	}
			  		    }while(temp<parts_iteration);
					    temp=0;
			  		    category_split = 0;
			  		    part_size = 0;
					    bzero(file_division, sizeof(file_division));
			  		    fclose(dfs_file);
			  		    bzero(data_array, MAXBUFSIZE);
			  		    recv(socket2, data_array, sizeof(data_array), 0);
			 		    printf("%s ****\n", data_array);
					    bzero(filepath, sizeof(filepath));
					    sscanf(data_array, "%*[^:]%*c%d %s %lu %s", &category_split, file_division, &part_size, filepath);
					    parts_iteration = (part_size/MAXBUFSIZE);
			  		    printf("********** Number of iterations: %d **********\n", parts_iteration);
					    bzero(data_array, MAXBUFSIZE);
			  		    strcpy(data_array, "Recieved Iteration");
					    bytes_tot = send(socket2, data_array, strlen(data_array), 0);
			  		    if(bytes_tot < 0)
					    {
			   		   	 printf("In Synq Send()\n");
			    			 perror("Error: \n");
			  		    }
					    bzero(destination_path, sizeof(destination_path));
			  		    sprintf(destination_path, ".%s/%s", argv[1], filepath );
			  		    printf("Path Directory:%s\n", destination_path);
			  		    mkdir(destination_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
					    bzero(dfsdata, sizeof(dfsdata));
			  		    sprintf(dfsdata, "%s/.%s.%d", destination_path,file_division, category_split);
			  		    printf("%s\n", dfsdata);
			  		    dfs_file = fopen(dfsdata, "ab");
			  		    if(dfs_file==0)
					    {
			    		    	printf("Error creating part number: %d\n", category_split);
						return -1;
			  		    }
			  		    do
					    {
			    		    	bzero(data_array, MAXBUFSIZE);
			    			bytes_tot = recv(socket2, data_array, sizeof(data_array), 0);
			    			fwrite(data_array, 1, bytes_tot, dfs_file);
			    			temp++;
			    			bzero(data_array, MAXBUFSIZE);
			    			strcpy(data_array, "Recieved Iteration");
						bytes_tot = send(socket2, data_array, strlen(data_array), 0);
			    			if(bytes_tot < 0)
						{
			     				printf("In Synq Send()\n");
			      				perror("Error: \n");
			   			}
			    			if(temp == (parts_iteration))
						{
			      				bzero(data_array, MAXBUFSIZE);
			      				bytes_tot = recv(socket2, data_array, sizeof(data_array), 0);
			      				fwrite(data_array, 1, bytes_tot, dfs_file);
			      				bzero(data_array, MAXBUFSIZE);
			      				strcpy(data_array, "Recieved Iteration");
							bytes_tot = send(socket2, data_array, strlen(data_array), 0);
			      				if(bytes_tot < 0)
							{
								printf("In Synq Send()\n");
								perror("Error: \n");
			      				}
			    			   }
			  			}while(temp<parts_iteration);
						temp = 0;
			  			category_split = 0;
			  			part_size= 0;
			  			bzero(file_division, sizeof(file_division));
			  			fclose(dfs_file);
						printf("\n********PUT operation Complete********\n\n");
	       		   	}
	      		   	else if(strcmp(&verify->command[0],"list")==0)
			   	{
				   	bzero(data_array, MAXBUFSIZE);
					char msg[] = "List synq";
					recv(socket2, data_array, sizeof(data_array), 0);
					bytes_tot = send(socket2, msg, strlen(msg), 0);
		  			DIR *current_dir;
		  			struct dirent *struct_dir;
					bzero(data_array, MAXBUFSIZE);
					recv(socket2, data_array, sizeof(data_array), 0);
					char filepath[128];
					bzero(filepath, sizeof(filepath));
					sscanf(data_array, "%s", filepath);
					bytes_tot = send(socket2, msg, strlen(msg), 0);
					bzero(destination_path, sizeof(destination_path));
					sprintf(destination_path, ".%s/%s", argv[1], filepath);
		       		        printf("Path Directory: %s\n", destination_path);
					mkdir(destination_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
					current_dir = opendir(destination_path);
		  			if(current_dir == NULL)
					{
		  				char msg[] = "Unable to read Directory";
			  			printf("%s\n", msg );
						bytes_tot = send(socket2, msg, strlen(msg), 0);
		  				perror("Error");
			  			continue;
		  			}
		  			else
					{
		  				bzero(data_array, sizeof(data_array));
		  				while((struct_dir = readdir(current_dir)) != NULL)
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
					   	if(strlen(data_array) != 0)
					   	{
		 			  	 	printf("Buffer Sending In list(NOT NULL):%s\n", data_array);
							bytes_tot = send(socket2, data_array, strlen(data_array), 0);
		  				 	if(bytes_tot < 0)
						 	{
		    						perror("Error: \n");
		 				 	}
	       				   	}
					   	else
					  	 {
			 			  	 bzero(data_array, sizeof(data_array));
			  				 sprintf(data_array, "...NO FILES on SERVER....%s  \n", argv[1]);
			  				 printf("Buffer Sending In list:%s\n", data_array);
							 bytes_tot = send(socket2, data_array, strlen(data_array), 0);
			 				 if(bytes_tot < 0)
							{
			    					perror("Error: \n");
			  				}
		       				   }
						   bzero(data_array, MAXBUFSIZE);
						   recv(socket2, data_array, sizeof(data_array), 0);
	  				}
	  			}
	      			else if(strcmp(&verify->command[0],"get")==0)
				{
					printf("\n************IN GET*********\n");
					bzero(data_array, MAXBUFSIZE);
					recv(socket2,data_array, sizeof(data_array), 0);
					printf("Buffer: %s \n", data_array);
					char filename[128];
					char filepath[128];
					bzero(filepath, sizeof(filepath));
					bzero(filename, sizeof(filename));
					bzero(destination_path, sizeof(destination_path));
					sscanf(data_array, "%s %s", filename, filepath);
					FILE *fp;
					sprintf(destination_path, ".%s/%s/", argv[1], filepath );
					printf("%s\n", destination_path);
					mkdir(destination_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		       		        char filename1[128];
					bzero(filename1, sizeof(filename1));
					sprintf(filename1, "%s.%s.1",destination_path, filename);
					fp = fopen(filename1, "r");
					if(fp==0)
					{
			  			bzero(data_array, MAXBUFSIZE);
			  			strcpy(data_array, filename1);
			  			strcat(data_array, " NO");
			  			if((bytes_tot = send(socket2,data_array, strlen(data_array), 0)) < 0)
						{
			   		       		 perror("Error \n");
			 		        }
					}
					else
					{
			  			bzero(data_array, MAXBUFSIZE);
			  			strcpy(data_array, filename1);
			  			strcat(data_array, " YES");
						bytes_tot = send(socket2,data_array, strlen(data_array), 0);
			  			if( bytes_tot < 0)
						{
			    				perror("Error: \n");
			  			}
			  			bzero(data_array, MAXBUFSIZE);
			 		        bytes_tot = recv(socket2,data_array, sizeof(data_array), 0);
			  			printf("Confirmation Buffer: %s\n", data_array);
			  			if(strstr(data_array, "SEND") != NULL)
						{

			    				while(1)
							{
			      					bzero(data_array, MAXBUFSIZE);
			      					int read_length = fread(data_array, 1, MAXBUFSIZE, fp);
			      					printf("Read Length of file requested:%d\n", read_length );
			      					if((bytes_tot = send(socket2,data_array, read_length, 0) < 0))
								{
									printf("Error: Writing to the socket\n");
									fseek(fp, (-1)*sizeof(data_array), SEEK_CUR);
			      					}
			     			        	if(read_length != MAXBUFSIZE)
								{
									break;
			     			        	}
								bzero(data_array, MAXBUFSIZE);
			      					bytes_tot = recv(socket2, data_array, sizeof(data_array), 0);
			      					printf("buffer recv for parts iteration : %s\n", data_array);
			   				 }
			 			}
						fclose(fp);
					}
					bzero(data_array, MAXBUFSIZE);
					bytes_tot = recv(socket2,data_array, sizeof(data_array), 0);
					printf("Recv after last iteraction:%s\n", data_array);
					char filename2[128];
					bzero(filename2, sizeof(filename2));
					sprintf(filename2, "%s.%s.2",destination_path, filename);
					fp = fopen(filename2, "r");
					if(fp==0)
					{
			  			bzero(data_array, MAXBUFSIZE);
			  			strcpy(data_array, filename2);
			  			strcat(data_array, " NO");
						bytes_tot = send(socket2, data_array, strlen(data_array), 0);
			  			if(bytes_tot < 0)
						{
			    				perror("Error: \n");
			  			}
					}
					else
					{
			  			bzero(data_array, MAXBUFSIZE);
			  			strcpy(data_array, filename2);
			  			strcat(data_array, " YES");
						bytes_tot = send(socket2,data_array, strlen(data_array), 0);
			  			if( bytes_tot < 0)
						{
			    				perror("Error: \n");
			  			}
			  			bzero(data_array, MAXBUFSIZE);
			  			bytes_tot = recv(socket2,data_array, sizeof(data_array), 0);
						if(strstr(data_array, "SEND") != NULL)
						{
		    					while(1)
							{
			      					 bzero(data_array, MAXBUFSIZE);
			     			        	 int read_length = fread(data_array, 1, MAXBUFSIZE, fp);
								 printf("Read Length of file requested:%d\n", read_length);
			      					 if((bytes_tot = send(socket2,data_array, read_length, 0) < 0))
								 {
								 	printf("Error: Writing to the socket\n");
									fseek(fp, (-1)*sizeof(data_array), SEEK_CUR);
			      					 }
			      					 if(read_length != MAXBUFSIZE)
								 {
								  	break;
			      					 }
								 bzero(data_array, MAXBUFSIZE);
			      					 bytes_tot = recv(socket2,data_array, sizeof(data_array), 0);
			      					 printf("buffer recv for parts iteration : %s\n", data_array);
						 	}
		  				}
						fclose(fp);
					}
					bzero(data_array, MAXBUFSIZE);
					bytes_tot = recv(socket2, data_array, sizeof(data_array), 0);
					printf("Recv after last iteraction:%s\n", data_array);
					char filename3[128];
					bzero(filename3, sizeof(filename3));
					sprintf(filename3, "%s.%s.3",destination_path, filename);
					fp = fopen(filename3, "r");
					if(fp==0)
					{
			  			bzero(data_array, MAXBUFSIZE);
			  			strcpy(data_array, filename3);
			  			strcat(data_array, " NO");
						bytes_tot = send(socket2,data_array, strlen(data_array), 0);
			  			if(bytes_tot < 0)
						{
			    				perror("Error: \n");
			  			}
			  			printf("Part 3 does not exist\n");
					}
					else
					{
			  			bzero(data_array, MAXBUFSIZE);
			  			strcpy(data_array, filename3);
			  			strcat(data_array, " YES");
						bytes_tot = send(socket2,data_array, strlen(data_array), 0);
			  			if(bytes_tot < 0)
						{
			    				perror("Error: \n");
			 			}
			  			bzero(data_array, MAXBUFSIZE);
			  			bytes_tot = recv(socket2,data_array, sizeof(data_array), 0);
			  			printf("Confirmation Buffer: %s\n", data_array);
			  			if(strstr(data_array, "SEND") != NULL)
						{
			    				while(1)
							{
			      					bzero(data_array, MAXBUFSIZE);
			      					int read_length = fread(data_array, 1, MAXBUFSIZE, fp);
								printf("Read Length of file requested:%d\n", read_length );
								bytes_tot = send(socket2,data_array, read_length, 0);
			      					if(bytes_tot < 0)
								{
									printf("Error: Writing to the socket\n");
									fseek(fp, (-1)*sizeof(data_array), SEEK_CUR);
			      					}
			      					if(read_length != MAXBUFSIZE)
								{
									break;
			      					}
								bzero(data_array, MAXBUFSIZE);
			      					bytes_tot = recv(socket2,data_array, sizeof(data_array), 0);
			      					printf("buffer recv for parts iteration : %s\n", data_array);
							 }
		  				}
						fclose(fp);
					}
					bzero(data_array, MAXBUFSIZE);
					bytes_tot = recv(socket2,data_array, sizeof(data_array), 0);
					printf("Recv after last iteraction:%s\n", data_array);
					char filename4[128];
					bzero(filename4, sizeof(filename4));
					sprintf(filename4, "%s.%s.4",destination_path, filename);
					fp = fopen(filename4, "r");
					if(fp==0)
					{
			  			bzero(data_array, MAXBUFSIZE);
			  			strcpy(data_array, filename4);
			  			strcat(data_array, " NO");
						bytes_tot = send(socket2,data_array, strlen(data_array), 0);
			  			if(bytes_tot < 0)
						{
					       		 perror("Error: \n");
						}
			  			printf("Part 4 does not exist\n");
					}
					else
					{
		  				bzero(data_array, MAXBUFSIZE);
		  				strcpy(data_array, filename4);
		  				strcat(data_array, " YES");
						bytes_tot = send(socket2,data_array, strlen(data_array), 0);
		  				if((bytes_tot) < 0)
						{
		    					perror("Error: \n");
		 				}
		  				bzero(data_array, MAXBUFSIZE);
		  				bytes_tot = recv(socket2,data_array, sizeof(data_array), 0);
		  				printf("Confirmation Buffer: %s\n", data_array);
		  				if(strstr(data_array, "SEND") != NULL)
						{
		    					while(1)
							{
		      						bzero(data_array, MAXBUFSIZE);
		      						int read_length = fread(data_array, 1, MAXBUFSIZE, fp);
		      						printf("Read Length of file requested:%d\n", read_length );
		      						if((bytes_tot = send(socket2,data_array, read_length, 0) < 0))
								{
		        						printf("Error: Writing to the socket\n");
		        						fseek(fp, (-1)*sizeof(data_array), SEEK_CUR);
		      						}
		      						if(read_length != MAXBUFSIZE)
								{
		        						break;
								}
								bzero(data_array, MAXBUFSIZE);
			      					bytes_tot = recv(socket2,data_array, sizeof(data_array), 0);
			      					printf("buffer recv for parts iteration : %s\n", data_array);
		    					 }
						}
						fclose(fp);
			 		}
					bzero(data_array, MAXBUFSIZE);
					bytes_tot = recv(socket2,data_array, sizeof(data_array), 0);
					printf("Recv after last iteraction:%s\n", data_array);
				}
	      			else if(!strcmp(&verify->command[0],"mkdir"))
				{
					printf("\n************IN MKDIR*********\n");
					bzero(data_array, MAXBUFSIZE);
					recv(socket2,data_array, sizeof(data_array), 0);
					printf("Sub-Directory Request: %s \n", data_array);
					char dir_name[128];
					bzero(dir_name, sizeof(dir_name));
					strcpy(dir_name, data_array);
					bzero(destination_path, sizeof(destination_path));
					sprintf(destination_path, ".%s/%s/%s", argv[1], &verify->username[0], dir_name);
					mkdir(destination_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
					printf("\n\n******* Sub-Directory %s Created *******\n", dir_name);

				}
		    	    }
	  		}
	   	 }
	    return 0;
}
