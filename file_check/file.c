#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#define READ_SIZE 32
#define DATA_SIZE 100
#define NEW_LINE 10
#define EOF_new 1


void test_file(uint8_t* fname,uint8_t* data)
{
	int32_t eof_check=0,check=0;
	FILE *fptr1 = fopen(fname,"r");
	FILE *fptr2 = fopen("test_write.txt","a");
	
	while(eof_check != EOF_new)
	{	
		data=fgets(data,READ_SIZE,fptr1);
		if(data!=NULL)
		{	
			printf("%s",data);
			check=fputs(data,fptr2);
		}
		eof_check=feof(fptr1);
	}
	fclose(fptr1);
	fclose(fptr2);
	return;
}

void read_file(uint8_t* fname,uint8_t* data)
{
	int32_t eof_check=0;
	FILE *fptr = fopen(fname,"r");
	while(eof_check != EOF_new)
	{		
		data=fgets(data,READ_SIZE,fptr);
		if(data!=NULL)
		{	
			printf("%s",data);
		}
		eof_check=feof(fptr);
	}
	fclose(fptr);
	return;
}

void write_file(uint8_t* fname,uint8_t* data)
{
	uint32_t error_check=0;
	FILE *fptr = fopen(fname,"w");
	error_check=fputs(data,fptr);
	fclose(fptr);
	return;
}

void append_file(uint8_t* fname,uint8_t* data)
{
	uint32_t error_check=0;
	FILE *fptr = fopen(fname,"a");
	error_check=fputs(data,fptr);
	fclose(fptr);
	return;
}

void main(void)
{
	uint8_t data[DATA_SIZE];
	write_file("test.txt","Hello World!!\n");
	append_file("test.txt","Monish Nene\n");
	read_file("test.txt",data);
	test_file("test.txt",data);
}
