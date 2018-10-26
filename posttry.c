//POST requested
  if(strncmp(first,"POST\0",5)==0)
  {
    int postdatalength=0;
    bzero(buffer,sizeof(buffer));
    //printf("\nPost wala data :\n%s\n",buffer0);//command recieved from client
    postdata =strtok(buffer0,"\n");//saving protocol in third variable
    //printf("\npost data isisisis :\n%s\n",postdata);//command recieved from client
    postdata1 =strtok(NULL,"\n\n");//saving protocol in third variable
    //printf("\npost data isisisis :\n%s\n",postdata1);//command recieved from client
    postdata2 =strtok(NULL,"\r\n");//saving protocol in third variable
    //printf("\npost data isisisis :\n%s\n",postdata2);//command recieved from client
    postdata3 =strtok(NULL,"\r\n");//saving protocol in third variable
    printf("\nPost Data is:\n%s\n",postdata3);//command recieved from client
    char *content="text/html";
    char header1[MAXBUFSIZE];
    //printf("\n%ld\n",strlen(postdata3));
    sprintf(header1,"HTTP/1.1 200 OK\r\nContent-Type: %s\r\nContent-Length: %ld\r\n\r\n<html><body><pre><h1>%s</h1></pre>\r\n",content,strlen(postdata3),postdata3);
    printf("\nPost Header: \n %s",header1);
    char dir1[MAXBUFSIZE] = "/home/PA2/www/index.html";
    pFile =fopen(dir1,"r");
    if (pFile==NULL)
    {
      printf("\nERROR: Invalid File Request Recieved\n");
      nbytes1 = send(conn,bad_request, strlen(bad_request), 0);
      shutdown(conn, SHUT_RDWR);
      close(conn);
      return;
    }
    else
    {
      printf("Index File Created\n");
    }
    var1 = fread(buffer,1,MAXBUFSIZE,pFile);
    //printf("file read is %ld\n",var1);
    nbytes=write(conn, buffer, var1);
    printf("Index File Sent");//index file requested
    //printf("sent file as %d\n",nbytes);
    bzero(buffer,sizeof(buffer));//buffer zero
    fclose(pFile);
  }
