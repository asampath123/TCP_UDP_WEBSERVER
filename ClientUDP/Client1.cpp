#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <err.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fstream>
#include <iostream>
#include <string>
using namespace std;
struct timeval start;
struct timeval end;

const int MAX_SEGMENT_SIZE=1472;
struct timeval tv;
struct timezone tz;



//struct to get details of server as objects
struct serverDetails{
	struct sockaddr_in serverAddress;
	int socketFileDesc;

};

long TotalTime(struct timeval start,struct timeval end) {
	long time=(end.tv_sec * 1000000 + end.tv_usec)-(start.tv_sec * 1000000 + start.tv_usec);
	return time;
}

//set arguments,prepare request to be sent to the server.
char * sendRequestToServer(char *argv[])
{
	//char buffer[1024];
	char statusLine[50]={0};
	char headerLine[100]={0};
	char connType[30]={0};
	int totalLength;
	char *completeRequest;


	strcat(statusLine, "GET /");
	strcat(statusLine,argv[3]);
	strcat(statusLine, " HTTP/1.1\r\n");
	strcat(headerLine, "Host: ");
	strcat(headerLine,argv[1]);
	strcat(headerLine, ":");
	strcat(headerLine,argv[2]);
	strcat(headerLine,"\n");
	strcat(connType, "Connection: close\n");


	totalLength=strlen(statusLine)+strlen(headerLine)+strlen(connType);
	completeRequest=(char *) calloc(totalLength,sizeof(char));

	strcat(completeRequest,statusLine);
	strcat(completeRequest,headerLine);
	strcat(completeRequest,connType);
	return completeRequest;
	}



//set up connection with server with server details, returns server details in a structure
struct serverDetails setUpConnectionWithServer(char *serverHostName,char *portNumber)
{
	printf("inside setUpConnectionWithServer \n");
	struct serverDetails serverDetailsObj;
	struct hostent *server;
	int socketFd;
	int port;
	port=atoi(portNumber);
	socketFd=socket(AF_INET,SOCK_DGRAM,0);
	printf("socket id is %d \n",socketFd);
	printf("port number is %d \n",port);
	if(socketFd<0)
	{
		printf("error while opening socket");
	}
		server = gethostbyname(serverHostName);
		bzero((char *) &serverDetailsObj.serverAddress,sizeof(serverDetailsObj.serverAddress));
		memcpy(&serverDetailsObj.serverAddress.sin_addr,server->h_addr_list[0],server->h_length);
		serverDetailsObj.serverAddress.sin_family=AF_INET;
		serverDetailsObj.serverAddress.sin_port = htons(port);
		serverDetailsObj.socketFileDesc=socketFd;
		return serverDetailsObj ;
}

int main(int argc, char *argv[]){
	printf("start client");
	struct serverDetails serverDetailsObj;
	char *completeRequest;
	//char *receiveBuffer= (char*) calloc(MAX_SEGMENT_SIZE,sizeof(char));
	int sendtoStatus;
	int i;
	int recvFrom;
	ofstream fpointer;
	char receiveBuffer[MAX_SEGMENT_SIZE];
	if (argc!=4){
		printf("Please provide correct input :: hostname,port,filepath url");
		exit(1);
	}
	for (i=1; i< argc; i++) {
	     printf("\n arg %d =%s", i, argv[i]);
	 }
	printf("\n arguments received \n");
	printf("argv 1 is %s \n",argv[1]);
	printf("port number is %s \n",argv[2]);
	serverDetailsObj=setUpConnectionWithServer(argv[1],argv[2]);
	//bzero(completeRequest,MAX_SEGMENT_SIZE);

	completeRequest=sendRequestToServer(argv);

	printf("*** Final Request being sent:%s\n",completeRequest);
	//send to the server
	gettimeofday(&start,NULL);
	printf("Last packet sent at %d\n",start.tv_usec);
	sendtoStatus = sendto(serverDetailsObj.socketFileDesc,completeRequest,strlen(completeRequest),0,((sockaddr*)&(serverDetailsObj.serverAddress)),sizeof(struct sockaddr_in));
	if (sendtoStatus < 0) {
		printf("ERROR writing to socket\n");
	}

	//recieve from the server


		int packetCount=0;
		fpointer.open(argv[3],std::ios::out);



		while(1){
		//bzero(receiveBuffer,MAX_SEGMENT_SIZE);
		//bzero(buffer,MAX_SEGMENT_SIZE);
		recvFrom= recvfrom(serverDetailsObj.socketFileDesc,receiveBuffer,MAX_SEGMENT_SIZE,0,NULL,NULL);
		if(packetCount==0)
		{
		//puts(receiveBuffer);
		}
		//cout<<"received buffer length is"<<strlen(receiveBuffer)<<endl;
		//printf("number of bytes read is %d \n",recvFrom);
		packetCount=packetCount+1;
		//printf("count of packets is %d \n",packetCount);


		if (!fpointer.is_open())
		{
			cout<< "file not found, failed to open file -- "<<"Client.txt"<<endl;
			exit(1);
		}
		fpointer.write(receiveBuffer,recvFrom);
		if(recvFrom<MAX_SEGMENT_SIZE)
		{
			printf("last packet ----- count of packets is %d \n",packetCount);
			//puts(receiveBuffer);
			fpointer.close();

			gettimeofday(&end,NULL);
			long timetaken=TotalTime(start,end);
			cout<<"total time"<<timetaken<<endl;
			break;

		}

		}

	//gettimeofday(&tv, &tz);
	//printf("Time of last packet received at %d\n",tv.tv_usec);
	//printf("number of bytes received is %d \n",recvFrom);
	if (recvFrom < 0) {
	printf("ERROR reading from socket\n");
	}
		//printf("***Response from the server......:%s\n",receiveBuffer);

close(serverDetailsObj.socketFileDesc);
return 0;
}


