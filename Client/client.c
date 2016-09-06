#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <err.h>
#include <errno.h>
#include <arpa/inet.h>


const int MAX_SEGMENT_SIZE=1472;
struct timeval start;
struct timeval end;

long TotalTime(struct timeval start,struct timeval end) {
	long time=(end.tv_sec * 1000000 + end.tv_usec)-(start.tv_sec * 1000000 + start.tv_usec);
	return time;
}


//structure for file parameters and traversing through file,return values together
struct files{
	char name[20][30];
	int fileCount;
};

//extract file by file from list of files
struct files extractFileNames(char *fName){

	struct files fileObj;
	fileObj.fileCount=0;
	int count=0;
	int lengthOfFile;
	char *contentOfFile;
	char *fileInList;

	FILE *filePointer;


	filePointer=fopen(fName,"r");
	fseek(filePointer, 0, SEEK_END);
	if(filePointer == NULL)
	{
	printf("error opening file");
	}
	else
		{
			lengthOfFile=ftell(filePointer);
			fseek(filePointer,0,SEEK_SET);
			contentOfFile=(char *)calloc(1,lengthOfFile);
			fread(contentOfFile,1,lengthOfFile,filePointer);
			printf("Contents in file :: %s \n",contentOfFile);
			fclose (filePointer);
			fileInList=strtok(contentOfFile,"\n");
					while(fileInList != NULL)
					{
						strcpy(fileObj.name[count],fileInList);
						count++;
						fileObj.fileCount++;
						fileInList=strtok(NULL,"\n");
					}
			}
			return fileObj;
}

void sendRequestToServer(int socketFileDescriptor,struct files fileObj, char *argv[],int indexOfFile,int persistence)
{

	char buffer[MAX_SEGMENT_SIZE];
	char statusLine[50]={0};
	char headerLine[100]={0};
	char connType[30]={0};
	char *completeRequest;
	int writeStatus,readStatus=1;

	strcat(statusLine, "GET /");
	strcat(statusLine,fileObj.name[indexOfFile]);
	strcat(statusLine, " HTTP/1.1\r\n");
	strcat(headerLine, "Host: ");
	strcat(headerLine,argv[1]);
	strcat(headerLine, ":");
	strcat(headerLine,argv[2]);
	strcat(headerLine,"\n");

	if(persistence)
	{
		strcat(connType, "Connection: keep-alive\n");
					}
	else
	{
		strcat(connType, "Connection: close\n");
	}

	int totalLength;
	totalLength=strlen(statusLine)+strlen(headerLine)+strlen(connType);
	completeRequest=(char *) calloc(totalLength,sizeof(char));

	strcat(completeRequest,statusLine);
	strcat(completeRequest,headerLine);
	strcat(completeRequest,connType);
	puts(completeRequest);
	gettimeofday(&start,NULL);
		printf("Last packet sent at %d\n",start.tv_usec);
	writeStatus=write(socketFileDescriptor,completeRequest,totalLength);
	if(writeStatus<0)
	{
		printf("error writing from client");
	}


	int packetCount=0;
	bzero(buffer,MAX_SEGMENT_SIZE);
	while(readStatus>0){
	readStatus=read(socketFileDescriptor,buffer,MAX_SEGMENT_SIZE);
	if(readStatus<0)
		{
			printf("error during read from client");
		}
	puts(buffer);
	printf("number of bytes read is %d \n \n \n",readStatus);
	++packetCount;
	if(readStatus<(MAX_SEGMENT_SIZE))
	{
		printf(" total count of packets is %d \n",packetCount);
		bzero(buffer,MAX_SEGMENT_SIZE);
		gettimeofday(&end,NULL);
					long timetaken=TotalTime(start,end);
					printf("total time is %ld \n",timetaken);
					break;
		break;
	}
	}

}



//set paramters of server and intitiate
int setUpConnectionWithServer(char *serverHostName,char *portNumber)
{
	printf("inside setUpConnectionWithServer \n");
	struct sockaddr_in serverAddress;
	struct hostent *server;
	int socketFd;
	int port;
	int connectStatus=0;
	port=atoi(portNumber);
	socketFd=socket(AF_INET,SOCK_STREAM,0);
	printf("socket id is %d \n",socketFd);
	printf("port number is %d \n",port);
	if(socketFd<0)
	{
		printf("error while opening socket");
	}
		server = gethostbyname(serverHostName);
		bzero((char *) &serverAddress,sizeof(serverAddress));

		memcpy(&serverAddress.sin_addr,server->h_addr_list[0],server->h_length);
		serverAddress.sin_family=AF_INET;
		serverAddress.sin_port = htons(port);
		printf("before connect \n");
		connectStatus=connect(socketFd,(struct sockaddr *)&serverAddress,sizeof(serverAddress));
		if(connectStatus<0)
		{

			printf("error connecting : %s",strerror(errno));
			exit(2);
		}
		printf("Connected to server \n");
		return socketFd ;
}



int main(int argc, char *argv[]) {
	printf("start client");
	struct files fileObj;
	int socketFileDesc;
	int num=0;
	int persistenceTrue;
	int i;
	if (argc!=5){
		printf("Please provide correct input :: hostname,port,persistence/non ,filepath url");
		exit(1);
	}
	for (i=1; i< argc; i++) {
	     printf("\n arg %d =%s", i, argv[i]);
	 }
	printf("\n arguments received \n");
	printf("argv 1 is %s \n",argv[1]);
	printf("port number is %s \n",argv[2]);
	socketFileDesc=setUpConnectionWithServer(argv[1],argv[2]);

	//check for connection type
	if (strcmp(argv[3],"p") == 0)
	{
		persistenceTrue=1;
		fileObj=extractFileNames(argv[4]);
	}
	else
	{
		persistenceTrue=0;
	}

	//call to server based on conn type
	if(persistenceTrue)
		{
			for (num=0;num<fileObj.fileCount;num++)
			{
				sendRequestToServer(socketFileDesc,fileObj,argv,num,persistenceTrue);
			}
		}
		else
		{
			strcpy(fileObj.name[0],argv[4]);
			sendRequestToServer(socketFileDesc,fileObj,argv,0,persistenceTrue);
		}

return 0;
}


