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
#include <pthread.h>
struct FDdetails
{
  struct addrinfo *address;
  struct sockaddr_in port;
  int clientSock;
  int uid;
};
int CAP = 2000;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
using namespace std;
ssize_t sendall(int s, char *buf, int *len)
{

		printf("entered Sendall\n");
    ssize_t total = 0;        // how many bytes we've sent
    ssize_t bytesleft = *len; // how many we have left to send
    ssize_t n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) {
         printf("Error Sending: %s\n", strerror(errno));
         break;
        }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here
    //sleep(5);
		//close(s);
    //return n==-1?-1:0; // return -1 onm failure, 0 on success
    return total;
} 
void* handleMessage(void* arg){


		//pthread_mutex_lock(&lock);
		
		//int clientfd = *(int*) client;
		//int clientfd = *((int*)client);
		FDdetails *currentClient = (FDdetails *)arg;
		int clientfd = currentClient->clientSock;
		//printf("Thread entered function\n");
		char client_message[CAP];
		memset(client_message, 0, CAP);
		int bytesRecv = 0;

		//Read Client Data
		//printf("Reading Data\n");
		bytesRecv = recv(clientfd, client_message, CAP, NULL);
		if(bytesRecv < 0){
			//Client dropped
			printf("Failed to recv, dropping client: %s\n", strerror(errno));
			//return;
		} //else printf("%s\n", client_message);
		
		//printf("Processing\n");
		//Process Message
		char* command = strtok(client_message, "/");
		char* path = strtok(NULL, " ");
		char* http = strtok(NULL, "\n");
		//printf("%s%s%s\n", command, path, http);
		//printf("Checking command\n");
		http[strlen(http)-1] = NULL;
		FILE *filePtr;
		int sizeOfFile = 0;
		
		//Check if its a GET or HEAD
		if(strcmp(command, "GET ") == 0){
			//If GET, open file and return 
			//printf("Command: \"GET\" recognized\n");
			
			//Check if valid path
			int nrOf = 0;
			for(int i = 1;i < strlen(path);i++){
				if(path[i] == '/'){
					nrOf++;
				}
			}
			//printf("Path: %s contains %d\n", path, nrOf);
			if(nrOf < 4){
				//Open filepath
				//printf("Valid path: %s\n", path);
				filePtr = fopen(path, "r");
	
				if(filePtr == NULL){
					//Incorrect path, adding error message
					printf("Path is NULL\n");
					char message[26];
					sprintf(message, "%s 404 Not Found\r\n\r\n", http);
					if(write(clientfd, &message, sizeof(message)-2) < 0){
						printf("Error sending: %s\n", strerror(errno));
					} //else printf("Message sent: %s\n");

				}
				else{
					//printf("File opened\n");
					//Send OK
					char message[20];
					sprintf(message, "%s 200 OK\r\n\r\n", http);
					//printf("%d, %d\n", sizeof(message), strlen(message));
					if(write(clientfd, &message, sizeof(message)) < 0){
						printf("Error sending: %s\n", strerror(errno));
					} //else printf("Message sent: %s\n");
				
					//Get filesize
					fseek(filePtr, 0, SEEK_END);
					sizeOfFile = ftell(filePtr);
					
					//Send data
					fseek(filePtr, 0, SEEK_SET);
					char buffer3[sizeOfFile];
					memset(buffer3, 0, sizeOfFile);
							
					fread(buffer3, sizeof(char), sizeof(buffer3), filePtr);
					
					//pthread_mutex_lock(&lock);
					ssize_t test = sendall(clientfd, buffer3, &sizeOfFile);
					//pthread_mutex_unlock(&lock);
					printf("Buffer3 size: %d\nSent bytes: %d\n", sizeOfFile, test);
					
				}
				fclose(filePtr);
			}
			else{
				//Wrong path. Adding error message
				printf("Path does not exist\n");
				char message[26];
				sprintf(message, "%s 404 Not Found\r\n\r\n", http);
				if(write(clientfd, &message, sizeof(message)-2) < 0){
					printf("Error sending: %s\n", strerror(errno));
				} else printf("Message sent: %s\n");
			}
			
		}
		else if(strcmp(command, "HEAD ") == 0){
			//If HEAD, make header?
			char message[20];
			sprintf(message, "%s 200 OK\r\n\r\n", http);
			if(write(clientfd, &message, sizeof(message)) < 0){
				printf("Error sending: %s\n", strerror(errno));
			} else printf("Message sent: %s\n");
		}
		
		//Wrong command. Close connection?
		else{
			printf("Invalid command: %s\n", command);
			//Add error 400?
			char message[26];
			sprintf(message, "%s 404 Not Found\r\n\r\n", http);
			if(write(clientfd, &message, sizeof(message)-2) < 0){
				printf("Error sending: %s\n", strerror(errno));
			} else printf("Message sent: %s\n");
		}	
	
		
		//printf("Closing\n");

		if(close(clientfd) < 0){
			printf("Error closing: %s\n", strerror(errno));
		} else printf("FD closed\n");
		//sleep(5);
		//pthread_mutex_unlock(&lock);
		if(pthread_detach(pthread_self()) < 0){
			printf("Error detaching: %s\n", strerror(errno));
		} else printf("Thread detached\n");
		//return NULL;
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
	pthread_t threadID;
	
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
	
	//Fork process (we now have 2 processes)
	//fork();
	//pid_t my_pid = getpid();
	//printf("Forked. PID: %d\n", my_pid);

	//signal(SIGPIPE, SIG_IGN);
	while(true){
	
	
	//char buffer2[11000];
		//Accept Client
		char server_message[CAP], client_message[CAP];
		int bytesRecv;
		struct sockaddr_in clientinfo;
		socklen_t len = sizeof(clientinfo);
		int clientfd;
		clientfd = accept(serverfd, (struct sockaddr*)&clientinfo, &len);
		if(clientfd < 0){
			printf("Accept error: %s\n", strerror(errno));
		} //printf("Accepted client\n");


		FDdetails *currentClient = (FDdetails *)malloc(sizeof(FDdetails));
    memset(currentClient, 0, sizeof(FDdetails));
    currentClient->clientSock = clientfd;
		pthread_create(&threadID, NULL, &handleMessage, (void*) currentClient);
  }
  printf("Done.\n");
  return(0);


  
}
