/*
Author: LI YUqi
Date: 9 May 2017
Name: project.c
All rights reserved 
LiYuqi@copyright
*/

/*This is the API code for clients in header <project.h>
It includes functions:
1. connect_init(), used for the communication with the manager, connect to the server
2. calculate_size(), used for calculating the size of the delivered file
3. check_urgent(), used for checking whether this file is urgent or not
*/
#include<stdio.h>//printf(), I/O
#include<sys/socket.h>//socket(),send() and recv()
#include<arpa/inet.h>//sock_in and inet_addr
#include<stdlib.h>//atoi() and exit()
#include<unistd.h>//close()
#include<netinet/in.h>//internet address family
#include<netdb.h>//internet structures
#include<ctype.h>
#include<fcntl.h>
#include<string.h>//strlen() and memset()
#include "project.h" 
#include<sys/stat.h>//calculate_fileSize() 
#include <pthread.h>
#include<errno.h>
struct sockaddr_in ManagerAddr;//a structure for the manager address
int ManagerAddrlen;//the length of the manager IP address

int connect_init(int sock,struct sockaddr_in servaddr, int addrlen, struct extra_information this_flow)
{
	printf("Are you here?\n");
	//decalaration
	int ConnectReturn;//the return value of connect() to the server

	int sendReturn;//the return value of send()
	int Newsock = -2;//the sock id of the connection to the manager
	ManagerAddr.sin_family= AF_INET;//TCP/IP suites
	ManagerAddr.sin_addr.s_addr=inet_addr("10.0.2.15");//the ip address of the manager
	ManagerAddr.sin_port = htons(1501);//the port number used for connection of the manager
	ManagerAddrlen = sizeof(ManagerAddr);//calculates the length of the manager address
	

	this_flow.DesIP = inet_ntoa(servaddr.sin_addr);
	this_flow.role = 1;
	printf("coming\n"); 
	
	//first, connects to the manager
	//build up a new socket used for the connection to the manager
	//because the user does not need to know the existence of the manager, there is no notices
	for(;;)
	{
		Newsock = socket(PF_INET,SOCK_STREAM,0);
		if (Newsock>0)
		break;
	}
	for(;;)
	{
		//connect to the manager
		ConnectReturn = connect(Newsock,(struct sockaddr *)&ManagerAddr,ManagerAddrlen);
		if(ConnectReturn == 0 )		
		break;
	}
	
	//parse this_flow into JSON string and copy it into sendbuffer
	parse_json(this_flow);
	printf("parsed\n"); 
	for(;;)
	{
		//send the buffer to the manager
		sendReturn = send(Newsock,JSONBuffer,sizeof(JSONBuffer),0);
		printf("send jsonbuffer"); 
		if(sendReturn>=0)
		break;
	}

	//close the connection and the socket
	shutdown(Newsock,2);
	close(Newsock);
	
	//connect to the server
	ConnectReturn = connect(sock,(struct sockaddr *)&servaddr, addrlen);
	printf("connect to server\n"); 
	if(ConnectReturn!=0)
	{
		printf("%d\n",ConnectReturn);
		printf("errno = %d\n", errno); 
		perror("failed");  
		printf("error: %s\n", strerror(errno)); 
		return ConnectReturn;//return the return value of connect() to the server 
		exit(0);
	}//the end of if
	else
	{
		//print some necessary information to the user
		printf("Connected to the server\n\n");
		return ConnectReturn;//return the return value of connect() to the server 
	}//the end of else

}//the end of connect_init


int accept_init(int sock,struct sockaddr_in clientaddr, socklen_t addrlen, struct extra_information this_flow)
{
	int sock2;
	int sock3;
	int sendReturn;
	int ConnectReturn;
	char *clientIP;
	
	ManagerAddr.sin_family= AF_INET;//TCP/IP suites
	ManagerAddr.sin_addr.s_addr=inet_addr("10.0.2.15");//the ip address of the manager
	ManagerAddr.sin_port = htons(1501);//the port number used for connection of the manager
	ManagerAddrlen = sizeof(ManagerAddr);//calculates the length of the manager address
	//accept a new connection
	
	if((sock2=accept(sock, (struct sockaddr*) &clientaddr, &addrlen))<0)
	{
		printf("accept() Failed.\n");
		printf("%d\n",sock2);
		exit(0);
	}
	else
	{	clientIP = inet_ntoa(clientaddr.sin_addr);
		printf("Handle a new client:%s\n",clientIP);
		printf("Sock: %d\n",sock2);
		
	}
	
	
	if(trigger==2)
	{
	this_flow.DesIP = inet_ntoa(clientaddr.sin_addr);
	this_flow.role = 2;
	parse_json(this_flow);
	sock3 = socket(PF_INET,SOCK_STREAM,0);
	if(sock3<0)
	{
		printf("Socket\n");
		printf("errno = %d\n", errno); 
		perror("failed");  
		printf("error: %s\n", strerror(errno)); 
	}
	//connect to the manager
	ConnectReturn = connect(sock3,(struct sockaddr *)&ManagerAddr,ManagerAddrlen);
	if(ConnectReturn!=0)
	{
		printf("Connect\n");
		printf("errno = %d\n", errno); 
		perror("failed");  
		printf("error: %s\n", strerror(errno)); 
	}
	
	//send the buffer to the manager
	sendReturn = send(sock3,JSONBuffer,sizeof(JSONBuffer),0);
	printf("%s\n",JSONBuffer);
	if(sendReturn<0)
	{
		printf("send\n");
		printf("errno = %d\n", errno); 
		perror("failed");  
		printf("error: %s\n", strerror(errno)); 
	}

	
	//close the connection and the socket
	shutdown(sock3,2);
	close(sock3);
	}//the end of if
	return sock2;
}
//parse this_flow into JSON string 
void parse_json(struct extra_information newflow)
	{

	int sp;//used for add jsonstring, making JSON form
	int sp1;//used for modifing ddl
	int sp2;//used for modifing send time
	char buffer[50] = {0};
	char buffer1[50] ={0};//used for modifing ddl 
	char buffer2[50]={0};//used for modifing sendtime
	char buffer3[20]={0};//used for adding string into buffers
	memset(buffer1,0,50);//clear the buffer
	memset(buffer2,0,50);//clear the buffer
	memset(buffer3,0,20);//clear the buffer
	memset(JSONBuffer,0,1024);//clear the buffer

		
	/*
	the forms of the ddl will be "NULL" or "Tue Jun 02 22:20:14 1970"
	first the codes need to judge it is a real time or NULL
	then separate the ddl string into several parts like ["","",""]
	*/
	
	
	sprintf(buffer,"%s",newflow.deadline);
	if(strcpy(buffer,"")==0)
		strcpy(newflow.deadline,"\"NULL\",\"NULL\",\"NULL\",\"NULL\",\"NULL\"");
	else
	{
		strncpy(buffer3,newflow.deadline,3);
		sp1 = sprintf(buffer1,"\"%s\",",buffer3);
  	
  		strncpy(buffer3,newflow.deadline+3,4);
  		sp1 += sprintf(buffer1+sp1,"\"%s\",",buffer3);
  	
  	
  		strncpy(buffer3,newflow.deadline+7,4);
  		sp1 += sprintf(buffer1+sp1,"\"%s\",",buffer3);
  	
  	
  		strncpy(buffer3,newflow.deadline+11,8);
  		sp1 += sprintf(buffer1+sp1,"\"%s\",",buffer3);
  	
  		strncpy(buffer3,newflow.deadline+19,7);
  		sp1 += sprintf(buffer1+sp1,"\"%s\"",buffer3);
  		
  		memset(buffer3,0,10);//clear the buffer

	}
	  
	
	/*
	the forms of the sendtime will be like "Tue Jun 02 22:20:14 1970"
	*/
	strncpy(buffer3,asctime(gmtime(&newflow.sendtime)),3);
	sp2 = sprintf(buffer2,"\"%s\",",buffer3);
  	
	strncpy(buffer3,asctime(gmtime(&newflow.sendtime))+3,4);
	sp2 += sprintf(buffer2+sp2,"\"%s\",",buffer3);
  	
  	
	strncpy(buffer3,asctime(gmtime(&newflow.sendtime))+7,4);
	sp2 += sprintf(buffer2+sp2,"\"%s\",",buffer3);
  	
  	
	strncpy(buffer3,asctime(gmtime(&newflow.sendtime))+11,8);
	sp2 += sprintf(buffer2+sp2,"\"%s\",",buffer3);
  	
	strncpy(buffer3,asctime(gmtime(&newflow.sendtime))+19,7);
	sp2 += sprintf(buffer2+sp2,"\"%s\"",buffer3);
	
 	
	/*
	if the value needs double quotation, "dq" will be used as a annotation
	*/

	sp = sprintf(JSONBuffer,"{\n\t\"flowSize\": %lf",newflow.flowSize);
	sp += sprintf(JSONBuffer+sp,"\t\"UrgentMode\": %d",newflow.UrgentMode);
	sp += sprintf(JSONBuffer+sp,"\t\"flowSpeed\": %lf",newflow.speed); 
	sp += sprintf(JSONBuffer+sp,"\t\"Deadline\": [%s]",buffer1);//dq
	sp += sprintf(JSONBuffer+sp,"\t\"TransmissionTime\": [%s]",buffer2);//dq
	sp += sprintf(JSONBuffer+sp,"\t\"TransmissionMode\": %d",newflow.TransmissionMode);
	sp += sprintf(JSONBuffer+sp,"\t\"Role\": %d",newflow.role);
	sp += sprintf(JSONBuffer+sp,"\t\"DestinationAddress\": \"%s\"}",newflow.DesIP);//dq

}


