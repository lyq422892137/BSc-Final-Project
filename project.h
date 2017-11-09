#ifndef __PROJECT_H__
#define __PROJECT_H__



typedef struct extra_information{
	double flowSize;//the size of the flow in Mb
	int UrgentMode ; //an integer from 1-9, 1 is not urgent at all, 9 is extremely urgent, 0 is not set
	double speed;//this is the default value of the speed, the unit is Mbps;
	char deadline[25];//the deadline of this flow
	time_t sendtime;//the transmission time of the flow
	int TransmissionMode;// 1 is unidirectional, 2 is bidirectional and 0 is not set 
	char *DesIP;//the ip address of the client's destination
	int role;// 1 is client , 2 is server, must be set in project.c
	}this_flow;
	

/*
information can be NOT SET:
filesize
urgentmode
speed
sendtime
transmissionmde
*/
//ddl should be set "NULL" if it is NOT SET

char JSONBuffer[1024];//the buffer to be sent to the manager in JSON

int trigger;//the trigger to send servers' conditions to the manager, 2 is send

int connect_init(int sock,struct sockaddr_in servaddr, int addrlen, struct extra_information this_flow);

int accept_init(int sock,struct sockaddr_in clientaddr, socklen_t addrlen, struct extra_information this_flow);

void parse_json(struct extra_information newflow);//parse struct extra_information this_flow into JSON string

#endif /* __PROJECT_H__ */
