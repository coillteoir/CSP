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

static const int MAXPENDING = 5; 

int main(int argc, char *argv[]) {
int numBytes = 0, char_in, count = 0, size = 0, recvLoop = 0, totalBytes =0;
char recvbuffer[BUFSIZE], sendbuffer[BUFSIZE], path[200] ={'.'}, discard1[50], discard2[50]; 

struct stat st;
FILE * hFile; //declare file pointer
	
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
		memset (sendbuffer, '\0', sizeof(sendbuffer));
		memset (recvbuffer, '\0', sizeof(recvbuffer));
		
		while (recvLoop  > 0) 
		{  

			numBytes = recv(clntSock, (recvbuffer+totalBytes), 1, 0); //note the one byte limitation -  argument three	
			totalBytes += numBytes; //updating the off-set

			if((totalBytes >= (BUFSIZE-2)) || (strstr(recvbuffer,"\r\n\r\n")>0))
				recvLoop = 0;  //Testing to ensure recv() does not over-write the recvbuffer OR looking for \r\n\r\n
					
		}


    if (numBytes < 0)
	DieWithSystemMessage("recv() failed");

	sscanf(recvbuffer, "%s %s %s", discard1, (path+1), discard2);
	printf("\npath contains: %s\n", path);


	if(strcmp(path, "./favicon.ico") == 0)
	{
		printf("\n\nFound and ignored favicon.ico\n\n");		
	}
	else
	{
		if(strcmp(path, "./") == 0)
		{
			strcpy(path, "./index.html");		//reset the path1 buffer back to "."
		}

		hFile = fopen(path, "r");  	//open the requested file			

		if (hFile == NULL) 	 		//if requested file does not exist
		{
			strcpy(path, "./error.html");		//reset the path1 buffer back to "."

			hFile = fopen(path, "r");  	//open the error file

			stat(path, &st);
			size = st.st_size;

			printf("\nERROR.HTML File size is: %d\n", size);
			snprintf(sendbuffer, sizeof(sendbuffer), "HTTP/1.1 404 File Not Found\r\nContent-Length: %d\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n", size); 
		}
		else
		{

			stat(path, &st);
			size = st.st_size;

			printf("\nRequested file size is: %d\n", size);
			snprintf(sendbuffer, sizeof(sendbuffer), "HTTP/1.1 200 Okay\r\nContent-Length: %d\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n", size); 
		}
	 
		ssize_t numBytesSent = send(clntSock, sendbuffer, strlen(sendbuffer), 0);
		if (numBytesSent < 0)
			DieWithSystemMessage("send() failed");

		strcpy(sendbuffer,"");  				//empty the outgoing buffer

		while((char_in = fgetc(hFile))!= EOF)   //reading one char at-a-time from the open file
		{
			sendbuffer[count] = char_in;	//storing the char in the outgoing buffer
			count++;				//increment the buffer index ready for next character
		}

		numBytesSent = send(clntSock, sendbuffer, count, 0);
		if (numBytesSent < 0)
			DieWithSystemMessage("send() failed");

		size = 0;
		count = 0;	
		fclose(hFile);			//close the file
		strcpy(path, ".");
	}//reset the path1 buffer back to "."
	close(clntSock); // Close client socket
  } //END FOR LOOP
  // NOT REACHED
}
