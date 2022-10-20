#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

/* You will to add includes here */
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <regex.h>
#include <stdbool.h>
#include <sys/select.h>
#include <sys/time.h>

int CAP = 2000;
using namespace std;

int main(int argc, char *argv[]){
  
	//Variables
	char* splits[CAP];
  char* p = strtok(argv[1], ":");
  int delimCounter = 0;
  char *Desthost;
  char *Destport;
  int port;
	int serverfd;
	struct sockaddr_in client;
	
  
  //Get argv
  while(p != NULL){
  	//Look for the amount of ":" in argv to determine if ipv4 or ipv6
  	splits[delimCounter++] = p;
  	p = strtok(NULL, ":");
  }
  
  Destport = splits[--delimCounter];
  Desthost = splits[0];
  
  for(int i = 1;i<delimCounter;i++){
  	
  	sprintf(Desthost, "%s:%s",Desthost, splits[i]);
  }

  port=atoi(Destport);
  printf("Host %s and port %d.\n",Desthost,port);
	
	//Getaddrinfo
	struct addrinfo hints, *serverinfo = 0;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	if(getaddrinfo(Desthost, Destport, &hints, &serverinfo) < 0){
		printf("Getaddrinfo error: %s\n", strerror(errno)); 
	} else printf("Getaddrinfo success\n");
	
	//Create socket
	serverfd = socket(serverinfo->ai_family, serverinfo->ai_socktype, 0);
	if(serverfd < 0){
		printf("Error creating socket: %s\n", strerror(errno));
	} else printf("Server socket created\n");
	
	//Bind socket
	if(bind(serverfd, serverinfo->ai_addr, serverinfo->ai_addrlen) < 0){
		printf("Socket bind failed: %s\n", strerror(errno));
	} else printf("Server socket bound\n");
	
	//Listen
	if(listen(serverfd, 10) < 0){
		printf("Listen failed: %s\n", strerror(errno));
	} else printf("Server listening\n");
	

	//How many forks am i supposed to have?
	//Fork process (we now have 2 processes)
	fork();
	pid_t my_pid = getpid();
	printf("Forked. PID: %d\n", my_pid);

	while(true){

		//Accept Client
		char server_message[CAP], client_message[CAP];
		int bytesRecv;
		struct sockaddr_in clientinfo;
		int len;
		int clientfd;
		clientfd = accept(serverfd, (struct sockaddr*)&clientinfo, (socklen_t*)&len);
		if(clientfd < 0){
			printf("Accept error: %s\n", strerror(errno));
		} printf("Accepted client\n");
	
		//Read Data
		bytesRecv = recv(clientfd, client_message, CAP, NULL);
		if(bytesRecv == 0){
			//Client dropped
		}
		//Add loop for double \n ?
		printf("PID: %d, received message: %s\n", my_pid, client_message);
	
		//Process Message
		char* copy; strcpy(copy, client_message); //Copy message so we dont split the OG message
		char* command = strtok(copy, " ");
		
		//Check if its a GET or HEAD
		if(strcmp(command, "GET") == 0){
			//If GET, open file and return 
		}
		else if(strcmp(command, "HEAD") == 0){
			//If HEAD, ????
		}
		//Wrong command. Close connection?
		else printf("Invalid command: %s\n", command);
		
		//Check version?
	
		//Return Data

		//Kill Child?
		return(0);
  }
  printf("Done.\n");
  return(0);


  
}
