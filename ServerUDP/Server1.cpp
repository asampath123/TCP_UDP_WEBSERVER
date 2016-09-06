/*
 * socket.c
 *
 *  Created on: Sep 11, 2015
 *      Author: abhinandan
 */

#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <string>
#include <string.h>
#include<pthread.h>
#include<time.h>
#include <sys/time.h>
#include<fstream>
#include<stdlib.h>

using namespace std;
//structure to read time
struct timeval tv;
struct timezone tz;
const int MAX_SEGMENT_SIZE=1472;
//const int MAX_SEGMENT_DATA_SIZE=1460;

//structure to process http requests
struct httpRequest{
	char method[20];
	char url[50];
	char version[10];
};


//get initial lines of request
struct httpRequest httpprocessGet(char headerline[]){
	struct httpRequest httpRequestObject;
	sscanf(headerline,"%s %s %s",httpRequestObject.method,httpRequestObject.url,httpRequestObject.version);
	printf("\n httpRequestLocation : %s \n",httpRequestObject.method);
	printf("httpRequestType : %s \n",httpRequestObject.url);
	printf("httpRequestVersion : %s \n",httpRequestObject.version);
	return httpRequestObject;
}


//create respone base on the request
char *createHttpResponse(httpRequest httpRequestObject,int socketfd,struct sockaddr_in client)
{
	//printf("inside createHttpResponse");
	char responseStausLine[20]={0};
	char responseHeaderLine[40]={0};
	char responseBody[100]={0};
	char lengthOfFileInString[20];
	fstream filePointer;
	char *filepath;
	int lengthOfFile;
	int n;


	filepath=strdup(httpRequestObject.url+1);
	printf("location is %s",filepath);
	filePointer.open(filepath,std::ios::in);
	filePointer.seekg(0,ios::end);
	lengthOfFile = filePointer.tellg();
	cout<<"filelength is"<<lengthOfFile<<endl;
	char *contentOfFile=(char*) calloc(lengthOfFile,sizeof(char));



	//bad requests
	if(strcmp(httpRequestObject.version,"HTTP/1.1")!=0)
					{
		strcat(responseStausLine,"HTTP/1.1 404 Not Found\n");
		strcat(responseHeaderLine,"Content-Type:text/html; charset=utf-8\n");
		strcat(responseBody,"Content-Length:11");
		strcat(responseBody,"\n\n");
		contentOfFile=(char *)calloc(1,11);
		strcat(contentOfFile,"Bad request");
		n=sendto(socketfd,contentOfFile,strlen(contentOfFile),0,(struct sockaddr *)&client,sizeof(client));
		cout<<"bad request"<<endl;
				exit(1);
					}

	//if file not available,return not found
	if(filePointer == NULL)
		{
		printf("Error opening the file \n");
		strcat(responseStausLine,"HTTP/1.1 404 Not Found\n");
		strcat(responseHeaderLine,"Content-Type:text/html; charset=utf-8\n");
		//int badLength=strlen("File not found error");
		//char badLengthInString[20];
		//sprintf(badLengthInString,"%d",badLength);
		strcat(responseBody,"Content-Length:");
		//strcat(responseBody,badLengthInString);
		strcat(responseBody,"\n\n");
		contentOfFile=(char *)calloc(1,20);
		strcat(contentOfFile,"File not found error");
		n=sendto(socketfd,contentOfFile,strlen(contentOfFile),0,(struct sockaddr *)&client,sizeof(client));
		cout<<"file not found sent"<<endl;
		exit(1);
		}

	else{
	strcat(responseStausLine,"HTTP/1.1 200 OK\n");
	strcat(responseHeaderLine,"Content-Type:text/html; charset=utf-8\n");
	strcat(responseBody,"Content-Length:");

	sprintf(lengthOfFileInString,"%d",lengthOfFile);
	strcat(responseBody,lengthOfFileInString);

	strcat(responseBody,"\n\n");


	bzero(contentOfFile,lengthOfFile);



	if(filePointer.is_open()) {
	//cout<<"is open"<<endl;
	filePointer.seekg(0,ios::beg);
	filePointer.read(contentOfFile,lengthOfFile);
	}
	filePointer.close();


	//contentOfFile=(char *)calloc(MAX_SEGMENT_SIZE,sizeof(char*));
	//if (contentOfFile)
	//{
	//	fread(contentOfFile,1,lengthOfFile,filePointer);
	//}

		//fread(contentOfFile,MAX_SEGMENT_SIZE,first,filePointer);
		int totalLengthOfResponse=strlen(responseStausLine)+strlen(responseHeaderLine)+strlen(responseBody)+strlen(contentOfFile);



			char *completeResponse=(char *)calloc(totalLengthOfResponse,sizeof(char));
			//bzero(completeResponse,totalLengthOfResponse);
			strcat(completeResponse,responseStausLine);
			strcat(completeResponse,responseHeaderLine);
			strcat(completeResponse,responseBody);
			cout<<"header length is"<<strlen(completeResponse)<<endl;
			strcat(completeResponse,contentOfFile);
			cout<<"content of file is"<<strlen(contentOfFile)<<endl;
			cout<<"header length + full resp is"<<strlen(completeResponse)<<endl;

			return completeResponse;

		/*else if (count+1==(lengthOfFile/MAX_SEGMENT_SIZE+1)) {
		char *completeResponse=(char *)calloc(MAX_SEGMENT_SIZE,sizeof(char*));
		bzero(completeResponse,MAX_SEGMENT_SIZE);
		strcat(completeResponse,responseStausLine);
		strcat(completeResponse,responseHeaderLine);
		strcat(completeResponse,responseBody);
		strcat(completeResponse,contentOfFile);
		return completeResponse;*/


		//}
		//else{
		//	return contentOfFile;
			//puts(contentOfFile);
			//printf("done till here \n");
		//}
	}


	//printf("Printing the final Response being sent %s \n\n ",completeResponse);



}

//start the server
int initiateServer(int port){
	int socketFiledescriptor;
	struct sockaddr_in server;
	socketFiledescriptor=socket(PF_INET,SOCK_DGRAM,0);
		if(socketFiledescriptor < 1)
			{
				printf("could not create socket");
			}
		bzero((char *) &server, sizeof(server));
		server.sin_addr.s_addr = inet_addr("127.0.0.1");
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
			 if (bind(socketFiledescriptor, (struct sockaddr *) &server, sizeof(server)) < 0)
			    {
				 printf("\n ERROR during binding,please change the port number");
				 exit(1);
			    }
			    printf("\n starting the UDP server");
			    return socketFiledescriptor;
}

//parse the httprequest for details
struct httpRequest HttpRequest(char * requestBuffer,struct httpRequest httpRequestObject){
		printf("\n inside HttpRequest \n");
		char *lineContent;
		char delimiter[]="\r\n";
		char *headerLine;

		int n=0;

			lineContent=strtok(requestBuffer,delimiter);
			printf("\n request number : %d",n+1);
			while(lineContent!=NULL){
			printf("\n content is....... %s",lineContent);
			headerLine=strstr(lineContent,"GET");
			if(headerLine!=NULL)
			{
				printf("\n get request is....%s",headerLine);
				httpRequestObject=httpprocessGet(headerLine);
			}
			lineContent=strtok(NULL,"\n");
		}
			return httpRequestObject;
}

int main(int argc, char **argv) {

	if (argc!=2){
						printf("Please provide correct input :: ./ProgramName port_number");
						exit(1);
					}
	int portNUmber=atoi(argv[1]);
	printf("!!!Port number is %d \n",portNUmber);

	struct httpRequest httpRequestObject;
	struct sockaddr_in client;
	socklen_t clientLength;
	int socketFiledescriptor;
	socketFiledescriptor=initiateServer(portNUmber);
	printf("\n sfd is %d",socketFiledescriptor);

	int n;
	//char *requestBuffer=(char*) calloc(MAX_SEGMENT_SIZE,sizeof(char*));
	char requestBuffer[MAX_SEGMENT_SIZE];
	char *completeResponse;
	clientLength=sizeof(client);
	char *response=(char*) calloc(MAX_SEGMENT_SIZE,sizeof(char));



			while(1)
				{

				int first=0;
				int last=0;
				bzero(requestBuffer,MAX_SEGMENT_SIZE);


				n=recvfrom(socketFiledescriptor,requestBuffer,MAX_SEGMENT_SIZE,0,(struct sockaddr *)&client,&clientLength);
				printf("bytes length recieved %d /n",n);


				//keep reading open for UDP.
				printf("recieved from client");
				//get request details
				httpRequestObject=HttpRequest(requestBuffer,httpRequestObject);

				//printf("filepath is %s",httpRequestObject.url+1);

				completeResponse=createHttpResponse(httpRequestObject,socketFiledescriptor,client);
				//filePointer.close();
				//filePointer.seekg(0,ios::beg);
				int completeLength=strlen(completeResponse);


				int count=0;
				//int segmentsCount=(lengthOfFile+76)/MAX_SEGMENT_SIZE+1;
				int segmentsCount=completeLength/MAX_SEGMENT_SIZE+1;
				cout<<"segments count is"<<segmentsCount<<endl;
				while(count!=segmentsCount){
						if(count+1!=segmentsCount){
						last=last+MAX_SEGMENT_SIZE;

						//bzero(completeResponse,MAX_SEGMENT_SIZE);
						//bzero(contentOfFile,MAX_SEGMENT_SIZE);
						//completeResponse=createHttpResponse(&filePointer,first,last,count,lengthOfFile);

						//strcpy(completeResponse,response);
						//cout<<strlen(completeResponse)<<endl;
						memcpy(response,completeResponse+first,MAX_SEGMENT_SIZE);
						if(count==0){
						//puts(response);
						}
						count=count+1;
						n=sendto(socketFiledescriptor,response,MAX_SEGMENT_SIZE,0,(struct sockaddr *)&client,sizeof(client));
												printf("sent count %d of bytes %d \n",count,n);
												if(n<-1)
														{
																	printf("error sending to client \n");
														}
						//bzero(response,MAX_SEGMENT_SIZE);

						first=last;
						//printf("count is %d \n",count);

							}
							else{
								printf("last packet \n");
								last=completeLength;
								//completeResponse=createHttpResponse(&filePointer,first,last,count,lengthOfFile);
								cout<<(last-first)<<endl;
								//cout<<strlen(completeResponse)<<endl;
								//puts(completeResponse);
								//strcpy(completeResponse,response);
								count=count+1;
								memcpy(response,completeResponse+first,(last-first));
								//puts(response);

								//cout<<"l of response"<<strlen(response)<<endl;
								printf("count is %d \n",count);
								//first=last;
								n=sendto(socketFiledescriptor,response,(last-first),0,(struct sockaddr *)&client,sizeof(client));
								//bzero(response,MAX_SEGMENT_SIZE);
								printf("sent count %d of bytes %d \n",count,n);

								//bzero(completeResponse,completeLength);
							}
						}
				gettimeofday(&tv, &tz);
				printf("Last packet sent at %d\n",tv.tv_usec);
				printf("sent to client \n");
				printf("waiting for next request......\n");

				}
	printf("\n end main \n");
	//close(socketFiledescriptor);
	printf("exiting main \n");
	return 0;
}












