#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Practical.h"
#include <unistd.h>
#include <sys/stat.h>
#define HOME_PAGE "HTTP/1.0 200 File Found\r\nContent-Length: 131\r\nConnection: close\r\nServer: httpserver\r\n\r\n<HTML><HEAD><TITLE>File  Found</TITLE></HEAD><BODY><h2>FILE Found</h2><hr><p>Your requested INDEX FILE was found.</p></BODY></HTML>"
#define ERROR_PAGE "HTTP/1.0 404 File Not Found\r\nContent-Length: 142\r\nConnection: close\r\n\r\n<HTML><HEAD><TITLE>File NOT Found</TITLE></HEAD><BODY><h2>FILE NOT Found</h2><hr><p>Your requested INDEX FILE was NOT found.</p></BODY></HTML>"


static const int MAXPENDING = 2;

int main(int argc, char *argv[]) {

	int recvLoop = 0, numBytes = 0, totalBytes =0;
	char recvbuffer[BUFSIZE], sendbuffer[BUFSIZE], uri[200] ={""}, discard1[50], discard2[50];
	//Increased BUFSIZE to 1024 in the Practical.h file
	
 	if (argc != 2) 
    		DieWithUserMessage("Parameter(s)", "<Server Port>");

  	in_port_t servPort = atoi(argv[1]); 

	int servSock; 

  	if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    		DieWithSystemMessage("socket() failed");

  	struct sockaddr_in servAddr;
  	memset(&servAddr, 0, sizeof(servAddr));       
  	servAddr.sin_family = AF_INET;                
  	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  	servAddr.sin_port = htons(servPort);
  
	if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
    		DieWithSystemMessage("bind() failed");

  	if (listen(servSock, MAXPENDING) < 0)
   	 	DieWithSystemMessage("listen() failed");

	for (;;) { 

    	int clntSock = accept(servSock, (struct sockaddr *) NULL, NULL);

    	if (clntSock < 0)
      		DieWithSystemMessage("accept() failed");

		recvLoop = 1;
		totalBytes = 0;
		memset (uri, '\0', sizeof(uri));
		memset (sendbuffer, '\0', sizeof(sendbuffer));
		memset (recvbuffer, '\0', sizeof(recvbuffer));
		
		while (recvLoop  > 0) 
		{  

			numBytes = recv(clntSock, (recvbuffer+totalBytes), 1, 0); //note the one byte limitation -  argument three	
			totalBytes += numBytes; //updating the off-set

			if((totalBytes >= (BUFSIZE-2)) || (strstr(recvbuffer,"\r\n\r\n")>0))
				recvLoop = 0;  //do not exceed the size of the recvbuffer OR looking for \r\n\r\n
					
		}
		if (numBytes < 0)
				DieWithSystemMessage("recv() failed");

		sscanf(recvbuffer, "%s %s %s\r\n", discard1, uri, discard2);  //parsing the incoming stream

		if(strcmp(uri, "/favicon.ico") == 0)
		{
			printf("\n\nFound and ignored favicon.ico\n\n");
			close(clntSock);

		}
		else{
			
		recvbuffer[totalBytes] = '\0'; 
		fputs(recvbuffer, stdout);

		if(strcmp(uri, "/index.html") == 0)
		{
		snprintf(sendbuffer, sizeof(sendbuffer), HOME_PAGE);
		}
		else
		{
		snprintf(sendbuffer, sizeof(sendbuffer), ERROR_PAGE);
		}

		ssize_t numBytesSent = send(clntSock, sendbuffer, strlen(sendbuffer), 0);

			if (numBytesSent < 0)
				DieWithSystemMessage("send() failed");
		
		close(clntSock);
		}
	  }
  
}
