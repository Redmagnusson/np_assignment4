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

int CAP = 10000;
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
		exit(0);
	} else printf("Getaddrinfo success\n");
	
	//Create socket
	serverfd = socket(serverinfo->ai_family, serverinfo->ai_socktype, 0);
	if(serverfd < 0){
		printf("Error creating socket: %s\n", strerror(errno));
		exit(0);
	} else printf("Server socket created\n");
	
	//Set socket options
	int opt = true;
	if(setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
		printf("Error setting options: %s\n", strerror(errno));
		exit(0);
	}
	
	//Bind socket
	if(bind(serverfd, serverinfo->ai_addr, serverinfo->ai_addrlen) < 0){
		printf("Socket bind failed: %s\n", strerror(errno));
		exit(0);
	} else printf("Server socket bound\n");
	
	//Listen
	if(listen(serverfd, 10) < 0){
		printf("Listen failed: %s\n", strerror(errno));
		exit(0);
	} else printf("Server listening\n");
	
	//Fork once or for to 10 processes?
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
			close(clientfd);
			continue;
		}
		//Add loop for double \n ?
		printf("PID: %d, received message: %s\n", my_pid, client_message);
	
		//Process Message
		char* copy = (char*)malloc(CAP); strcpy(copy, client_message); //Copy message so we dont split the OG message
		char* command = strtok(copy, " /");
		
		//Check if its a GET or HEAD
		if(strcmp(command, "GET") == 0){
			//If GET, open file and return 
			printf("Command: \"GET\" recognized\n");
			
			//Get path
			//strtok(NULL, "/");
			char* path = strtok(NULL, " ");
			//Check if valid path
			int nrOf;
			for(int i = 1;i < strlen(path);i++){
				if(path[i] == '/'){
					nrOf++;
				}
			}
			printf("Path: %s contains %d\n", path, nrOf);
			if(nrOf == 3){
				//Open filepath
				printf("Valid path\n");
				FILE *filePtr;
				filePtr = fopen(path, "r");
				if(filePtr == NULL){
					printf("Failed to open file: filePtr == NULL\n");
					exit(0);
				}
				else printf("File opened\n");
				//Extract data and store it into the buffer
				fscanf(filePtr, "%s", server_message);
				
			}
			else if(nrOf == 0){
				FILE *filePtr;
				filePtr = fopen("randomFile", "r");
				if(filePtr == NULL){
					printf("Failed to open file: filePtr == NULL\n");
					exit(0);
				}
				else printf("File opened\n");
				fscanf(filePtr, "%s", server_message);
			}
			else{
				//Wrong path. Send error?
				printf("Path does not exist\n");
				continue;
			}
			
			
			
		}
		else if(strcmp(command, "HEAD") == 0){
			//If HEAD, make header?
		}
		//Wrong command. Close connection?
		else{
			printf("Invalid command: %s\n", command);
			close(clientfd);
			continue;
		}	
		//Check version?
		
		//Return Data
		printf("Server msg: %s\n", server_message);
		if(send(clientfd, &server_message, strlen(server_message), 0) < 0){
			printf("Error sending: %s\n", strerror(errno));
		} else printf("Message sent\n");
		//Kill Child?
		close(clientfd);
  }
  printf("Done.\n");
  return(0);


  
}
