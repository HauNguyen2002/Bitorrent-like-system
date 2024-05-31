

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <pthread.h>


#define TRACKER_PORT 8888
#define MAX_PIECES 3
#define BUFFER_FILE_SIZE 500*1024

const char TRACKER_IP[]="127.0.0.1";

pthread_mutex_t mutex;

void error(const char *msg){
	perror(msg);
	exit(1);
}


typedef struct{
	int port;
	int part;
	char filename[100];
	char piecename[100];
}PieceInfo;


void findNewSeeder(char* filename, int part){
	int sockfd, portno, n;
	int portList[MAX_PIECES];
	int partOrder[MAX_PIECES];
	char sdList[255];
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	char buffer[256];

	for (int i=0;i<MAX_PIECES;i++){
		portList[i]=partOrder[i]=0;
	}
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
	strcpy(buffer,filename);
	write(sockfd, &buffer, sizeof(buffer));
	read(sockfd, &buffer, 255);
	if(buffer[0]=='\0'){
		printf("The file is not available\n");
		return 1;
	}
	strcpy(sdList,buffer);
	bzero(buffer,255);
	for(int i=0;sdList[i]!='\0';i++){
		if(i==0 || sdList[i-1]=='\n'){
			int part=(int)(sdList[i])-48;;
			partOrder[part-1]=part;
			portList[part-1]=-1;
		}
		if(sdList[i+1]=='\n' || sdList[i+1]=='\0'){
			char portStr[5];
			int idx=0;
			for(int p=i-3;p<=i;p++){
				portStr[idx]=sdList[p];
				idx++;
			}
			int port;
			sscanf(portStr,"%d", &port);
			for(int i=0;i<MAX_PIECES;i++){
				if(portList[i]==-1){
					portList[i]=port;
					break;
				}
			}
			bzero(portStr,5);
		}
	}
	close(sockfd);
}

void* connectSeeder(void* arg){
	PieceInfo* piece=(PieceInfo*)arg;
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[BUFFER_FILE_SIZE];
	portno = piece->port;
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
	bzero(buffer,BUFFER_FILE_SIZE);
	strcpy(buffer,"hello");
	write(sockfd,&buffer,sizeof(buffer));
	bzero(buffer,BUFFER_FILE_SIZE);
	read(sockfd, &buffer, BUFFER_FILE_SIZE);
	char partStr[4];
	char piecename[100];
	for(int i=0;piece->filename[i]!='.';i++){
		piecename[i]=piece->filename[i];
	}
	strcat(piecename,"_");
	sprintf(partStr,"%d",piece->part);
	strncat(piecename,&partStr,1);
	strcat(piecename,".txt");
	strcpy(piece->piecename,piecename);
	FILE* fptr;
	fptr=fopen(piecename,"w");
	pthread_mutex_lock(&mutex);
	fprintf(fptr,"%s",buffer);
	pthread_mutex_unlock(&mutex);
	sleep(1);
	bzero(buffer,BUFFER_FILE_SIZE);
	fclose(fptr); 
	close(sockfd);
}
int main(int argc, char* argv[]){
	if(argc<2){
		printf("Invalid Filename\n");
		return -1;
	}
	char filename[100];
	strcpy(filename,argv[1]);
	int sockfd, portno, n;
	int portList[MAX_PIECES];
	int partOrder[MAX_PIECES];
	char sdList[255];
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	char buffer[256];

	for (int i=0;i<MAX_PIECES;i++){
		portList[i]=partOrder[i]=0;
	}
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
	strcpy(buffer,filename);
	write(sockfd, &buffer, sizeof(buffer));
	read(sockfd, &buffer, 255);
	if(buffer[0]=='\0'){
		printf("The file is not available\n");
		return 1;
	}
	strcpy(sdList,buffer);
	bzero(buffer,255);
	for(int i=0;sdList[i]!='\0';i++){
		if(i==0 || sdList[i-1]=='\n'){
			int part=(int)(sdList[i])-48;;
			partOrder[part-1]=part;
			portList[part-1]=-1;
		}
		if(sdList[i+1]=='\n' || sdList[i+1]=='\0'){
			char portStr[5];
			int idx=0;
			for(int p=i-3;p<=i;p++){
				portStr[idx]=sdList[p];
				idx++;
			}
			int port;
			sscanf(portStr,"%d", &port);
			for(int i=0;i<MAX_PIECES;i++){
				if(portList[i]==-1){
					portList[i]=port;
					break;
				}
			}
			bzero(portStr,5);
		}
	}

	close(sockfd);
	pthread_t th[MAX_PIECES];
	pthread_mutex_init(&mutex,NULL);
	printf("Connected to Seeder\n");
	printf("Start Downloading pieces\n");
	PieceInfo piece[MAX_PIECES];
	for(int i=0;i<MAX_PIECES;i++){
		if(portList[i]!=0){
			piece[i].port=portList[i];
			piece[i].part=partOrder[i];
			strcpy(piece[i].filename,filename);
			pthread_create(&th[i],NULL,connectSeeder,(void*)&piece[i]);
		}
	}
	for(int i=0;i<MAX_PIECES;i++){
		if(portList[i]!=0) pthread_join(th[i],NULL);
	}
	pthread_mutex_destroy(&mutex);

	FILE* output;
	if(access(filename, F_OK)==0) remove(filename);
	output=fopen(filename,"w");
	output=fopen(filename,"a");
	for(int i=0;i<MAX_PIECES;i++){
		if(portList[i]!=0){
			FILE* pieceFile;
			char pieceData[BUFFER_FILE_SIZE];
			pieceFile=fopen(piece[i].piecename,"r");
			fgets(pieceData,BUFFER_FILE_SIZE,pieceFile);
			fprintf(output,"%s",pieceData);
			fclose(pieceFile);
			remove(piece[i].piecename);
		}
	}
	fclose(output);


	printf("Download Successfully\n");
	return 0;
}
