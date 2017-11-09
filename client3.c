#include<stdio.h>//printf()
#include<sys/socket.h>//socket(),send(),recv()
#include<arpa/inet.h>//sockaddr_in and inet_addr()
#include<stdlib.h>//exit()
#include<unistd.h>//close()
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>  
#include <pthread.h>
#include<string.h>
#include"project.h"

/*
to make main() and thread() work coordinately, we need some global value
*/
struct sockaddr_in DesAddr;//declare the struct of destination
int sock1;
int DesAddrLen;
void* thread(void* threadid);//declare a function

/*
main() is responsible for setting some inital value and create threads and sockets
*/
int main()
{	

	errno = 0; //errno used for print error

	//define the struct
	DesAddr.sin_family = PF_INET;
	DesAddr.sin_addr.s_addr = inet_addr("10.0.2.15");//the ip address is 10.0.2.15
	DesAddr.sin_port = htons(20000);//port number is 20000
	DesAddrLen = sizeof(DesAddr);

	int threadCreate;//thread descriptor
	
	pthread_t threadid;

	srand((unsigned)time(NULL)); 
	/*
	create several sockets
	*/
	for(;;)
	{


	if((sock1=socket(PF_INET,SOCK_STREAM,0))==-1)
		{	
			printf("Sorry, socket establishment failed.\n");
		 	printf("errno = %d\n", errno); 
    		perror("failed");  
      		printf("error: %s\n", strerror(errno));
			exit(0);
		}
	else
		printf("Socket Establishment Succeeded.\n");
		
	threadCreate = pthread_create(&threadid,NULL,(void *) thread,NULL); //create a thread, reaturn 0 if succeeds
	
	if(	threadCreate!=0) 
	{
		printf ("Create pthread error!\n");
		printf("errno = %d\n", errno); 
		perror("failed");  
		printf("error: %s\n", strerror(errno));
		exit (0);
	}
	pthread_join(threadid,NULL);
	}

	return 1;
}


/*
thread() is responsible for communications to servers
*/
void* thread(void* threadid){
	errno = 0; //errno used for print error
	
	sleep(3);//run per sec
	
	int choice1;//choose to generate a ddl or not
	int choice3;//choose to generate a flowsize or not
	int choice4;//choose to generate a flowspeed or not
	int choice5;//...send mode or not
	int choice6;//...send tme or not
	int choice7;//...urgent mode or not
	char filebuffer[1024];
	char *filename = (char*)malloc(0);//assign space for filename
	
	struct extra_information this_flow;//instantiation
	memset(&this_flow,0,sizeof(this_flow));
 	/*
 	generate a large number of strcutures' values in a short time
 	*/    
    
	/*
	generate a rand filesize in the range of 1-1024*1024mb
	0 means NOT SET 1,2,3,4 mean SET
	generate a rand filespeed in the range of 1 - 20*1024 mbps and 0 means NOT SET
	*/
	choice3 = rand()%(4-0+1)+0;
	if(choice3>0&&choice3<=4)	
		this_flow.flowSize= rand()%(1024*1024-1+1)+1/(double)(RAND_MAX);
	
	choice4 = rand()%(4-0+1)+0;
	if(choice4>0&&choice4<=4)	
		this_flow.speed = rand()%(1024*10-1+1)+1/(double)(RAND_MAX);
		
	
	/*
	generate a rand urgentmode and transmission mode
	2 means NOT SET in trans mode
	*/
	choice7 = rand()%(4-0+1)+0;//0-4, 0-2 is generating the value
	if(choice7>0&&choice7<=2)	
		this_flow.UrgentMode=rand()%(9-1+1)+1;

	choice5 = rand()%(4-0+1)+0;
	if(choice5>0&&choice5<=4)
		this_flow.TransmissionMode = rand()%(2-1+1)+1;
	
	//generate a rand send time in YEAR 2017 d = 1483200000;//2017/11/0:00 t = 1514736000;//2018/1/1/0:00
	//no Not SET in this scenario, NOT SET should be 0 in other scenario
	choice6 = rand()%(4-0+1)+0;
	if(choice6>0&&choice6<=4)
		this_flow.sendtime = rand()%(1514736000-1483200000+1)+1483200000;
	
	/*
	generate a random number from 0-3 0,1,2 will generate random ddl, 3 will be NOT SET
	suppose all ddls are the tomorrow of sendtimes
	Not SET is "NULL"
	
	*/
	struct tm *area1;//the structure used for store ddl
	int hours;//the hour of the dll 24-h
	int minutes;//the minute of the ddl
	choice1 = rand()%(3-0+1)+0;
	if(choice1<3&&choice1>0)
	{
		tzset(); /*tzset()*/
		area1 = gmtime(&this_flow.sendtime);//get the sendtime in structure

		hours =  rand()%(23-0+1)+0;
		minutes =  rand()%(59-0+1)+0;
	
		area1->tm_hour = hours;//change the hour
		area1->tm_min = minutes;//change the minute
		area1->tm_mday = area1->tm_mday  +1;
		strcpy(this_flow.deadline,asctime(area1));
	}

	//select the file the user wants to send

	strcpy(filename,"try.txt");
	FILE *fp = fopen(filename, "r"); 
  
    if(fp == NULL) 
    { 
    	printf("File: %s can not open or NO such file\n",filename); 
   		printf("errno = %d\n", errno); 
 		perror("failed");  
 		printf("error: %s\n", strerror(errno));
    	exit(1);
    } 
    

 	//connect to the server
 	connect_init(sock1,DesAddr,DesAddrLen,this_flow);
 	
 	/*
 	read file content into filebuffer
 	send the filebuffer to the server
 	*/
	printf("Reading the file content into the buffer now...\n");
	
	fread(filebuffer,sizeof(filebuffer),1,fp);
	fseek(fp,sizeof(filebuffer),SEEK_CUR);
	if((send(sock1,filebuffer,sizeof(filebuffer),0))<0)
		{
		printf("Sorry. Sending failed.\n");
 		printf("errno = %d\n", errno); 
 		perror("failed");  
 		printf("error: %s\n", strerror(errno));
		}
	else
		{
		fclose(fp);
		printf("%s\n",filebuffer);
		memset(filebuffer,0,sizeof(filebuffer));
		} 
	
	/*
	close the connection between the client and the server
	close the socket
	quit the thread
	*/
	shutdown(sock1,2);
	close(sock1);
	pthread_exit(NULL);		


}


