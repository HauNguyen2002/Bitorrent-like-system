#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8888
#define MAX_CONNECTION 10


typedef struct {
    int sockfd;
    int newsockfd;
    char buffer[256];
    socklen_t clilen;
    struct sockaddr_in cli_addr;

}Peer;

void error(const char *msg){
    perror(msg);
    exit(1);
}

char* findFile(FILE* fptr, char filename[]){
    int countDeli=0;
    char* result = (char*)malloc(255 * sizeof(char));
    char line[100];
    char temp[100];
    char ch;
    bzero(result,255);
    bzero(line,100);
    bzero(temp,100);
    while(fgets(line,100,fptr)){
        for(int i=0;line[i]!=',';i++){
            strncat(temp,&line[i],1);
        }
        if(strcmp(temp,filename)==0){
            for(int i=strlen(temp)+1;line[i]!='\0';i++){
                strncat(result,&line[i],1);
            }
        }
        bzero(temp,100);
    }
    return result;
}

void* connectThread(void* arg){
    Peer* pr=(Peer*)arg;
    while(1){
        listen(pr->sockfd, MAX_CONNECTION);
        pr->clilen=sizeof(pr->cli_addr);

        pr->newsockfd=accept(pr->sockfd, (struct sockaddr*) &(pr->cli_addr), &(pr->clilen));

        if(pr->newsockfd <0) error("Error on Accept");
        bzero(pr->buffer,255);
        read(pr->newsockfd, &(pr->buffer), 255);
        FILE *fptr;
        if(pr->buffer[0]=='s'){
            printf("Seeder\n");
            if(access("Slist.txt", F_OK)==0){
                fptr=fopen("Slist.txt","a");
            }
            else{
                fptr=fopen("Slist.txt","w");
            }
            char data[100];
            bzero(data,100);
            for(int i=2;pr->buffer[i]!='\0';i++) data[i-2]=pr->buffer[i];
                fprintf(fptr,"%s\n",data);
            fclose(fptr);
        }
        else if(pr->buffer[0]=='d'){
            printf("Disconnect Seeder\n");
            FILE* tempfptr;
            tempfptr=fopen("temp.txt","w");
            fptr=fopen("Slist.txt","r");
            char data[100];
            char line[100];
            bzero(data,100);
            bzero(line,100);
            for(int i=2;pr->buffer[i]!='\0';i++) data[i-2]=pr->buffer[i];
            while(fgets(line,100,fptr)){
                if(strcmp(data,line)!=-10 && line!="\n") fprintf(tempfptr,"%s",line);
            }
            fclose(fptr);
            fclose(tempfptr);
            remove("Slist.txt");
            rename("temp.txt","Slist.txt");
        }
        else{
            printf("Leetcher\n");
            if(access("Slist.txt", F_OK)==0){
                fptr=fopen("Slist.txt","r");
                strcpy(pr->buffer,findFile(fptr,pr->buffer));

            }
            else{
                strcpy(pr->buffer,"The file is unavailable!");
            }
            write(pr->newsockfd,&(pr->buffer),sizeof(pr->buffer));
        }
        bzero(pr->buffer,256);
    }
}
void* print(){
    // while(1){
    //     printf("Hello world\n");
    //     sleep(1);
    // }
}


int main(int argc, char* argv[]){

    int sockfd, newsockfd,portno,n;
    char buffer[255];

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

    Peer pr;
    pr.sockfd=sockfd;
    pr.newsockfd=newsockfd;
    pr.clilen=clilen;
    pr.cli_addr=cli_addr;

    pthread_t thread[10];
    if (pthread_create(&thread[0], NULL, connectThread, (void *)&pr)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }
    if (pthread_create(&thread[1], NULL, print, NULL)) {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    for(int i=0;i<2;i++){
        pthread_join(thread[i], NULL);
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}


