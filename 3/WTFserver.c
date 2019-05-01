#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
struct node{
	pthread_mutex_t lock;
	char * name;
	struct node * next;
};
pthread_mutex_t masterLock;
struct node * keychain;

int create(char * project){
	pthread_mutex_lock(&masterLock);
	struct stat st = {0};
	if (stat("./projects", &st) == -1) {
		mkdir("./projects", 0700);
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
	if(pthread_mutex_lock(&(keychain->lock)) == 0){
		printf("locked\n");
	}
	pthread_mutex_unlock(&(keychain->lock));
	pthread_mutex_unlock(&masterLock);
	return 0;
}

int main(int argc, char** argv){
	struct sockaddr_in address;
	int socketfd = socket(AF_INET,SOCK_STREAM,0);
	
	if (socketfd <= 0){
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

	if (bind(socketfd,(struct sockaddr*)&address,sizeof(address)) < 0){
		printf("Bind failed\n");
		return 0;
	}
	
	if(listen(socketfd, 20) < 0){
		printf("Can't listen\n");
		return 0;
	}
	
	while(1){
		struct sockaddr_in incoming;
		memset(&incoming,0,sizeof(incoming));
		socklen_t incomSize = sizeof(incoming);
		int newfd = accept(socketfd,(struct sockaddr*)&incoming,&incomSize);
		/*char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&(incoming.sin_addr),str,INET_ADDRSTRLEN);
		printf("%s,",str);*/
		char buffer[2000];
		read(newfd, buffer, 2000);
		if(strcmp(buffer, "create") == 0){
			create(buffer);
			send(newfd, "haha", 4, 0);
		}else if(strcmp(buffer, "destroy") == 0){
			//destroy(buffer);
		}
		sleep(2);
		send(newfd, "poop\0", 5, 0);	
	}
	return 0;
}
