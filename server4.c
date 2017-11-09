#include<stdio.h>//printf()
#include<sys/socket.h>//socket(),...
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <pthread.h>
#include "project.h"

struct sockaddr_in localAddr;
struct sockaddr_in clientAddr;

int sock1;//used for listening
int sock2;//used for communication
socklen_t localAddrLen;//the length of the local addr
socklen_t clientAddrLen;//the length of the client addr
char buffer[1024*1024];//received buffer
char *clientIP;
int localPort = 20000;
int recvMsgSize;//received message size
FILE *fp;
void thread();

int main()
{
	//declare

	pthread_t threadid;
	int threadCreate;

	//build up the sock1

	if((sock1=socket(PF_INET,SOCK_STREAM,0))<0)//based on TCP
	{	
		printf("socket() Failed.\n");
		exit(0);
	}
	else
		printf("Creating socket 1 succeeded.\n ");

	//give value to the struct
	
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_port = htons(localPort);
	
	localAddrLen = sizeof(localAddr);
	//bind the socket to the port
	if((bind(sock1, (struct sockaddr*) &localAddr, localAddrLen))<0)
	{
		printf("bind() Failed.\n");
		exit(0);
	}
	else
		printf("bind() succeeded.\n");

	//listen new connections
	if((listen(sock1,100))<0)//can listen 100 connections
	{
		printf("listen() failed.\n");
		exit(0);
	}
	else
		printf("Listening...\n");
	for(;;)
	{


	struct extra_information newflow;//instantiation
	memset(&newflow,0,sizeof(newflow));
	
	int choice1 = 0;//flow size
	int choice2 = 0; //flowspeed
	int choice3 =0;//urgen levels
	int choice4 = 0;//trigger 
	trigger = 0;
	choice4 = rand()%(4-0+1)+0;
	if(choice4 ==4 )
	{
		
		trigger = 2;
	choice1 = rand()%(4-0+1)+0;
	
	if(choice1<=3)	
		newflow.flowSize= rand()%(1024*1024-1+1)+1/(double)(RAND_MAX);
		
	printf("size: %lf\n\n\n",newflow.flowSize);
	
	choice2 = rand()%(4-0+1)+0;
	if(choice2<=2)	
		newflow.speed= rand()%(1024*1024-1+1)+1/(double)(RAND_MAX);
		
	printf("speed: %lf\n\n\n",newflow.speed);
	
	choice3 = rand()%(4-0+1)+0;
	if(choice3>2)
		newflow.UrgentMode=10;
	printf("um: %d\n\n\n",newflow.UrgentMode);
}
		
	sock2 = accept_init(sock1,clientAddr,clientAddrLen,newflow);
	threadCreate = pthread_create(&threadid,NULL,(void *) thread,NULL); //create a thread, reaturn 0 if succeeds
	if(	threadCreate!=0) 
	{
		printf ("Create pthread error!\n");
		exit (0);
	}

	clientAddrLen = sizeof(clientAddr);
	pthread_join(threadid,NULL);	
	}//the end of fpr loop


	shutdown(sock1,2);
	close(sock1);
}

void thread(){
	
	//receive messages

		
	memset(&buffer, 0, sizeof(buffer));
	if((recvMsgSize =recv(sock2, buffer, sizeof(buffer),0))<0)
	{
		printf("recv() failed.\n");
		printf("Received length: %d\n",recvMsgSize);
		
	}
	else
	{	
		
		if((fp = fopen("/home/student/Project/ServerReceive.txt","ab+")) == NULL)
			{
				fprintf(stderr,"Writing the buffer to the  failed.\n");
				exit(0);
			}
		printf("Received:\n\n");
		printf("%s\n",buffer);
		fwrite(buffer,sizeof(buffer),1,fp); 
		
		fseek(fp,sizeof(buffer),SEEK_CUR);
		
	}
	
	
	shutdown(sock2,2);
	close(sock2);
	

	
	fclose(fp);
	
} 


