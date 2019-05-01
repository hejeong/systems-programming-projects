#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stddef.h>

int create(char * name, int sock){
	
	send(sock, "create", 6, 0);
	sleep(1);
	send(sock, name, strlen(name), 0);
	char buffer[1000];
	recv(sock, buffer, 1000, 0);
	int size = atoi(buffer);
	
	if(size == 0){
		printf("project already exists on the server\n");
		return 0;
	}
	
	char * projDir = malloc(3 + strlen(name));
	strcpy(projDir, "./\0");
	strcat(projDir, name);
	
	struct stat st2 = {0};
	if (stat(projDir, &st2) == -1) {
		mkdir(projDir, 0700);
	}else{
		printf("project created successfully in server, however project already exists in local repository. Please use the checkout command to get the new project\n");
		return 0;
	}
	
	char * manifest = malloc(12 + strlen(projDir));
	strcpy(manifest, projDir);
	strcat(manifest, "/.Manifest");
	int fd = open(manifest, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
	
	int remaining = size;
	int written;
	while( (remaining > 0) && ((written = recv(sock, buffer, 1000, 0)) > 0) ){
		printf("haha\n");
		write(fd, buffer, written);
		remaining = remaining - written;
		printf("%d remaining\n", remaining);
	}
	close(fd);
	return 0;
}

int main(int argc, char ** argv){
	int port = atoi(argv[1]);
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	//addr.sin_addr.s_addr = inet_addr("1.1.1.1.1");

	int socketfd = socket(AF_INET,SOCK_STREAM, 0);
	if (socketfd <= 0){
		printf("ERROR: Failed to open socket.\n");
	}

	if (connect(socketfd, (struct sockaddr *) &address, sizeof(address)) < 0)
	{
		printf("Error: Connection failed.\n");
		return -1;
	}
	printf("connected\n");
	create("p1", socketfd);
	
	return 0;
}