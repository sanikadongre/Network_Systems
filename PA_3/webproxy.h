#ifndef _WEBPROXY_H
#define _WEBPROXY_H

#include <stdint.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>
#include <memory.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <time.h>
#include <openssl/md5.h>


#define MAXBUFSIZE (1024)

uint8_t error_method[] = "HTTP/1.1 400 Bad Request\n\rContent-Type: text/html\nContent-Length: %d\n\r\n<html><body><H1>Error 400 Bad Request: Method Not Supported </H1></body></html>",
error_url[] = "HTTP/1.1 400 Bad Request\n\rContent-Type: text/html\nContent-Length: %d\n\r\n<html><body><H1>Error 400 Bad Request: Invalid URL </H1></body></html>",
error_version[] = "HTTP/1.1 400 Bad Request\n\rContent-Type: text/html\nContent-Length: %d\n\r\n<html><body><H1>Error 400 Bad Request: Invalid HTTP Version </H1></body></html>",
error_servernotfound[] = "HTTP/1.1 404 Bad Request\n\rContent-Type: text/html\nContent-Length: %d\n\r\n<html><body><H1>Error 404 Bad Request: Server Not Found </H1></body></html>",
error_blocked[] = "HTTP/1.1 403 Forbidden\n\rContent-Type: text/html\nContent-Length: %d\n\r\n<html><body>ERROR 403 Forbidden</body></html>";

void shutdown_condition(int proxysock);
void error_condition(int proxysock, uint8_t err_buff[]);
uint8_t* md5_calculate(uint8_t *req_url);
int flag_condition(int flag, FILE *fptr);
int fetch(char* extract_line, size_t length, FILE *fptr, uint8_t* ip);
int forbid_data(uint8_t *name, uint8_t *addr_ip_forbid);
int cache_file_data(uint8_t *req_url, unsigned long int cache_timeout);
int cache_data(uint8_t *name, uint8_t *addr_ip);
int prefetch_data(uint8_t* addr_ip_prefetch, uint8_t* name_file, uint8_t* name, uint8_t* num);






#endif
