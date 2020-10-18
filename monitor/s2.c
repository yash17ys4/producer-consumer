#include "../utils/unp.h"
#include "../utils/error.c"
#include "../utils/wrapsock.c"
#include<time.h>
#include <fcntl.h>
#include <pthread.h>
#include<semaphore.h>

/* count of clients connected to server*/
int consumerCount = 0;
int producerCount = 0 ;


/* The mutex lock */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_attr_t attr;

/* semaphores */
sem_t full,empty;


#define MAXPRODUCERS 1024
#define MAXCONSUMERS 1024
#define LIMIT 1024
#define BUF_SIZE 10


struct client{

   int index;
   int sockID;
   struct sockaddr_in clientAddr;
   socklen_t len;

};



/* consumer threads */
struct client Consumers[MAXCONSUMERS+1];
pthread_t C_threads[MAXCONSUMERS+1];

/* producer threads */
struct client Producers[MAXPRODUCERS+1];
pthread_t P_threads[MAXPRODUCERS+1];


/* the buffer */
int Buffer[BUF_SIZE];

/* buffer counter */
int counter;

void initializeData() {

   /* Create the mutex lock */
   pthread_mutex_init(&mutex, NULL);

   /* Create the full semaphore and initialize to 0 */
   sem_init(&full, 0, 0);

   /* Create the empty semaphore and initialize to BUFFER_SIZE */
   sem_init(&empty, 0, BUF_SIZE);

   /* Get the default attributes */
   pthread_attr_init(&attr);

   /* init buffer */
   counter = 0;
}

int consume_item(int *item){
   if(counter>0){
      --counter;
      *item=Buffer[counter];
      return 0;
   }return -1;

}
void *handle_cosume(void* clientInfo){

   struct client* ca=(struct client *)clientInfo;
   int fd=ca->sockID;
   int n,data;

   while(1){
      n=recv(fd,&data,sizeof(data),0);
      sem_wait(&full);
      pthread_mutex_lock(&mutex);
      fprintf(stderr, "Got a consume\n");
      if(consume_item(&data)) {
         fprintf(stderr,"Consumer report error condition\n");
      }
      else {
         n=send(fd,&data,sizeof(data),0);
         fprintf(stderr,"Consumer comsumed%d\n", data);
      }
      pthread_mutex_unlock(&mutex);
      sem_post(&empty);
   }







}
int insert_item(int item){
   if(counter<BUF_SIZE){
      Buffer[counter]=item;
      counter++;
      return 0;
   }return -1;
}

void *handle_produce(void* clientInfo){
   struct client* ca=(struct client *)clientInfo;
   int fd=ca->sockID;
   int n,data;

   while(1){

      n=recv(fd,&data,sizeof(data),0);
      sem_wait(&empty);
      pthread_mutex_lock(&mutex);

      if(insert_item(data)) {
         fprintf(stderr," Producer report error condition\n");
      }
      else {
         fprintf(stderr,"Producer produced %d\n", data);
      }
      data=1;
      n=send(fd,&data,sizeof(data),0);
      pthread_mutex_unlock(&mutex);
      sem_post(&full);

   }

}
   char buf[MAXLINE+1];
int main(int argc, char const *argv[])
{
   int listenfd,connfd;

   struct sockaddr_in servaddr,connaddr;
   time_t t;

   if(argc!=2)
	  	err_quit("Usage : ./server <PORT>");

	

   listenfd=Socket(AF_INET,SOCK_STREAM,0);
   
   bzero(&servaddr,sizeof(servaddr));
   servaddr.sin_family=AF_INET;
   servaddr.sin_port=htons(atoi(argv[1]));
   servaddr.sin_addr.s_addr=htonl(INADDR_ANY);

   int opts=1;
   setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&opts,sizeof(opts));

   Bind(listenfd,(struct sockaddr* )&servaddr,sizeof(servaddr));

   Listen(listenfd,5);

   char ip4[INET_ADDRSTRLEN];
   inet_ntop(AF_INET,&servaddr.sin_addr,ip4,INET_ADDRSTRLEN);
   fprintf(stderr,"----Server started at %s:%d----\n",ip4,ntohs(servaddr.sin_port));
   initializeData();
   while(1){

	  struct client ca;
	  ca.sockID=Accept(listenfd,(struct sockaddr*)&ca.clientAddr,&ca.len); 
     fprintf(stderr,"got a connection\n");
     int n=Recv(ca.sockID,buf,MAXLINE,0);
     fprintf(stderr,"%s:%d\n",buf,n);
     if(strcmp(buf,"producer")==0){

         ca.index=producerCount;
         Producers[producerCount]=ca;
         pthread_create(&P_threads[producerCount],&attr,handle_produce,(void*)&ca);
         producerCount++;

     }
     else if(strcmp(buf,"consumer")==0){
         fprintf(stderr, "adwadadw\n");

         ca.index=consumerCount;
         Consumers[consumerCount]=ca;
         pthread_create(&C_threads[consumerCount],&attr,handle_cosume,(void*)&ca);
         consumerCount++;


     }
     else{
      // puts("fwaawf");

     }
   }
	  


   return 0;
}

