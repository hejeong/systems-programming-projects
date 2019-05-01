#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
struct node{
	pthread_mutex_t lock;
	char * name;
	struct node * next;
};
pthread_mutex_t masterLock;
struct node * keychain;

int create(char * project, int sock){
	DIR * dir;
	if(pthread_mutex_lock(&masterLock) != 0){
		printf("broken thread lock\n");
		return 0;
	}
	struct stat st1 = {0};
	if (stat("./projects", &st1) == -1) {
		mkdir("./projects", 0700);
	}
	
	dir = opendir("./projects");
	
	
	char * projDir = malloc(12 + strlen(project));
	strcpy(projDir, "./projects/\0");
	strcat(projDir, project);
	
	struct stat st2 = {0};
	if (stat(projDir, &st2) == -1) {
		mkdir(projDir, 0700);
	}else{
		printf("project already exists\n");
	}
	
	
	if(keychain == NULL){
		keychain = malloc(sizeof(struct node));
		keychain->name = malloc(strlen(project));
		strcpy(keychain->name, project);
		keychain->next = NULL;
		if(pthread_mutex_init(&(keychain->lock), NULL) == 0){
			printf("works\n");
		}
	}
	char * manifest = malloc(12 + strlen(projDir));
	strcpy(manifest, projDir);
	strcat(manifest, "/.Manifest");
	int fd = open(manifest, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG);
	if(fd == -1){
		printf("unable to write\n");
		pthread_mutex_unlock(&masterLock);
		return 0;
	}
	write(fd, "1\n", 2);
	
	struct stat fileStat;
	char fileSize[1000];
	fstat(fd, &fileStat);
	sprintf(fileSize, "%d", fileStat.st_size);
	send(sock, fileSize, strlen(fileSize), 0);
	printf("sent %s\n", fileSize);
	sleep(1);
	int sent;
	int remaining = fileStat.st_size;
	close(fd);
	fd = open(manifest, O_RDONLY);
	while( (remaining > 0) && ((sent = sendfile(sock, fd, NULL, 1000)) > 0) ){
		printf("sent %d\n", sent);
		remaining = remaining - sent;
	}
	
	
	printf("haha\n");
	
	
	close(fd);
	
	pthread_mutex_unlock(&masterLock);
	return 0;
}

int main(int argc, char** argv){
	struct sockaddr_in address;
	int list = socket(AF_INET,SOCK_STREAM,0);
	
	if (list <= 0){
		printf("Can't open socket\n");
		return 0;
	}	
	if (argc != 2){
		printf("Need port number\n");
		return 0;
	}
	int port = atoi(argv[1]);
	if(port <= 0){
		printf("Invalid port\n");
		return 0;
	}
	memset(&address,0,sizeof(address));

	address.sin_family =AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(list,(struct sockaddr*)&address,sizeof(address)) < 0){
		printf("Bind failed\n");
		return 0;
	}
	
	if(listen(list, 20) < 0){
		printf("Can't listen\n");
		return 0;
	}
	
	while(1){
		struct sockaddr_in incoming;
		memset(&incoming,0,sizeof(incoming));
		socklen_t incomSize = sizeof(incoming);
		int comm = accept(list,(struct sockaddr*)&incoming,&incomSize);
		/*char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&(incoming.sin_addr),str,INET_ADDRSTRLEN);
		printf("%s,",str);*/
		printf("client accepted\n");
		char buffer[2000];
		recv(comm, buffer, 2000, 0);
		if(strcmp(buffer, "create") == 0){
			create(buffer, comm);
		}else if(strcmp(buffer, "destroy") == 0){
			//destroy(buffer);
		}
	}
	return 0;
}
