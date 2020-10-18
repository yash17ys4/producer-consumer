#include "../utils/unp.h"
#include "../utils/error.c"
#include "../utils/wrapsock.c"
#include<pthread.h>  // mulit-threading
#include<fcntl.h>

#define PATH "data/"

void *doRecv(void *sockfd){
	int fd=*((int *)sockfd);
	while(1){
		char buf[MAXLINE+1];

		int n=Read(fd,buf,MAXLINE);
		buf[n]=0;
		printf("----->\n%s\r\n",buf);

	}

}

int  pollServer(int fd){
  int n;
  int r=1;
  n=send(fd,&r,sizeof(r),0);
  fprintf(stderr, "Request to cosume\n");
  int reply;
  n=recv(fd,&reply,sizeof(reply),0);
  fprintf(stderr,"Successfully Consumed %d\n",reply);
  return reply;

}
int main(int argc, char const *argv[])
{
   int sockfd,n;
   struct sockaddr_in servaddr;

   if(argc!=2){
   	err_quit("Usage ./client <PORT>");
   }

   sockfd=Socket(AF_INET,SOCK_STREAM,0);



   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family=AF_INET;
   servaddr.sin_port=htons(atoi(argv[1]));

   servaddr.sin_addr.s_addr=INADDR_ANY; 
   // connect to any ipaddr

   // if(inet_pton(AF_INET,&argv[1],&servaddr.sin_addr)<=0)
		// err_quit("inet_pton error for %s",argv[1]);

   Connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));

   fprintf(stderr,"---Connection Established---");
   send(sockfd,"consumer",sizeof("cconsumer"),0);
   do{

    pollServer(sockfd);

   }while(1);

	return 0;
}
