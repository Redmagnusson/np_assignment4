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
#include <sys/wait.h>

int CAP = 2000;
using namespace std;
void handleMessage(int* clientfd){

		//printf("Fork entered function\n");
		char client_message[CAP];
		memset(client_message, 0, CAP);
		char buffer2[11000]; // CHANGE THIS TO DYNAMIC
		int bytesRecv = 0;
		char msg[100];
		memset(msg, 0, 100);

		//Read Data
		bytesRecv = recv(*clientfd, client_message, CAP, NULL);
		if(bytesRecv == 0){
			//Client dropped
			printf("Failed to recv, dropping client: %s\n", strerror(errno));
			//close(*clientfd);
			return;
		}
		printf("Fork msg: %s\n", client_message);
		char buffer3[20] = "HTTP/1.1 200 OK\r\n\r\n";
		if(write(*clientfd, buffer3, sizeof(buffer3)-1) < 0){
			printf("Error sending: %s\n", strerror(errno));
			//close(*clientfd);
			return;
		} else printf("Message sent\n");
		
		//Process Message
		char* command = strtok(client_message, "/");
		
		printf("Command: %s\n", command);
		//Check if its a GET or HEAD
		if(strcmp(command, "GET ") == 0){
			//If GET, open file and return 
			printf("Command: \"GET\" recognized\n");
			
			//Get path
			char* path = strtok(NULL, " ");
			//Check if valid path
			int nrOf = 0;
			for(int i = 1;i < strlen(path);i++){
				if(path[i] == '/'){
					nrOf++;
				}
			}
			printf("Path: %s contains %d\n", path, nrOf);
			if(nrOf < 4){
				//Open filepath
				printf("Valid path: %s\n", path);
				FILE *filePtr;
				filePtr = fopen(path, "r");
				if(filePtr == NULL){
					printf("Failed to open path: %s\n", strerror(errno));
					exit(0);
				}
				else printf("File opened\n");
				//Extract data and store it into the buffer
				
				fseek(filePtr, 0, SEEK_SET);
				fread(buffer2, sizeof(char), sizeof(buffer2), filePtr);
				//fscanf(filePtr, "%s", buffer2);
				fclose(filePtr);
				
			}
			else{
				//Wrong path. Send error?
				printf("Path does not exist\n");
				//close(*clientfd);
				//return 0;
			}
			
		}
				else if(strcmp(command, "HEAD") == 0){
			//If HEAD, make header?
		}
		//Wrong command. Close connection?
		else{
			printf("Invalid command: %s\n", command);
			//close(clientfd);
			//continue;
		}	
		//Check version?
		
		printf("Returning data\n");
		//Return Data
		char* finalMessage = (char*)malloc(CAP);
		char buffer[20] = "HTTP/1.1 200 OK\r\n\r\n";
		
		//sprintf(finalMessage, "HTTP/1.1 200 OK\r\n%s\r\n\r\n", buffer2);
		//printf("Strlen: %d, Sizeof: %d\n", strlen(buffer2), sizeof(buffer2));
		//if(send(clientfd, &buffer, sizeof(buffer), 0) < 0){
			//printf("Error sending: %s\n", strerror(errno));
		//} else printf("Message sent\n");
		if(write(*clientfd, &buffer2, sizeof(buffer2)) < 0){
			printf("Error sending: %s\n", strerror(errno));
		} else printf("Message sent: %s\n");

		//close(*clientfd);
		

}
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
	serverfd = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
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
	
	//Fork once or to 10 processes?
	//Fork process (we now have 2 processes)
	//fork();
	pid_t my_pid = getpid();
	//printf("Forked. PID: %d\n", my_pid);

	while(true){
	
	
	char buffer2[11000];
		//Accept Client
		char server_message[CAP], client_message[CAP];
		int bytesRecv;
		struct sockaddr_in clientinfo;
		socklen_t len = sizeof(clientinfo);
		int clientfd;
		clientfd = accept(serverfd, (struct sockaddr*)&clientinfo, &len);
		if(clientfd < 0){
			printf("Accept error: %s\n", strerror(errno));
		} printf("Accepted client\n");
	
	
		//Fork process
		pid_t forkID = fork();
		//Check if the process is a child
		if(forkID == 0){
			printf("Fork successfull\n");
			//close(serverfd);
			handleMessage(&clientfd);
			
		
		}
		//OLD CODE IT NEVER RUNS
		else if(1 == 2){
		//Read Data
		char msg[100];
		memset(msg, 0, 100);
		bytesRecv = recv(clientfd, client_message, CAP, NULL);
		if(bytesRecv == 0){
			//Client dropped
			printf("Dropping client\n");
			close(clientfd);
			continue;
		}
		printf("Client msg: %s\n", client_message);
		char buffer3[20] = "HTTP/1.0 200 OK\r\n\r\n";
				if(write(clientfd, buffer3, sizeof(buffer3)-1) < 0){
			printf("Error sending: %s\n", strerror(errno));
		} else printf("Message sent\n");
		printf("PID: %d, received message: %s\n", my_pid, client_message);
	
		//Process Message
		char* copy = (char*)malloc(CAP); strcpy(copy, client_message); //Copy message so we dont split the OG message
		char* command = strtok(copy, "/");
		
		//Check if its a GET or HEAD
		if(strcmp(command, "GET ") == 0){
			//If GET, open file and return 
			printf("Command: \"GET\" recognized\n");
			
			//Get path
			//strtok(NULL, "/");
			char* path = strtok(NULL, " ");
			//Check if valid path
			int nrOf = 0;
			for(int i = 1;i < strlen(path);i++){
				if(path[i] == '/'){
					nrOf++;
				}
			}
			printf("Path: %s contains %d\n", path, nrOf);
			if(nrOf < 4){
				//Open filepath
				printf("Valid path: %s\n", path);
				FILE *filePtr;
				filePtr = fopen(path, "r");
				if(filePtr == NULL){
					printf("Failed to open path: %s\n", strerror(errno));
					exit(0);
				}
				else printf("File opened\n");
				//Extract data and store it into the buffer
				//fscanf(filePtr, "%s", server_message);
				+
				
				fseek(filePtr, 0, SEEK_SET);
				fread(buffer2, sizeof(char), sizeof(buffer2), filePtr);
				//fscanf(filePtr, "%s", buffer2);
				fclose(filePtr);
				
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
			//close(clientfd);
			continue;
		}	
		//Check version?
		
		//Return Data
		char* finalMessage = (char*)malloc(CAP);
		char buffer[20] = "HTTP/1.0 200 OK\r\n\r\n";
		
		//sprintf(finalMessage, "HTTP/1.1 200 OK\r\n%s\r\n\r\n", server_message);
		printf("Strlen: %d, Sizeof: %d\n", strlen(buffer2), sizeof(buffer2));
		//if(send(clientfd, &buffer, sizeof(buffer), 0) < 0){
			//printf("Error sending: %s\n", strerror(errno));
		//} else printf("Message sent\n");
				if(write(clientfd, &buffer2, sizeof(buffer2)) < 0){
			printf("Error sending: %s\n", strerror(errno));
		} else printf("Message sent\n");

		close(clientfd);
		}
		close(clientfd);
		
		if(forkID == 0){
			//kill(forkID, SIGKILL);
		}
		forkID = wait(NULL);
  }
  printf("Done.\n");
  return(0);


  
}
