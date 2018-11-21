/****************************
*Webproxy
*Date: 11/16/2018
*Network Systems PA3
*Sanika Dongre
*****************************/
#include "webproxy.h"

/*******************************
*Shutdown condition loop
*@param: int (socket info)
**********************************/

void shutdown_condition(int proxysock)
{
	shutdown(proxysock, SHUT_RDWR);
	close(proxysock);
}
/***************************************
*Error condition loop
*@params: int (socket info) and char 
*array for error type. It is used to raise
* 400 error for invalid url, method and 
* request and 404 if the server is not found
* and 403 for blocked sites.
****************************************/
void error_condition(int proxysock, uint8_t error_buff[])
{
	int bytes_tot;
	printf("error\n%s\n", error_buff);
	bytes_tot = send(proxysock, error_buff, strlen(error_buff), 0);
	shutdown_condition(proxysock);

}
/*******************************************
*Md5 calculate loop
*To calculate the hash value of requested url
*It uses the <openssl/md5.h> library
*The hash value for the string is returned
*********************************************/
uint8_t* md5_calculate(uint8_t *req_url)
{
	  uint8_t hash_hex[16];
	  int i;
	  MD5_CTX mdContext;
	  MD5_Init (&mdContext);
	  MD5_Update (&mdContext, req_url, strlen(req_url));
	  MD5_Final (hash_hex,&mdContext);
	  char *md5string =(char*) malloc(sizeof(char)*256);
	  for(i = 0; i < 16; i++)
	  {
	    	snprintf(&md5string[i*2], 32, "%02x", hash_hex[i]);
	  }

	  return (char *)md5string;
}
/******************************************************
*Flag condition loop
*It checks the value of the flag
*Used for toggling flags and returning back to the 
*function from where it was requested
********************************************************/

int flag_condition(int flag, FILE *fptr)
{
	if(flag==1)
	 {
		fclose(fptr);
		return 1;
	 }
	 fclose(fptr);
	 return 0;
}
/**************************************************************
*fetch loop
*function used to get the line from the file (data_forbidden) that 
*stores the ip
*address and name of the site
****************************************************************/	
int fetch(char *extract_line, size_t length, FILE *fptr, uint8_t *ip)
{
	int flag = 0, value;
	while((getline(&extract_line, &length, fptr)) != -1)
	{
		if(strstr(extract_line, ip))
		{
		  	flag =1;
		  	break;
		}
	 }
	 value = flag_condition(flag, fptr);

}
/*************************************************************
*Forbid data loop 
*It checks if the website is blocked (if it is present in the
* data_forbidden file.
**************************************************************/
int forbid_data(uint8_t *name, uint8_t *addr_ip_forbid)
{
	  size_t length;
	  int flag = 0, val;
	  FILE *fptr;
	  char* extract_line=NULL;
	  if((fptr = fopen("data_forbidden", "r")) == NULL)
	  {
	  	return 0;
	  }
	  else
	  {
	  	if(strchr(name, ':'))
		{  
	      		sscanf(name, "%[^:]%*c", addr_ip_forbid);
			val = fetch(extract_line, length, fptr, addr_ip_forbid);
	      		
	    	 }
		 else
		 {
	     		val = fetch(extract_line, length, fptr, name);	
	    	 }
	  }
}
/********************************************************************
*Cache file data loop
*This function is used for checking if the cached file is present 
*If the cached file is present data is retrieved from cached file
**********************************************************************/
int cache_file_data(uint8_t *req_url, unsigned long int cache_timeout)
{	
	  uint8_t file_name[MAXBUFSIZE];
	  size_t length;	
	  FILE *fptr;
	  char *md5_url = md5_calculate(req_url); //to calculate the md5 hash value for the requested url
	  char* extract_line=NULL;
	  unsigned long int time_file=0;
	  time_t get_curr;
	  get_curr = time(NULL);  
	  if(md5_url != 0)
	  {
	  	bzero(file_name, sizeof(file_name));
	  	sprintf(file_name, "./cache/%s", md5_url); 
	  	printf("The file name in cache_folder: %s\n", file_name);
	  	if((fptr = fopen(file_name, "r")) == NULL)
	  	{
	    		return 0;
	  	}
	  	else
	  	{
			getline(&extract_line, &length, fptr);
	    		sscanf(extract_line, "%lu", &time_file); 
			if((get_curr - time_file) >= cache_timeout)
	  		{
		      		fclose(fptr);
		      		remove(file_name);
		      		return 0;
	   		}
	  	        else
	   		{
		      		fclose(fptr);
		     		return 1;
	    		}
	   	}
	  }
	  else
	  {
		perror("hash value error");
	  }
	  
}
/*****************************************************
*Cache data loop
* It checks the cache folder to check for names of
* the host and the IP address from the data_file in
*the cache folder. 
*****************************************************/
int cache_data(uint8_t *name, uint8_t *addr_ip)
{
	  
	  uint8_t file_name[MAXBUFSIZE];
	  size_t length;
	  FILE* fptr;
	  char* extract_line=NULL;
	  int flag=0, value;
	  bzero(file_name, sizeof(file_name));
	  sprintf(file_name, "./cache/data_file");
	  if((fptr = fopen(file_name, "r")) == NULL)
	  {

		return 0;
	  }
	  else
	  { 
	    	while((getline(&extract_line, &length, fptr)) != -1)
		{
		      	if(strstr(extract_line, name))
			{
				sscanf(extract_line, "%*[^ ]%*c%s", addr_ip);
				flag =1;
				break;
	      		}
	    	}
	    	value = flag_condition(flag, fptr); //function call to flag condition loop
	  }
}
/********************************************************
*Prefetch data loop. It is used for prefetching the links
* that are embedded within a link and store them on a cache
*********************************************************/
int prefetch_data(uint8_t* addr_ip_prefetch, uint8_t* name_file, uint8_t* name, uint8_t* num)
{
	  
	  uint8_t req_url[MAXBUFSIZE], url_fetch[MAXBUFSIZE], server_req[MAXBUFSIZE], buffer[MAXBUFSIZE], file_cache[64];
	  size_t length;
	  FILE * fptr;
	  FILE * fdptr;
	  char* extract_line = NULL;
	  char * ret_st_opt = NULL;
	  char* md5_url = NULL;
	  char* url_full;
	  int bytes_tot = 0, pre_sock;
	  time_t get_curr2;
	  struct sockaddr_in server_add_prefetch;
	  bzero(&server_add_prefetch,sizeof(server_add_prefetch));              
	  server_add_prefetch.sin_family = AF_INET;                
	  server_add_prefetch.sin_port = htons(atoi(num));      
	  server_add_prefetch.sin_addr.s_addr = inet_addr(addr_ip_prefetch); 
	  if((fptr = fopen(name_file, "r")) == NULL)
	  {
		printf("file can't be read properly\n");
		return 1;
	  }
	  else
	  {
			while((getline(&extract_line, &length, fptr) != -1))
			{
		        	bzero(file_cache, sizeof(file_cache));
				bzero(req_url, sizeof(req_url));
				bzero(url_fetch, sizeof(url_fetch));
				bzero(server_req, sizeof(server_req));
				bzero(buffer, sizeof(buffer));
				if((ret_st_opt = strstr(extract_line, "href")))
				{  
					if ((pre_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
					{
				  		printf("Error creating socket at proxy \n");
				  		exit(1);
					}
					if((connect(pre_sock, (struct sockaddr *)&server_add_prefetch, sizeof(server_add_prefetch))) < 0)
					{
				  		printf("Error in Connect to the server. \n");
				  		exit(1);
					}
					if((url_full = strstr(extract_line, "http://")))
					{   
		  				sscanf(url_full, "%[^\"]", req_url);
					}
					else
					{
		 			 	sscanf(ret_st_opt, "%*[^=]%*c%*c%[^\"]", url_fetch );
		  			 	if(url_fetch[0] != '/')
					 	{
		    					sprintf(req_url, "http://%s/%s", name, url_fetch);
							
		  			 	}
		  				else
						{
		    					sprintf(req_url, "http://%s%s", name, url_fetch);
		  				}
					}
					sprintf(server_req, "GET %s HTTP/1.0\r\n\r\n", req_url);  
					md5_url = md5_calculate(req_url); 
					sprintf(file_cache, "./cache/%s", md5_url);
					send(pre_sock, server_req, strlen(server_req), 0);
					fdptr = fopen(file_cache, "ab");
					if(fdptr < 0)
					{
		  				printf("Error Creating Cache file in Prefetch\n");
		  				exit(1);
					}
					get_curr2 = time(NULL);
					fprintf(fdptr, "%lu\n", get_curr2);   //updating time to file
					while((bytes_tot = recv(pre_sock, buffer, sizeof(buffer), 0)))
					{ 
		  				printf("Prefetch Link Received Bytes: %d\n", bytes_tot);
		  				fwrite(buffer, 1, bytes_tot, fdptr);
		  				bzero(buffer, sizeof(buffer));
					}
					fclose(fdptr);
	      			}
	    		}
	    		fclose(fptr);
		        return 0;
	  }

}

int main(int argc, char* argv[])
{
	int sock, fork_child = 1, clientlen = 0, proxy_size = 0, proxysock, proxy_sock, bytes_tot, flag = 0, fork_child_2, pre, host_flag , cache_flag, forbidden_flag;
	unsigned long int timeout;
        uint8_t file_name[MAXBUFSIZE], method_req[MAXBUFSIZE], url_req[MAXBUFSIZE], version_req[MAXBUFSIZE], addr_ip[145] = "",addr_ip_forbid[145] = "", num[32] = "80", host_name[MAXBUFSIZE], buffer[MAXBUFSIZE], req_buffer[MAXBUFSIZE];
	FILE *fptr;
	char *md5_url;
	char* extract_line = NULL;
	size_t length;
	time_t get_curr;
	struct hostent *server_hostname;							
	struct sockaddr_in addr_server;
	bzero(buffer, sizeof(buffer));
	bzero(req_buffer, sizeof(req_buffer));
	struct sockaddr_in client_addr, webproxy_addr;
	clientlen = sizeof(client_addr);
        proxy_size = sizeof(webproxy_addr);
	if (argc < 3)
	{
		printf ("\nUsage: <portNo> <timeout>\n"); //to run the file port no and timeout must also be indicated
		exit(1);
	}
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if ((sock) == 0)
	{
		perror("Socket can't be created"); //socket creation error
	}
	puts("Socket created");
	bzero(&webproxy_addr, proxy_size);
	webproxy_addr.sin_family = AF_INET;
	webproxy_addr.sin_port = htons(atoi(argv[1]));
	webproxy_addr.sin_addr.s_addr = INADDR_ANY;
	if(bind(sock, (struct sockaddr *)&webproxy_addr, proxy_size) < 0)
	{
		perror("Socket binding error"); //socket binding error
	}
	puts("Socket bind successfully - Proxy server starts");
	if(listen(sock, 8) < 0)
	{
		perror("Socket listening error");
	   
	}
	puts("socket listening successful");
	while(1)
	{
	   
		if((proxysock = accept(sock, (struct sockaddr *)&client_addr, &clientlen)) < 0)
	    	{
	     		perror("Connection accept failed"); 
	      		return 1;
	    	}
		puts("connection accepted");
	      	fork_child = fork();
		if(fork_child == 0)
		{
			timeout = atoi(argv[2]);
			printf("Port: %d Socket: %d\n\n", atoi(argv[1]), proxysock);
			while((bytes_tot = recv(proxysock, buffer, sizeof(buffer), 0)))
			{
				strncpy(req_buffer, buffer, bytes_tot);
				bzero(method_req, sizeof(method_req));
				bzero(url_req, sizeof(url_req));
				bzero(version_req, sizeof(version_req));
			    	sscanf(buffer, "%s %s %s", method_req, url_req, version_req);
				printf("Method: %s \tURL: %s \tVersion:%s\n", method_req, url_req, version_req);
				md5_url = md5_calculate(url_req); 
			    	if(strncmp(method_req, "GET\0", 4) != 0)
				{
					error_condition(proxysock, error_method); //invalid method error
					continue;
			    	}
			    	else if((strstr(url_req, "https") != NULL) || (strstr(url_req, "http") == NULL))
				{
			      		error_condition(proxysock, error_url); //invalid url error
			      		continue;
			    	}
				else if((strncmp(version_req, "HTTP/1.0", 8) != 0) && (strncmp(version_req, "HTTP/1.1", 8) != 0))
				{
			      		error_condition(proxysock, error_version); //invalid version error
			      		continue;
			    	}
			    	else
				{
			      		sscanf(url_req, "%*[^/]%*c%*c%[^/]", host_name);
			     		printf("Hostname: %s\n", host_name );
					forbidden_flag = forbid_data(host_name, addr_ip_forbid); 
					if(forbidden_flag == 1)
					{
						error_condition(proxysock, error_blocked);
						continue;
			     		}
					cache_flag = cache_file_data(url_req, timeout);
					if(cache_flag == 1)
					{
						printf("\n %d Page is found in cache\n", proxysock); //pgae is found in the cache
						bzero(file_name, sizeof(file_name));
						sprintf(file_name, "./cache/%s", md5_url);
						fptr = fopen(file_name, "r");
						getline(&extract_line, &length, fptr); //get the line where file pointer points to
						bzero(buffer, sizeof(buffer));
						while((bytes_tot = fread(buffer, 1, sizeof(buffer), fptr)))
						{
				  			send(proxysock, buffer, bytes_tot, 0);
				  			bzero(buffer, sizeof(buffer));
						}
						fclose(fptr);
						continue;
			     		 }
					 else
					 {
						printf("\n %d Page not present in cache\n", proxysock);
						if(strchr(host_name, ':'))
						{
				  			bzero(num, sizeof(num));
				  			sscanf(host_name, "%[^:]%*c%[^/]", addr_ip, num);
				  			bzero(&addr_server,sizeof(addr_server));               
				  			addr_server.sin_family = AF_INET;                 
				  			addr_server.sin_port = htons(atoi(num));      
				  			addr_server.sin_addr.s_addr = inet_addr(addr_ip); 
						}
						else
						{
				 
				  			host_flag = cache_data(host_name, addr_ip);
							if(host_flag==1)
							{
				    				printf("\n Host is in cache\n"); //host is found in cache in data_file
								bzero(file_name, sizeof(file_name));
				    				sprintf(file_name, "./cache/data_file");
								bzero(&addr_server,sizeof(addr_server));              
				    				addr_server.sin_family = AF_INET;                
				    				addr_server.sin_port = htons(atoi(num));      
				    				addr_server.sin_addr.s_addr = inet_addr(addr_ip); 
				  			}
				  			else
							{
				    				printf("\n Host not found in Cache");
				    				bzero(&addr_server,sizeof(addr_server));              
				    				addr_server.sin_family = AF_INET;                 
				    				addr_server.sin_port = htons(atoi(num));     
				   				server_hostname = gethostbyname(host_name);					 
				    				bcopy((char*)server_hostname->h_addr, (char*)&addr_server.sin_addr, server_hostname->h_length);
								if(server_hostname < 0)
								{
				     					error_condition(proxysock, error_servernotfound);
				      					continue;
				   				}
				    				else
								{
				      					bzero(file_name, sizeof(file_name));
				     				        sprintf(file_name, "./cache/data_file");
				      					fptr = fopen(file_name, "ab");
									bzero(buffer, sizeof(buffer));
				      					sprintf(buffer, "%s %s\n", host_name, inet_ntoa(addr_server.sin_addr));
				      					fwrite(buffer, 1, strlen(buffer), fptr);
				      					fclose(fptr);
				    				}
				  			}
						  }
						  if ((proxy_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) //socket for data fetch
						  {
				  				perror("Socket creation error");
				  				continue;
						  }
						  if((connect(proxy_sock, (struct sockaddr *)&addr_server, sizeof(addr_server))) < 0)
						  {
				  				perror("Connection failure");
				  				continue;
						  }
						  send(proxy_sock, req_buffer, strlen(req_buffer), 0);
						  bzero(buffer, sizeof(buffer));
						  bzero(file_name, sizeof(file_name));
						  sprintf(file_name, "./cache/%s", md5_url);
						  fptr = fopen(file_name, "ab");
						  if(fptr < 0)
						  {
				  		  	printf("Cache file creation error\n");
				  			exit(1);
						  }
						  get_curr = time(NULL);  //to get the current time
						  fprintf(fptr, "%lu\n", get_curr);  // saving the file with time updates
						  bzero(buffer, sizeof(buffer));
						  while((bytes_tot = recv(proxy_sock, buffer, sizeof(buffer), 0)))
						  {
				 			 if(strstr(buffer, "<html"))
							{
				    				flag = 1;
				  			}
							send(proxysock, buffer, bytes_tot, 0);
				  			fwrite(buffer, 1, bytes_tot, fptr);
				  			bzero(buffer, sizeof(buffer));
						  }
						  fclose(fptr);
						  if(flag==1)
						  {
				  			fork_child_2 = fork(); //fork to create child thread from where the code executes if a prefetch link is being obtained
							if(fork_child_2 == 0)
							{
				    				pre = prefetch_data(inet_ntoa(addr_server.sin_addr), file_name, host_name, num); //function call to prefetch links operation
				   			        exit(0);
				 		         }
						  }
			      		}
			    	}
			  }
			close(proxysock);
			exit(0);
	      	}
	  }
	  return 0;
}
