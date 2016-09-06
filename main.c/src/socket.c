/*
 * socket.c
 *
 *  Created on: Sep 11, 2015
 *      Author: abhinandan
 */


#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<pthread.h>

const int MAX_SEGMENT_SIZE=1472;
struct httpRequest{
	char method[20];
	char url[50];
	char version[10];
};


//structure to process requests
struct httpRequest httpprocessGet(char headerline[]){

	struct httpRequest httpRequestObject;
	sscanf(headerline,"%s %s %s",httpRequestObject.method,httpRequestObject.url,httpRequestObject.version);
	printf("\n httpRequestLocation : %s \n",httpRequestObject.method);
	printf("httpRequestType : %s \n",httpRequestObject.url);
	printf("httpRequestVersion : %s \n",httpRequestObject.version);
	return httpRequestObject;
}

//to check for connection status
int	 checkPersistence(char connLine[])
{
	//struct httpRequest httpRequestObject;
    char connectionType[20];
	//puts(connLine);
	sscanf(connLine,"\n Connection : %s \n",connectionType);
	printf("\n connectionType is : %s \n",connectionType);
	if(strcmp(connectionType,"keep-alive")==0)
	{
		return 1;
	}
	else if (connectionType==NULL) {
	return 0;
	}
	else
		return 0;

}

//build response based on type of requests received
int createHttpResponse(int cfd, struct httpRequest httpRequestObject){
	printf("inside createHttpResponse");
	FILE *filePointer;
	char responseStausLine[20]={0};
	char responseHeaderLine[40]={0};
	char responseBody[100]={0};



	char *contentOfFile;
	int urlLength=strlen(httpRequestObject.url);
	//printf(" \n url is %s \n length is %d \n path is %s \n\n", httpRequestObject.url,urlLength,httpRequestObject.url+1);

	char *filepath=strdup(httpRequestObject.url+1);
	printf("location is %s",filepath);
	filePointer=fopen(filepath,"r");
	if(strcmp(httpRequestObject.version,"HTTP/1.1")!=0)
	{
		strcat(responseStausLine,"HTTP/1.1 404 Not Found\n");
		strcat(responseHeaderLine,"Content-Type:text/html; charset=utf-8\n");
		strcat(responseBody,"Content-Length:11");
		strcat(responseBody,"\r\n");
		contentOfFile=(char *)calloc(1,11);
		strcat(contentOfFile,"Bad request");
	}
	else if(filePointer == NULL)
		{
		printf("Error opening the file \n");
		strcat(responseStausLine,"HTTP/1.1 400 Bad Request\n");
		strcat(responseHeaderLine,"Content-Type:text/html; charset=utf-8\n");
		int badLength=strlen("File not found error");
		char badLengthInString[20];
		sprintf(badLengthInString,"%d",badLength);
		strcat(responseBody,"Content-Length:");
		strcat(responseBody,badLengthInString);
		strcat(responseBody,"\r\n");
		contentOfFile=(char *)calloc(1,50);
		strcat(contentOfFile,"File not found error");

		}
	else{
	strcat(responseStausLine,"HTTP/1.1 200 OK\n");
	strcat(responseHeaderLine,"Content-Type:text/html; charset=utf-8\n");
	//strcat(responseHeaderLine,"charset:utf-8")
	//strcat(responseHeaderLine,"\n");
	fseek(filePointer,0,SEEK_END);
	int lengthOfFile;
	lengthOfFile=ftell(filePointer);
	printf("length of file is %d \n",lengthOfFile);
	char lengthOfFileInString[20];
	strcat(responseBody,"Content-Length:");
	sprintf(lengthOfFileInString,"%d",lengthOfFile);
	strcat(responseBody,lengthOfFileInString);

	strcat(responseBody,"\r\n");
	fseek(filePointer,0,SEEK_SET);
	//contentOfFile=(char *)calloc(1,lengthOfFile);
	contentOfFile=(char *) calloc(lengthOfFile,sizeof(char));
	fread(contentOfFile,lengthOfFile,1,filePointer);
	fclose (filePointer);
	}


	int totalLengthOfResponse=strlen(responseStausLine)+strlen(responseHeaderLine)+strlen(responseBody)+strlen(contentOfFile);
	//printf("Length of Final is %d\n",totalLengthOfResponse);

	//char *completeResponse=(char *)calloc(1,totalLengthOfResponse);
	char *completeResponse=(char *)calloc(totalLengthOfResponse,sizeof(char));
	bzero(completeResponse,totalLengthOfResponse);
	strcat(completeResponse,responseStausLine);
	strcat(completeResponse,responseHeaderLine);
	strcat(completeResponse,responseBody);
	strcat(completeResponse,contentOfFile);
	printf("total response lenght is %d \n",strlen(completeResponse));
	//puts();
	//puts(responseBody);
	//printf("Printing the final Response being sent %s \n\n ",completeResponse);
	int writtenBytes=0;
	writtenBytes=write(cfd ,completeResponse ,strlen(completeResponse));
	printf("number of bytes written to client is %d \n",writtenBytes);
	return 0;
}

//start the server and return socketfileDescriptor
int initiateServer(int port){
	int socketFiledescriptor;
	struct sockaddr_in server;

	socketFiledescriptor=socket(AF_INET,SOCK_STREAM,0);
		if(socketFiledescriptor < 1)
			{
				printf("could not create socket");
			}
		bzero((char *) &server, sizeof(server));
		server.sin_addr.s_addr = INADDR_ANY;
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		if (bind(socketFiledescriptor, (struct sockaddr *) &server, sizeof(server)) < 0)
			    {
				 printf("\n ERROR during binding,please change the portnumber");
				 exit(1);
			    }
		//	 printf("\n 4");
			    listen(socketFiledescriptor,5);
			    printf("\n starting the tcp server");
			    return socketFiledescriptor;
}

//handle requests and process them
void HttpRequest(int newFileDesc){
		printf("\n inside HttpRequest \n");
		//char *requestBuffer=(char*)calloc(MAX_SEGMENT_SIZE,sizeof)
		char *requestBuffer= (char*) calloc(MAX_SEGMENT_SIZE,sizeof(char));
		int readStatus;
		int bufferLength;
 		char *lineContent;
		char delimiter[]="\r\n";
		char *temp;
		char *conn;
		int keepAlive;
		struct httpRequest httpRequestObject;
		//bzero(requestBuffer,MAX_SEGMENT_SIZE);
		printf("\n before cfd \n");
		int cfd= newFileDesc;



		bzero(requestBuffer,MAX_SEGMENT_SIZE);
		int n=0;
		printf("cfd from thread %d \n",cfd);
		while (1)
			{
			bzero(requestBuffer,MAX_SEGMENT_SIZE);
			readStatus = read(cfd,requestBuffer,MAX_SEGMENT_SIZE);
			if(readStatus==0)
			{
			continue;
			}
			if(readStatus<0)
			{
				printf("error reading");

			}

			//get line on delimiter \r\n
			lineContent=strtok(requestBuffer,delimiter);
			printf("\n request number : %d",n+1);
			while(lineContent!=NULL){


			printf("\n content is....... %s",lineContent);
			temp=strstr(lineContent,"GET");

			//get statusline
			if(temp!=NULL)
			{
				printf("\n get request is....%s",temp);
				httpRequestObject=httpprocessGet(temp);
			}

			//get connectiontype
			conn=strstr(lineContent,"Connection: ");
			if(conn!=NULL)
			{
				printf("\n connection type is.....%s",conn);
				keepAlive=checkPersistence(conn);
				printf("keepalive status is %d \n",keepAlive);

			}
			lineContent=strtok(NULL,"\n");
		}

			n=n+1;
			printf("\n response number : %d \n",n);
			if(requestBuffer!=NULL)
			{
				//sleep(1);
			createHttpResponse(cfd,httpRequestObject);
			printf("done sending \n");
			}
			//if non persistent
			if(keepAlive==0)
			{
				printf("keepAlive==0 \n");
				printf("closing connection \n");
				break;
			}
			//sleep(2);
			}
		printf("closing cfd \n");
		close(cfd);
}

//accept and return newFileDescriptor
int listenConnection(int sfd){
		printf("inside listenConnection");
	     int newfd;
	     socklen_t clientLength;
	     struct sockaddr_in clientAddress;
	     clientLength = sizeof(clientAddress);
	     //clilen = sizeof(cli_addr);
	     newfd = accept(sfd,(struct sockaddr *) &clientAddress,&clientLength);
	     if (newfd < 0)
	     {
	          error("ERROR on accept");
	     }
	     printf("inside listencConnection :: cfd is %d \n", newfd);
	          return newfd;
	}





int main(int argc, char **argv) {


	if (argc!=2){
					printf("Please provide correct input :: ./ProgramName port_number");
					exit(1);
				}
		int portNUmber=atoi(argv[1]);
		printf("!!!Port number is %d \n",portNUmber);

	//int portNUmber=20202;
	int newFiledesc;
	int num=0;
	struct sockaddr_in client;
	socklen_t clientLength;
	int socketFiledescriptor;
	pthread_t pThread[20];
	int threadCount = 0;
	socketFiledescriptor=initiateServer(portNUmber);
		printf("\n sfd is %d",socketFiledescriptor);


		newFiledesc=accept(socketFiledescriptor,(struct sockaddr *) &client, &clientLength);
		//multithreading
		//HttpRequest function handles functionality of the whole request response which is passed to thread
		//for(threadCount=0;threadCount<20;threadCount++)
		while(threadCount<20){
			printf("creating thread %d \n",threadCount);
			int threadStatus=pthread_create(&pThread[threadCount],NULL,&HttpRequest,newFiledesc);
			threadCount=threadCount+1;
			if(threadStatus)
			{
				printf("thread creation error \n");
				exit(2);
			}
			newFiledesc=accept(socketFiledescriptor,(struct sockaddr *) &client, &clientLength);
		}//pthread_join(&pThread[threadCount],NULL);
	close(socketFiledescriptor);
	return 0;
}








