#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>


#define TRACKER_PORT 8888
#define PORT 8001
#define BUFFER_FILE_SIZE 1024*1024
const char TRACKER_IP[]="127.0.0.1";
const char IP[]="127.0.0.1";

void error(const char *msg){
	perror(msg);
	exit(1);
}

void connectLeetcher(){
	int sockfd, newsockfd,portno,n;
	char buffer[BUFFER_FILE_SIZE];

	struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;

	sockfd= socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd<0){
		error("Error opening Socket");
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = PORT;

	serv_addr.sin_family= AF_INET;
	serv_addr.sin_addr.s_addr= INADDR_ANY;
	serv_addr.sin_port= htons(portno);

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))<0){
		error("Binding Failed");
	}
	listen(sockfd,5);
	clilen=sizeof(cli_addr);
	
	newsockfd=accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);
	
	if(newsockfd <0) error("Error on Accept");
	bzero(buffer,BUFFER_FILE_SIZE);
	read(newsockfd,&buffer,BUFFER_FILE_SIZE);
	printf("Start sending file\n");
	FILE* fptr;
	fptr=fopen("part1.txt","r");
	fgets(buffer,BUFFER_FILE_SIZE,fptr);
	write(newsockfd, &buffer,BUFFER_FILE_SIZE);
	bzero(buffer,BUFFER_FILE_SIZE);
	printf("Complete sending file\n");
	close(newsockfd);
	close(sockfd);
}


int main(int argc, char* argv[]){
	int sockfd, portno, n;
	char portStr[4];
	sprintf(portStr,"%d",PORT);
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	char buffer[256];
	portno = TRACKER_PORT;
	sockfd = socket(AF_INET, SOCK_STREAM,0);
	if (sockfd<0) error("Error opening socket");
	server = gethostbyname(TRACKER_IP);
	if(server == NULL) fprintf(stderr, "Error, no such host");
	bzero((char*) &serv_addr,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port=htons(portno);
	if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))<0)
		error("Connection failed");
	
	bzero(buffer,255);
	strcpy(buffer,"s,textfile.txt,1,3,");
	strncat(buffer,&IP,sizeof(IP));
	strcat(buffer,",");
	strncat(buffer,&portStr,sizeof(portStr));
	write(sockfd, &buffer, sizeof(buffer));
	
	connectLeetcher();
	close(sockfd);

	portno = TRACKER_PORT;
	sockfd = socket(AF_INET, SOCK_STREAM,0);
	if (sockfd<0) error("Error opening socket");
	server = gethostbyname(TRACKER_IP);
	if(server == NULL) fprintf(stderr, "Error, no such host");
	bzero((char*) &serv_addr,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port=htons(portno);
	if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))<0)
		error("Connection failed");
	bzero(buffer,255);
	strcpy(buffer,"d,textfile.txt,1,3,127.0.0.1,8001");
	write(sockfd, &buffer, sizeof(buffer));
	close(sockfd);
	return 0;
}
