#include<stdio.h>//printf()
#include<sys/socket.h>//socket(),...
#include<arpa/inet.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include "jsmn.h"
#include <errno.h>
#include <math.h>
//to make functions work coordinately, there are some global values

/*
group 1: used for communication
*/ 
struct sockaddr_in localAddr;
struct sockaddr_in clientAddr;
socklen_t localAddrLen;//the length of the local addr
socklen_t clientAddrLen;//the length of the client addr
int sock1;//used for listening
int sock2;//used for communication
char buffer[500];//received buffer
char *clientIP;
int localPort = 1501;
int recvMsgSize;//received message size

/*
group 2: used for the manager's functions 
*/
struct recvInformation{
	double flowsize;//the size of the flow
	int urgentmode;//the urgent level of the flow, the same as UrgentMode in extra_information
	double flowspeed;//the speed of the flow
	char ddl[20];//the ddl of the flow
	char transmissiontime[20];//the time of the client sending the flow
	int sendmode;//unidirectional is 1, bidirectional is 0
	char serverip[25];//the ip address of the server
	char clientip[25];//the ip addressof the client
	int role;//from a server or a client
	char others[300];//unexpeceted contents
	}new_flow;// it is a structre array
	
/*
next variables are series of counters or descriptors for recording and storing flow information
*/

/*
variables about flows' total numbers 
*/
unsigned int TotalCounter;//the totoal counter of flows 
unsigned int count1;// how many flows every minute has
unsigned int ServerRole1;//how many client in total
unsigned int ServerRole2;//how many client in the min
unsigned int ClientRole1;//how many server in total
unsigned int ClientRole2;//how many server in the min
/*
variables about speed
*/
double flowspeed1;//the average filespeed of all flows
double flowspeed2;//the average filespeed of flows in the minute
double flowspeed7;//the average filespeed of client flows in the minute
double flowspeed8;//the average filespeed of client flows in total
double flowspeed9;//the average filespeed of server flows in the minute
double flowspeed0;//the average filespeed of server flows in total

unsigned int flowspeed3;//NOT SET in total of client mode
unsigned int flowspeed4;//NOT SET in the minute of cient mode
unsigned int flowspeed5;//the speed is faster than 5*1024 mbps in total
unsigned int flowspeed6;//the speed is faster than 5*1024 mbps in the minute
unsigned int flowspeed11;//not set in total of server mode
unsigned int flowspeed12;//not set in total of server mode in min 

/*
variables about flowsize
*/
double flowsize1;//total size in total
double flowsize2;//total size in per min
unsigned int flowsize3;//counting the number of all files whose sizes are bigger than 5*1024 mb
unsigned int flowsize4;//counting the number of files whose sizes are bigger than 5*1024 mb in the minute
unsigned int flowsize5;//NOT SET in the minute
unsigned int flowsize6;//NOT SET in total
unsigned int flowsize7;//small recv of the server in toal
unsigned int flowsize8;//small recv of the server in this min
/*
variables about urgent levels
*/
unsigned int umcount1;//urgent mode is um in total
unsigned int umcount2;//um in per min
unsigned int umcount3;//flows which didn't set urgent level in this minute
unsigned int umcount4;//flows which didn't set urgent level in total
unsigned int umcount5;//don't send urgent flow to the server in total
unsigned int umcount6;//don't send urgent flow to the server in the min
/*
variables about trans mode
*/
unsigned int trscount1;//NOT SET transmission mode in total
unsigned int unicount;//unidirectional in total
unsigned int bicount;//bidirectonal in total

/*
variables about parsing strings
*/
int parseflag1;//used for failed to parse JSON in total
int parseflag2;//used for failed to parsd JSON in per min

/*
variables about storing
*/
FILE *fp;//the fp of RawMaterials1.txt (extremely urgent 9-8)
FILE *fp4;//the fp of RawMaterials2.txt (very urgent 7-5)
FILE *fp5;//the fp of RawMaterials3.txt (not urgent <=0 <5)
FILE *fp6;//the fp of RawMaterials4.txt	(not set 10)

void thread1();
void thread2();
void login();
int parse_struct();//parse JSON string into, used by the manager
static int jsoneq(const char *json, jsmntok_t *tok, const char *s);
void Flows();
void fspeed();
void fsize();
void furgentmode();
void fsendmode();

int main()
{
	//declare
	errno = 0;
	pthread_t threadid1;
	pthread_t threadid2;
	int thread1Create;
	int thread2Create;
	
	
//	login();
	
 	TotalCounter = 0;
 	ServerRole2 = 0;
 	ServerRole1 = 0;
 	ClientRole1 = 0;
 	ClientRole2 = 0;
 	flowspeed1 = 0.00;
 	flowspeed2 = 0.00;
 	flowspeed3 = 0;
 	flowspeed4 = 0;
 	flowspeed5 = 0;
 	flowspeed6 = 0;
 	flowspeed7 = 0.0;
 	flowspeed8 = 0.0;
 	flowspeed9 = 0.0;
 	flowspeed0= 0.0;
 	flowsize1 = 0.00;
 	flowsize2 = 0.00;
 	flowsize3 = 0;
 	flowsize4 = 0;
 	flowsize5 = 0;
 	flowsize6 = 0;
 	flowsize7 = 0;
 	flowsize8 = 0;
 	umcount1 = 0;
 	umcount2 = 0;
 	umcount3 = 0;
 	umcount4 = 0;
 	umcount5 = 0;
 	umcount6 = 0;
 	trscount1 = 0;
 	bicount = 0;
 	unicount =0;
 	trscount1 = 0;
 	parseflag1 = 0;
 	parseflag2 = 0;
	//build up the sock1
	if((sock1=socket(PF_INET,SOCK_STREAM,0))<0)//based on TCP
	{	
		printf("socket() Failed.\n");
		printf("errno = %d\n", errno); 
 		perror("failed");  
 		printf("error: %s\n", strerror(errno));
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
		printf("errno = %d\n", errno); 
 		perror("failed");  
 		printf("error: %s\n", strerror(errno));
		exit(0);
	}
	else
		printf("bind() succeeded.\n");

	//listen new connections
	if((listen(sock1,1000))<0)//can listen 100 connections
	{
		printf("listen() failed.\n");
		printf("errno = %d\n", errno); 
 		perror("failed");  
 		printf("error: %s\n", strerror(errno));
		exit(0);
	}
	else
		printf("Listening...\n");

	thread2Create = pthread_create(&threadid2,NULL,(void *) thread2,NULL);
	if(	thread2Create!=0) 
		{
			printf ("Create pthread2 error!\n");
			printf("errno = %d\n", errno); 
 			perror("failed");  
 			printf("error: %s\n", strerror(errno));
			exit (0);
		}
	for(;;)
	{	
			//accept a new connection
		clientAddrLen = sizeof(clientAddr);

				
		if((sock2=accept(sock1, (struct sockaddr*)&clientAddr, &clientAddrLen))<0)
		{
			printf("accept() Failed.\n");
			printf("%d\n",sock2);
			printf("errno = %d\n", errno); 
 			perror("failed");  
 			printf("error: %s\n", strerror(errno));
			exit(0);
		}
		else
		{	
			clientIP = inet_ntoa(clientAddr.sin_addr);
		}
		
		thread1Create = pthread_create(&threadid1,NULL,(void *) thread1,NULL); //create a thread, reaturn 0 if succeeds
	
		if(	thread1Create!=0) 
		{
			printf ("Create pthread1 error!\n");
			printf("errno = %d\n", errno); 
 			perror("failed");  
 			printf("error: %s\n", strerror(errno));
			exit (0);
		}
		
	pthread_join(threadid1,NULL);	
	
	}//the end of for loop


	shutdown(sock1,2);
	close(sock1);
	return 1;
	
}

void thread1(){
	
	errno = 0;
	//receive messages

		
	memset(&buffer, 0, strlen(buffer));
	//receive the JSON string, store it into buffer
	if((recvMsgSize =recv(sock2, buffer, sizeof(buffer),0))<0)
	{
		printf("recv() failed.\n");
		printf("Received length: %d\n",recvMsgSize);
		printf("errno = %d\n", errno); 
 		perror("failed");  
 		printf("error: %s\n", strerror(errno));
		
	}
	else
	{	
		
		
		parse_struct();
		Flows();
		fspeed();
		fsize();
		furgentmode();
		fsendmode();
		shutdown(sock2,2);
		close(sock2);
		
		
	}//the end of else

	

	
} 
void thread2()
{
	int sp;
	double OneGB = 1024.0;
	double OneTB = 1024.0*1024.0;
	time_t t;
	char Recvbuffer[2048];
	FILE *fp3;//the fp of RecvInfo.txt
	errno = 0;
	tzset();
	for(;;)
	{
	sleep(60);
	double totalcount = (double)TotalCounter;
	double counter1 = (double)count1;
	double flowsp3 = (double)flowspeed3;
	double flowsp4 = (double)flowspeed4;
	double flowsp11 = (double)flowspeed11;
	double flowsp12 = (double)flowspeed12;
	double pflag1 = (double)parseflag1;
	double pflag2 = (double)parseflag2;
	double clientsp1 = (double)ClientRole1;
	double clientsp2 = (double)ClientRole2;
	double serversp1 = (double)ServerRole1;
	double serversp2 = (double)ServerRole2;
	time(&t);
	/*
	here we keep the time by local time because it is more acceptable for the user
	*/
	sp = sprintf(Recvbuffer,"\n------------%s\n",asctime(localtime(&t)));
	printf("\n\n"); 
	printf("\n----------%s\n",asctime(localtime(&t)));	
	
	printf("-----------------Total---------------\n");
	printf("%d Flows in Total\n",TotalCounter);
	sp += sprintf(Recvbuffer+sp,"%d Flows in Total\n",TotalCounter);
	printf("%d Pieces of Flow Information\n",ClientRole1);
	sp += sprintf(Recvbuffer+sp,"%d Flows are Communication Flows in Total\n",ClientRole1);
	printf("%d Flow Requirements\n",ServerRole1);
	sp += sprintf(Recvbuffer+sp,"%d Flows are Server Requirements in Total\n",ServerRole1);
	printf("%d Flows cannot be analyzed in Total\n",parseflag1);
	sp += sprintf(Recvbuffer+sp,"%d Flows cannot be analyzed in Total\n",parseflag1);
	
	
	printf("--Speed:\n\tAverage Speed: %lf Mbps\n",fabs(flowspeed1/(totalcount-flowsp3-pflag1-flowsp11)));
	sp += sprintf(Recvbuffer+sp,"The Average Speed of All Flows: %lf Mbps\n",fabs(flowspeed1/(totalcount-flowsp3-pflag1-flowsp11)));
	printf("\tAverage Speed of Communication Flows: %lf Mbps\n",fabs(flowspeed8/(clientsp1-flowsp3)));
	sp += sprintf(Recvbuffer+sp,"The Average Speed of Communication Flows: %lf Mbps\n",fabs(flowspeed8/(clientsp1-flowsp3)));
	printf("\tAverage Speed of Server Requirements: %lf Mbps\n",fabs(flowspeed0/(serversp1-flowsp11)));
	sp += sprintf(Recvbuffer+sp,"The Average Speed of Server Rquirements: %lf Mbps\n",fabs(flowspeed0/(serversp1-flowsp11)));
	printf("\tSpeed Higher than 5 Gbps (Communication Flow): %d\n",flowspeed5);
	sp += sprintf(Recvbuffer+sp,"The Number of Flows whose speeds are higher than 5Gps: %d\n",flowspeed5);
	
	if(flowsize1>1024.0&&flowsize1<OneTB)
	{
		printf("--Size:\n\tTotal Size: %lf Gb\n",flowsize1/OneGB);
		sp += sprintf(Recvbuffer+sp,"The Total Size of All Flows: %lf Gb\n",flowsize1/OneGB);
	}
	else if(flowsize1>=1024.0*1024)
	{
		printf("--Size:\n\tTotal Size: %lf Tb\n",flowsize1);
		sp += sprintf(Recvbuffer+sp,"The Total Size of All Flows: %lf Tb\n",flowsize1/OneTB);
	}
	else if(flowsize1<=1024.0&&flowsize1>=0)
	{
		printf("--Size:\n\tTotal Size: %lf Mb\n",flowsize1);
		sp += sprintf(Recvbuffer+sp,"The Total Size of All Flows: %lf Mb\n",flowsize1);
	}

	printf("\tFlows' Data Bigger than 5 GB: %d\n",flowsize3);
	sp += sprintf(Recvbuffer+sp,"The Number of Flow Data which are Bigger than 5 GB: %d\n",flowsize3);
	printf("\tSize-Unknown Flows: %d\n",flowsize6);
	sp += sprintf(Recvbuffer+sp,"The Number of Flow which not marked their sizes: %d\n",flowsize6);
	printf("\tServers' Loads Smaller than 100 Mb: %d\n",flowsize7);
	sp += sprintf(Recvbuffer+sp,"The Number of servers which can not receive flows smaller than 100 MB %d\n",flowsize7);
	
	
	printf("--Urgent Level:\n\tUrgent Flows:%d\n",umcount1);
	sp += sprintf(Recvbuffer+sp,"The Number of Flows which are Urgent:%d\n",umcount1);
	printf("\tUnmarked Flows:%d\n",umcount4);
	sp += sprintf(Recvbuffer+sp,"The Number of Flows which are not Marked Urgent Levels:%d\n",umcount4);
	printf("\tServers Cannot Deal with Urgent Flows:%d\n",umcount5);
	sp += sprintf(Recvbuffer+sp,"The Number of Servers which cannot receive urgent flows:%d\n",umcount5);
	
	printf("--Transmission:\n\t%d Flows will be Bidirectional\n",bicount);
	sp += sprintf(Recvbuffer+sp,"%d Flows will be Bidirectional\n",bicount);
	printf("\t%d Flows will be Unidirectional\n",unicount);
	sp += sprintf(Recvbuffer+sp,"%d Flows will be Unidirectional\n",unicount);
	printf("\t%d Flows are not Marked\n",trscount1);
	sp += sprintf(Recvbuffer+sp,"%d Files are not Marked Transmission Mode\n",trscount1);
	
	printf("\n");
	
	printf("---------IN THIS MINUTE--------------\n");
	printf("%d Flows in the minute\n",count1);
	sp += sprintf(Recvbuffer+sp,"\n%d Flows in the minute\n",count1);
	printf("%d Pieces of Flow Information\n",ClientRole2);
	sp += sprintf(Recvbuffer+sp,"%d Flows are Communication Flows in the minute\n",ClientRole2);
	printf("%d Flow Requirements\n",ServerRole2);
	sp += sprintf(Recvbuffer+sp,"%d Flows are Server Requirements in Total\n",ServerRole2);
	printf("%d Flows cannot be analyzed in the minute\n",parseflag2);
	sp += sprintf(Recvbuffer+sp,"%d Flows cannot be analyzed in this minute\n",parseflag2);
	

	printf("--Speed:\n\tAverage Speed: %lf Mbps\n",fabs(flowspeed2/(counter1-flowsp4-pflag2-flowsp12)));
	sp += sprintf(Recvbuffer+sp,"The Average Speed of Flows in the minute: %lf Mbps\n",fabs(flowspeed2/(counter1-flowsp4-pflag2-flowsp12)));
	printf("\tAverage Speed of Communication Flows: %lf Mbps\n",fabs(flowspeed7/(clientsp2-flowsp4)));
	sp += sprintf(Recvbuffer+sp,"The Average Speed of Communication Flows: %lf Mbps\n",fabs(flowspeed7/(clientsp2-flowsp4)));
	printf("\tAverage Speed of Server Requirements: %lf Mbps\n",fabs(flowspeed9/(serversp2-flowsp12)));
	sp += sprintf(Recvbuffer+sp,"The Average Speed of Server Rquirements: %lf Mbps\n",fabs(flowspeed9/(serversp2-flowsp12)));
	printf("\tSpeed Higher than 5 Gbps (Communication Flows): %d\n",flowspeed6);
	sp += sprintf(Recvbuffer+sp,"The Number of Flows whose speeds are higher than 5Gps: %d\n",flowspeed6);
	
	if(flowsize2>1024.0&&flowsize2<OneTB)
	{
		printf("--Size:\n\tTotal Size: %lf Gb\n",flowsize2/OneGB);
		sp += sprintf(Recvbuffer+sp,"The Total Size of Flows in the minute: %lf Gb\n",flowsize2/OneGB);
	}
	if(flowsize2>1024*1024.0)
	{
		printf("--Size:\n\tTotal Size: %lf Tb\n",flowsize2);
		sp += sprintf(Recvbuffer+sp,"The Total Size of Flows in the minute: %lf Tb\n",flowsize2/OneTB);
	}
	if(flowsize2>=0&&flowsize2<=1024)
	{
		printf("--Size:\n\tTotal Size: %lf Mb\n",flowsize2);
		sp += sprintf(Recvbuffer+sp,"The Total Size of Flows in the minute: %lf Mb\n",flowsize2);
	}

	printf("\tFlow Data Bigger than 5 GB: %d\n",flowsize4);
	sp += sprintf(Recvbuffer+sp,"The Number of Flow Data which are Bigger than 5 GB: %d Mb\n",flowsize4);
	printf("\tSize-Unknown Flows: %d\n",flowsize5);
	sp += sprintf(Recvbuffer+sp,"The Number of Flows whose Sizes are Unknown: %d Mb\n",flowsize4);
	printf("\tServers' Loads Smaller than 100 Mb: %d\n",flowsize8);
	sp += sprintf(Recvbuffer+sp,"The Number of servers which can not receive flows smaller than 100 MB %d\n",flowsize8);
	
	
	printf("--Urgent Level:\n\tUrgent Flows:%d\n",umcount2);
	printf("\tUnmarked: %d\n",umcount3);
	printf("\tServers Cannot Deal with Urgent Flows:%d\n",umcount6);
	sp += sprintf(Recvbuffer+sp,"The Number of Servers which cannot receive urgent flows:%d\n",umcount6);
	sp += sprintf(Recvbuffer+sp,"The Number of Flows which are Urgent in the minute:%d\n",umcount2);
	sp += sprintf(Recvbuffer+sp,"The Number of Flows which are Unmarked Urgent levels in the minute:%d\n",umcount3);
	
	if((fp3 = fopen("/home/student/Project/RecvInfo.txt","ab+")) == NULL)
			{
				fprintf(stderr,"Writing the buffer to RecvInfo.txt failed.\n");
				exit(0);
			
			 	printf("errno = %d\n", errno); 
        		perror("failed");  
        		printf("error: %s\n", strerror(errno)); 
 		        	
		        
			}
	
	fwrite(Recvbuffer,sizeof(Recvbuffer),1,fp3); 
	fseek(fp3,sizeof(Recvbuffer),SEEK_CUR);
	fclose(fp3);
	memset(&Recvbuffer, 0, sizeof(Recvbuffer));
	count1 = 0;
	flowspeed2 = 0.00;
	flowspeed6 = 0;
	flowspeed7 =0.00;
	flowspeed9 =0.00;
	flowsize2 = 0.00;
	flowsize4 = 0;
	flowsize5 = 0;
	flowsize8 = 0;
	umcount2 = 0;
	umcount3 = 0;
	parseflag2 = 0;
	ClientRole2 = 0;
	ServerRole2 = 0;
	}
}





void login()
{	
	char* user = (char*)malloc(0);
	char* word = (char*)malloc(0);
	char* username = "project";
	char* password = "123456";
	printf("Hello, please log in:\n");
	printf("Username:\n");
	scanf("%s",user);
	printf("Password:\n");
	scanf("%s",word);
	
	for(;;)
	{
		if (strcmp(user,username)!= 0||strcmp(word,password)!= 0)
		{
				printf("Sorry, log in error.\n\n");
				printf("please try again.\n"); 
				printf("Username:\n");
				scanf("%s",user);
				printf("Password:\n");
				scanf("%s",word);
				
		}
		else
			break;
	}
}


/*
the next function was copied from simple.c on https://github.com/zserge/jsmn/tree/master/example
it is opensouce
*/
static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

/*
the next function was modified based on simple.c on https://github.com/zserge/jsmn/tree/master/example
the main method, usage and important annotations were from simple.c
it is opensouce
the structure array is created by the author
*/
int parse_struct()
{	

	//declaration
	errno = 0;
	int i;
	int r;
	jsmn_parser p;
	jsmntok_t t[128]; /* We expect no more than 128 tokens */
	int sp;//StructureInfo.txt
	int sp1;//used for ParseFailed.txt
	int sp2;//StrucutreInfo.xls
	time_t d;
	char filebuffer[1024];//the buffer written into StructureInfo.txt
	char filebuffer2[1024];//ParseFailed.txt
	char filebuffer3[100];//StrcutureInfo.xls
	char *newbuffer = (char*)malloc(0);
	FILE *fp2;//the fp of StructureInfo.txt
	FILE *fp8;//the fp of ParseFailed.txt
	FILE *fp9;// StructureInfo.xls
	newbuffer = (char*)buffer;
	memset(&new_flow,0,sizeof(new_flow));
	

	/*
	to parse char into int or float, intermediary variables will be needed.
	those strings will first be copied into the next pointers 
	then those will be parsed into int or double
	*/
	char *fsize = (char*)malloc(0);//the size of the file
	char *urmode = (char*)malloc(0);//the urgent mode
	char *fspeed = (char*)malloc(0);//the speed of the file
	char *Transmode = (char*)malloc(0);//the transmission mode
	char *rolemode  = (char*)malloc(0); 

	sp = sprintf(filebuffer,"Flow No.\t%d\t",TotalCounter+1);
/**
 * Creates a new parser based over a given buffer with an array of tokens
 * available.
 */
	jsmn_init(&p);
/**
 * Parse JSON string and  tokens.
 */
 	
	r = jsmn_parse(&p, newbuffer, strlen(newbuffer), t, sizeof(t)/sizeof(t[0]));
	if (r < 0) {
		
		parseflag1 = parseflag1 +1;
		parseflag2 = parseflag2 +1;
		if((fp8 = fopen("/home/student/Project/ParseFailed.txt","ab+")) == NULL)
			{	
			
				fprintf(stderr,"Writing the buffer to ParseFailed.txt failed.\n");
				printf("errno = %d\n", errno); 
        		perror("failed");  
        		printf("error: %s\n", strerror(errno)); 
				exit(0);
	
		        
			}
	tzset();
	time(&d);
	sp1 = sprintf(filebuffer2,"\n------------%s\n",asctime(localtime(&d)));
	sp1 += sprintf(filebuffer2+sp1,"Failed to parse JSON :%d\n",r);
	fwrite(filebuffer2,sizeof(filebuffer2),1,fp8); 
	fwrite(newbuffer,sizeof(newbuffer),1,fp8); 
	fseek(fp8,sizeof(newbuffer),SEEK_CUR);
	fclose(fp8);
	parseflag2 = 0;
	return 1;
	}

	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT) {
		printf("Object expected\n");
		return 1;
	}

	/* Loop over all keys of the root object */
for (i = 1; i < r; i++) {
		
		//get flowsize
		if (jsoneq(newbuffer, &t[i], "flowSize") == 0) {
		
			new_flow.flowsize = atof(strncpy(fsize,newbuffer + t[i+1].start,t[i+1].end-t[i+1].start));
			sp += sprintf(filebuffer+sp,"-------------\n- FlowSize:\t%lf\n",new_flow.flowsize);
			sp2 = sprintf(filebuffer3,"FlowSize:\t%lf\t",new_flow.flowsize);
	
			i++;
		} 
		//get urgent mode
			else if (jsoneq(newbuffer, &t[i], "UrgentMode") == 0) {
		
			new_flow.urgentmode = atof(strncpy(urmode,newbuffer + t[i+1].start,t[i+1].end-t[i+1].start));
			sp += sprintf(filebuffer+sp,"- UrgentMode:\t%d\n",new_flow.urgentmode);
			sp2 += sprintf(filebuffer3+sp2,"UrgentMode:\t%d\t",new_flow.urgentmode);
		
			i++;
		}
		//get speed
		else if (jsoneq(newbuffer, &t[i], "flowSpeed") == 0) {
			
			new_flow.flowspeed = atof(strncpy(fspeed,newbuffer + t[i+1].start,t[i+1].end-t[i+1].start));
			sp += sprintf(filebuffer+sp,"- FlowSpeed:\t%lf\n",new_flow.flowspeed);
			sp2 += sprintf(filebuffer3+sp2,"FlowSpeed:\t%lf\t",new_flow.flowspeed);
			i++;
		}
		//get ddl directly
		else if (jsoneq(newbuffer, &t[i], "Deadline") == 0) {
			int j;
		
			if (t[i+1].type != JSMN_ARRAY) {
				continue; /* We expect groups to be an array of strings */
			}
			for (j = 0; j < t[i+1].size; j++) {
		//		jsmntok_t *g = &t[i+j+2];
			
			}
			strncpy(new_flow.ddl,newbuffer + t[i+1].start,t[i+1].end-t[i+1].start);
			sp += sprintf(filebuffer+sp,"- Deadline:\t%s\n",new_flow.ddl);
			i += t[i+1].size + 1;
			
		} 
	
		//get transmission time directly
		else if (jsoneq(buffer, &t[i], "TransmissionTime") == 0) {
			int j;
		
			if (t[i+1].type != JSMN_ARRAY) {
				continue; /* We expect groups to be an array of strings */
		}
			for (j = 0; j < t[i+1].size; j++) {
			//	jsmntok_t *g = &t[i+j+2];
				
			}
				strncpy(new_flow.transmissiontime,newbuffer + t[i+1].start,t[i+1].end-t[i+1].start);
				sp += sprintf(filebuffer+sp,"- Transmission Time:\t%s\n",new_flow.transmissiontime);
				
				
			i += t[i+1].size + 1;
		} else if
		
	
		//get transmisson mode
		(jsoneq(newbuffer, &t[i], "TransmissionMode") == 0) {
		
			new_flow.sendmode = atof(strncpy(Transmode,newbuffer + t[i+1].start,t[i+1].end-t[i+1].start));
			sp += sprintf(filebuffer+sp,"- Transmission Mode:\t%d\n",new_flow.sendmode);
			sp2 += sprintf(filebuffer3+sp2,"Transmission Mode:\t%d\t",new_flow.sendmode);
			i++;
		}
		//get the ip address of the server 
		else if (jsoneq(newbuffer, &t[i], "DestinationAddress") == 0) {
		
			strncpy(new_flow.serverip,newbuffer + t[i+1].start,t[i+1].end-t[i+1].start);
			sp += sprintf(filebuffer+sp,"- Destination IP:\t%s\n",new_flow.serverip);

			i++;
		} 
		else if (jsoneq(newbuffer, &t[i], "Role") == 0) {
		
			new_flow.role = atof(strncpy(rolemode,newbuffer + t[i+1].start,t[i+1].end-t[i+1].start));
			sp += sprintf(filebuffer+sp,"- Role:\t%d\n",new_flow.role);
			sp2 += sprintf(filebuffer3+sp2,"Role:\t%d\n",new_flow.role);

			i++;
		} 
		
		 else {
			printf("Unexpected key: %.*s\n", t[i].end-t[i].start,
					newbuffer + t[i].start);
			strncpy(new_flow.others,newbuffer + t[i].start,t[i].end-t[i].start);
			sp += sprintf(filebuffer+sp,"- Unexpected key:\t%s\n",new_flow.others);
		}
		//get the ip address of the client 
		
	}
	strcpy(new_flow.clientip,inet_ntoa(clientAddr.sin_addr));
	sp += sprintf(filebuffer+sp,"- Source IP:%s\n",new_flow.clientip);
	//write the parsed structure into the txt file
	if((fp2 = fopen("/home/student/Project/StructureInfo.txt","ab+")) == NULL)
			{
				fprintf(stderr,"Writing the buffer to StructureInfo.txt failed.\n");
				printf("errno = %d\n", errno); 
        		perror("failed");  
        		printf("error: %s\n", strerror(errno)); 
				exit(0);
		        
			}
	
	if((fp9 = fopen("/home/student/Project/StructureInfo.xls","ab+")) == NULL)
			{
				fprintf(stderr,"Writing the buffer to StructureInfo.xls failed.\n");
				printf("errno = %d\n", errno); 
        		perror("failed");  
        		printf("error: %s\n", strerror(errno)); 
				exit(0);
		        
			}
	
	fwrite(filebuffer,sizeof(filebuffer),1,fp2); 
	fwrite(filebuffer3,sizeof(filebuffer3),1,fp9); 
	fseek(fp2,sizeof(filebuffer),SEEK_CUR);
	fseek(fp9,sizeof(filebuffer3),SEEK_CUR);
	fclose(fp2);
	fclose(fp9);
	return EXIT_SUCCESS;
}

void Flows()
{
	
	TotalCounter = TotalCounter +1;
	count1 = count1 +1;
			
		if(new_flow.role == 1)
		{
			ClientRole1 = ClientRole1 +1;
			ClientRole2 = ClientRole2 +1;
		}
		if(new_flow.role ==2)
		{
			ServerRole1 = ServerRole1+1;
			ServerRole2 = ServerRole2+1;
		}
}

void fspeed()
{
	
		
		
		
		flowspeed1 = flowspeed1+new_flow.flowspeed;
		flowspeed2 = flowspeed2+new_flow.flowspeed;
		
		if(new_flow.role ==1)
		{
			flowspeed7 = flowspeed7 + new_flow.flowspeed;
			flowspeed8 = flowspeed8 + new_flow.flowspeed;
			if(new_flow.flowspeed-0.0<0.000001)
			{
			flowspeed3 = flowspeed3 +1;
			flowspeed4 = flowspeed4 +1;
			}
			if(new_flow.flowspeed > 5*1024.0)
		{
			flowspeed5 = flowspeed5 +1;
			flowspeed6 = flowspeed6 +1;
		}
			
		}
		if(new_flow.role ==2)
		{
			flowspeed9 = flowspeed9 + new_flow.flowspeed;
			flowspeed0 = flowspeed0 + new_flow.flowspeed;
			if(new_flow.flowspeed-0.0<0.000001)
			{
			flowspeed11 = flowspeed11 +1;
			flowspeed12 = flowspeed12 +1;
			}
		
		}
	
}

void fsize()
{
		flowsize1 = flowsize1+new_flow.flowsize;
		flowsize2 = flowsize2+new_flow.flowsize;
		
		if(new_flow.role ==1)
		{
			if(new_flow.flowsize>5*1024.0)
		{
			flowsize3 = flowsize3 +1; 
			flowsize4 = flowsize4 +1;
		}
		if(new_flow.flowsize-0.00<0.000001)
			flowsize5 = flowsize5 +1;
			
		}
		
		if((new_flow.flowsize <100.0)&&(new_flow.role ==2))
		{
			flowsize7 = flowsize7 + 1;
			flowsize8 = flowsize8 + 1;
		}
	
			
}

void furgentmode()
{
		if(new_flow.urgentmode>7&&new_flow.urgentmode<10)
		{
			umcount1 = umcount1 +1;
			umcount2 = umcount2 +1;
		}
		if(new_flow.urgentmode == 0)
			umcount3 = umcount3+1; 
			
		if(new_flow.urgentmode == 10 &&new_flow.role ==2)
		{
			umcount5 = umcount5+1;
			umcount6 = umcount6+1;
		}
		
			//the first quarter
		if(new_flow.urgentmode>=8&&new_flow.urgentmode<=9)
		{
			//write the received JSON strings into the txt file as raw materials
			if((fp = fopen("/home/student/Project/RawMaterials1.txt","ab+")) == NULL)
			{
				fprintf(stderr,"Writing the buffer to RawMaterials.txt failed.\n");  
			 	printf("errno = %d\n", errno); 
        		perror("failed");  
        		printf("error: %s\n", strerror(errno)); 
			 
				exit(0);
			}
	
			fwrite(buffer,sizeof(buffer),1,fp); 
			fseek(fp,sizeof(buffer),SEEK_CUR);
	
	
			shutdown(sock2,2);
			close(sock2);
			fclose(fp);
		}
		
	 if(new_flow.urgentmode>=5&&new_flow.urgentmode<=7)
		{
			//write the received JSON strings into the txt file as raw materials
			if((fp4 = fopen("/home/student/Project/RawMaterials2.txt","ab+")) == NULL)
			{
				fprintf(stderr,"Writing the buffer to RawMaterials.txt failed.\n");  
			 	printf("errno = %d\n", errno); 
        		perror("failed");  
        		printf("error: %s\n", strerror(errno)); 
			 
				exit(0);
			}
	
			fwrite(buffer,sizeof(buffer),1,fp4); 
			fseek(fp4,sizeof(buffer),SEEK_CUR);
	
	
			shutdown(sock2,2);
			close(sock2);
			fclose(fp4);
		}
		
	 if(new_flow.urgentmode<5&&new_flow.urgentmode>0)
		{
			//write the received JSON strings into the txt file as raw materials
			if((fp5 = fopen("/home/student/Project/RawMaterials3.txt","ab+")) == NULL)
			{
				fprintf(stderr,"Writing the buffer to RawMaterials.txt failed.\n");  
			 	printf("errno = %d\n", errno); 
        		perror("failed");  
        		printf("error: %s\n", strerror(errno)); 
			 
				exit(0);
			}
	
			fwrite(buffer,sizeof(buffer),1,fp5); 
			fseek(fp5,sizeof(buffer),SEEK_CUR);
	
	
			shutdown(sock2,2);
			close(sock2);
			fclose(fp5);
		}
	 if(new_flow.urgentmode<=0||new_flow.urgentmode>9)
		{
			//write the received JSON strings into the txt file as raw materials
			if((fp6 = fopen("/home/student/Project/RawMaterials4.txt","ab+")) == NULL)
			{
				fprintf(stderr,"Writing the buffer to RawMaterials.txt failed.\n");  
			 	printf("errno = %d\n", errno); 
        		perror("failed");  
        		printf("error: %s\n", strerror(errno)); 
			 
				exit(0);
			}
	
			fwrite(buffer,sizeof(buffer),1,fp6); 
			fseek(fp6,sizeof(buffer),SEEK_CUR);
			fclose(fp6);
		}
}

void fsendmode()
{
	
		if(new_flow.sendmode==1)
			unicount = unicount +1;
		if(new_flow.sendmode==2)
			bicount = bicount +1;
		if(new_flow.sendmode==0)
			trscount1 = trscount1+1;
}