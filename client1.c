/*
Author: LI Yuqi
All Rights Reserved @copyright
Project Code
*/

#include<stdio.h>//printf()
#include<sys/socket.h>//socket(),send(),recv()
#include<arpa/inet.h>//sockaddr_in and inet_addr()
#include<stdlib.h>//exit()
#include<unistd.h>//close()
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h> 
#include <time.h>//time() localtime() asctime()
#include<string.h>//memset()
#include"project.h"
//functions declaration
int connect_init(int sock,struct sockaddr_in servaddr, int addrlen, struct extra_information this_flow);
double calculate_fileSize(struct extra_information* this_flow);
int set_urgent(struct extra_information* this_flow);
double set_speed(struct extra_information* this_flow);
void calculate_ddl(struct extra_information* this_flow);
void set_transmissionTime(struct extra_information* this_flow);
int sendfile(int sock, struct extra_information* this_flow);
int recvfile(int sock, struct extra_information this_flow);

//the main function
int main()
{

	//variables declaration
	struct extra_information this_flow;//the structure defined in the head file
	struct sockaddr_in DesAddr;//the structure of the server address
 	int sock1 = -2;//used for connecting to the server
	int DesAddrLen;//the length of the server's IP address	
	int transFlag1 = 1;//the transmission flag, the default value is 1--update. 0 is download mode
	char* transFlag2 = (char*) malloc(0); //the transmission choice flag
	
	DesAddr.sin_family = AF_INET;//Unix
	DesAddr.sin_addr.s_addr = inet_addr("10.0.2.15");//the ip address of the server
	DesAddr.sin_port = htons(20000);// the port number of the server
	
	DesAddrLen= sizeof(DesAddr);//the lenght of the server address

	//build up the first socket used for communication to the server
	if((sock1=socket(PF_INET,SOCK_STREAM,0))==-1)
		{	
			printf("Sorry, socket establishment failed.\n");
			exit(0);
		}
	else
			printf("Socket Establishment Succeeded.\n");
	
	
	/*
	the following codes are about get the file name and transmission mode of this communication
	*/
	//set transmission mode and do corresponding steps
	printf("\n-------------------------------------------\n");
	printf("Please input \"y\" if you want to send a file to a server and \"n\" is not.\n");
	scanf("%s",transFlag2);
	printf("\n-------------------------------------------\n");
	//select the file the user wants to send or download
	printf("\n-------------------------------------------\n");
	printf("Please select the file you want to send/download\n");
	scanf("%s",this_flow.filename);//get this_flow->filename
	printf("\n-------------------------------------------\n");
	
	//judge whether it is update files or download files
	if (strcmp(transFlag2,"y")!= 0&&strcmp(transFlag2,"n")!= 0)
	{
		printf("Please input again.\n");
		scanf("%s",transFlag2);
	}
	
	if(strcmp(transFlag2,"y")==0)//update mode
	{
		/*
		the following codes  will set this_flow values
		*/
		//get this_flow->fileSize
		calculate_fileSize(&this_flow);
	
		//get this_flow->UrgentMode
 		set_urgent(&this_flow);
		
		//get this_flow->speed
		set_speed(&this_flow);
		
		//get this_flow->TransmissionTime
		set_transmissionTime(&this_flow);
		
		//get this_flow->deadline
		calculate_ddl(&this_flow);
		
		//set this_flow.TransmisiionMode;
		this_flow.TransmissionMode = 1;//update mode
		
		//connect to the server
 		if(connect_init(sock1,DesAddr,DesAddrLen,this_flow)==0)
 			printf("Connection Established.\n");
		else
		{
			printf("Sorry, connection failed.\n");
			exit(0);
		}
		//send the file to the server
		sendfile(sock1,&this_flow);
	}//the end of if update mode
	
	if(strcmp(transFlag2,"n")==0)//download
	{
		/*
		the following codes  will set this_flow values
		*/
		//set file size
		this_flow.fileSize = 0.00;
	
		//get this_flow->UrgentMode
 		set_urgent(&this_flow);
	
		//get this_flow->sendtime
		set_transmissionTime(&this_flow);
		
		//set this_flow. ddl
		strcpy(this_flow.deadline,"NULL"); 
		//set this_flow.TransmisiionMode;
		this_flow.TransmissionMode = 1;//download mode
		
		
		if(connect_init(sock1,DesAddr,DesAddrLen,this_flow)==0)
 			printf("Connection Finished.\n");
		else
		{
			printf("Sorry, connection failed.\n");
			exit(0);
		}
		
		printf("receive the file!!!!!!!!!!\n");
		if(recvfile(sock1,this_flow)<0)
			printf("Sorry, Receiving the file failed.\n");
	}// the end of if download mode
	return 1;
	
}



double calculate_fileSize(struct extra_information* this_flow)
{
	printf("\n");
	
	FILE *fp;
	fp = fopen(this_flow->filename, "r"); 
  
    if(fp == NULL) 
   { 
    	printf("File: %s can not open or NO such file\n", this_flow->filename); 
    	exit(1);
    } 
    
    //stat is a structure used to deal with files in <sys/stat.h>
	struct stat statbuf;
	//copy the filename into statbuf
	stat(this_flow->filename,&statbuf);
	//copy the fileSize from statbuf.st_size to this_flow
	this_flow->fileSize = statbuf.st_size;
	
	//judge whether the fileSize >0
	if(this_flow->fileSize<=0.00)
	{	
		printf("Sorry, the size of your file is %lf\n",this_flow->fileSize);
		exit(1);
	} 

	return this_flow->fileSize;
    
}

int set_urgent(struct extra_information* this_flow)
{
	//choose whether the user wants to set the urgent number
	char* judgeWord = (char*) malloc(0);//the input value of whether the user wants to set the urgent mode or not
	printf("\n-------------------------------------------\n");
	printf("Do you want to set a urgent mode for this file?\n");
	printf("Please input \"y\" or \"n\"\n");
	scanf("%s",judgeWord);
	printf("\n-------------------------------------------\n");
	//judge the choice
	
	if (strcmp(judgeWord,"y")!= 0&&strcmp(judgeWord,"n")!= 0)
	{
		printf("please try again.\n"); 
		printf("Please input \"y\"(YES) or \"n\"(NO)\n");
		scanf("%s",judgeWord);
				
	}
	
	if(strcmp(judgeWord,"n")==0)//the user does not want to set urgentMode
		this_flow->UrgentMode = 10;
	
	if(strcmp(judgeWord,"y")==0)//the user wants to set urgentMode
	{
		//instruction
		printf("\n-------------------------------------------\n");
		printf("Please define whether the file is urgent or not.\n");
		printf("You should input an integer from 0 to 9 to define the urgent level.\n");
		printf("9 means extremely urgent, 0 means not urgent at all.\n");
		printf("Now you can input the number:\n");
		printf("\n-------------------------------------------\n");
		//get the value
		scanf("%d",&this_flow->UrgentMode);

		//judge whether this value is in the range
		if((this_flow->UrgentMode<0)||(this_flow->UrgentMode)>9)
		{
			printf("Sorry, your number is not in the range, please input again:\n");
			scanf("%d",&this_flow->UrgentMode);
		
		}
	}//the end of the second else of
	
	return this_flow->UrgentMode;
}

double set_speed(struct extra_information* this_flow)
{

	char* Speedflag = (char*) malloc(0);
	//judge whether the user wants to set the speed of the flow
	printf("\n-------------------------------------------\n");
	printf("Do you want to set a speed?\nPlease input \"y\"(YES) or \"n\"(NO)\n");
	scanf("%s",Speedflag);
	printf("\n-------------------------------------------\n");
	
	if(strcmp(Speedflag,"n")==0)
		this_flow->speed = 0.00;
	else if(strcmp(Speedflag,"y")==0)
	{
		//instruction
		printf("\n-------------------------------------------\n");
		printf("Please input the transferring speed of your speed:\n");
		printf("The unit of this speed is Gb/s.\n");
		printf("The range of the speed should between 0.1000 to 280.0000\n");
		printf("\n-------------------------------------------\n");
		//usually, the speed could be up to 10GB/s
		//according to news from The Hong Kong Polytechnic University, their data centres' speed could up to 240GB/s, which could be the most fast in the world now. 
		scanf("%lf",&this_flow->speed);

		//check the range
		if((this_flow->speed<0.1000)||(this_flow->speed)>280.0000)
		{
			printf("Sorry, your number is not in the range, please input again:\n");
			scanf("%lf",&this_flow->speed);
		}
		
	}//the end of else
		
	return this_flow->speed;
}
 
void set_transmissionTime(struct extra_information* this_flow)
{
	char* transFlag = (char*) malloc(0);
	tzset();//get the time zone
	//judge whether the user wants to send the transmission time
	printf("\n-------------------------------------------\n");
	printf("Do you want to show the transmission time?\nPlease input \"y\"(YES) or \"n\"(NO)\n");
	scanf("%s",transFlag);
	printf("\n-------------------------------------------\n");
	if(strcmp(transFlag,"n")==0)
		this_flow->sendtime = 0;// the time received should be Year 1900
	if(strcmp(transFlag,"y")==0)
		this_flow->sendtime = time(NULL);//get the local system time	
}

void calculate_ddl(struct extra_information* this_flow)
{
	char* ddlFlag = (char*) malloc(0);
	int hours;//the hour of the dll 24-h
	int minutes;//the minute of the ddl
	int days;//the day of the ddl 
	time_t ddl;//the ddl of the flow
	struct tm *area;//the structure used for store ddl
	tzset();//get the time zone
	//judge whether the user wants to set the speed of the flow
	
	printf("\n-------------------------------------------\n");
	printf("Do you want to set a deadline?\nPlease input \"y\"(YES) or \"n\"(NO)\n");
	scanf("%s",ddlFlag);
	printf("\n-------------------------------------------\n");
	if(strcmp(ddlFlag,"n")==0)
		strcpy(this_flow->deadline,"NULL");// the time received should be Year 1900
	if(strcmp(ddlFlag,"n")==1)
	{
		printf("\n-------------------------------------------\n");
		printf("Please set the day of the deadline. (Integer)\n");
		scanf("%d",&days);
		printf("\n-------------------------------------------\n");
		printf("Please set the hour of the deadline. (Integer 0-24)\n");
		scanf("%d",&hours);
		printf("\n-------------------------------------------\n");
		printf("Please set the minute of the deadline. (Integer)\n");
		scanf("%d",&minutes);
		printf("\n-------------------------------------------\n");
		ddl = time(NULL);//get the local system time first in second
		area = localtime(&ddl);//get the local system time in structure
		area->tm_mday = days;//change the day
		area->tm_hour = hours;//change the hour
		area->tm_min = minutes;//change the minute
		strcpy(this_flow->deadline,asctime(area));
		printf("The Deadline of this flow is %s.\n",this_flow->deadline);
		printf("\n-------------------------------------------\n");
	}		

} 

int sendfile(int sock, struct extra_information* this_flow)
{
	//decalaration
	FILE *fp; 
	char filebuffer[1024];//the buffer used to send()
	int ReturnValue;//the return value of send()
	//open the file
	if((fp = fopen(this_flow->filename,"r"))==NULL)
	{
		printf("Cannot open the file.\n");
		exit(0);
	}
	else
		printf("Open the file %s.\n",this_flow->filename);
		
	printf("Reading the file content into the buffer now...\n");
	for(;;)
	{
		fread(filebuffer,1024,1,fp);
		fseek(fp,1024,SEEK_CUR);
		if((ReturnValue=send(sock,filebuffer,sizeof(filebuffer),0))<0)
		{
			printf("Sorry. Sending failed.\n");
			printf("Sending again.\n");
		}
		else
		{
			printf("%s\n",filebuffer);
			break;
		}
			
		
	//clear the send buffer
	memset(filebuffer,0,sizeof(filebuffer));	
			
	}// the end of for loop
	//close the file
	fclose(fp);
	return ReturnValue;
}

int recvfile(int sock, struct extra_information this_flow)
{
	int recvMsgSize = 0;//the size of the received flow
	int ReturnValue = 0;//the return value of send()
	char buffer[1024];//the received buffer
	FILE *fp;//file descriptor
	
	//if the user wants to download a file, the first 8 bits of the first buffer are always "01010101"
	strcpy(buffer,this_flow.filename);//copy the file name into the buffer
 	strcat(buffer,"01010101");//add "01010101" into the buffer
 	
	//send the file name to the server
	if((ReturnValue=send(sock,buffer,sizeof(buffer),0))<0)
		printf("Sorry. Sending the filename failed.\n");
	else
		printf("Request Sent.\n");
	memset(buffer,0,1024);//after send the buffer, clear the buffer
		
	//receive the file
	recvMsgSize =recv(sock, buffer, sizeof(buffer),0);
	printf("Received:\n\n");
	printf("%s\n",buffer);
	
	//wirte the buffer into clientRecv.txt
	if((fp = fopen("clientRecv.txt","wb")) == NULL)
	{
		fprintf(stderr,"Writing the buffer to the file failed.\n");
		exit(0);	
	}
	else
	{
		
		fwrite(buffer,sizeof(buffer),1,fp); 
		memset(buffer,0,1024);//after send the buffer, clear the buffer
		fclose(fp);
	}
	
	return recvMsgSize;
}
