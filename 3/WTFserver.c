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

int regularFileOrDirectory(const char* path){
	struct stat fileStat;
	if(stat(path, &fileStat) < 0){
		return -1;
	}
	// Return 0 if it is of type "Directory"
	// Else Return 1 if it is of type "Regular File"
	if(S_ISDIR(fileStat.st_mode)){
		return 0;
	}else if(S_ISREG(fileStat.st_mode)){
		return 1;
	}
}

int create(char * projDir, int sock, char * name){
	if(pthread_mutex_lock(&masterLock) != 0){
		printf("broken thread lock\n");
		return 0;
	}
	
	struct stat st = {0};
	if (stat(projDir, &st) == -1) {
		mkdir(projDir, 0700);
	}else{
		printf("project already exists\n");
		send(sock, "0", 1, 0);
		return 0;
	}
	
	struct node * ptr = malloc(sizeof(struct node));
	ptr->name = malloc(strlen(name));
	strcpy(ptr->name, name);
	ptr->next = keychain;
	keychain = ptr;
	
	char * verDir = malloc(1 + strlen(projDir));
	strcpy(verDir, projDir);
	strcat(verDir, "/1\0");
	mkdir(verDir, 0700);
	
	char * manifest = malloc(12 + strlen(verDir));
	strcpy(manifest, verDir);
	strcat(manifest, "/.Manifest");
	int fd = open(manifest, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG);
	if(fd == -1){
		printf("unable to write\n");
		pthread_mutex_unlock(&masterLock);
		return 0;
	}
	write(fd, "0\t1\n", 2);
	
	struct stat fileStat;
	char fileSize[1000];
	fstat(fd, &fileStat);
	sprintf(fileSize, "%d", fileStat.st_size);
	send(sock, fileSize, strlen(fileSize), 0);
	printf("sent %s    length %d\n", fileSize, strlen(fileSize));
	sleep(1);
	int sent;
	int remaining = fileStat.st_size;
	close(fd);
	fd = open(manifest, O_RDONLY);
	while( (remaining > 0) && ((sent = sendfile(sock, fd, NULL, 1000)) > 0) ){
		printf("sent %d\n", sent);
		remaining = remaining - sent;
	}
	
	
	close(fd);
	
	pthread_mutex_unlock(&masterLock);
	return 0;
}

int destroy(char * currentDir){
	DIR *dir;
	struct dirent *dent;
	char buffer[1000];
	char *path;
	
	strcpy(buffer, currentDir);
	//open directory
	dir = opendir(buffer);
	//graceful error if dir can't be opened
	if(dir == NULL){
		printf("Directory %s cannot be opened\n", currentDir);
		return;
	}
	while((dent = readdir(dir)) != NULL){
		// skip over [.] and [..]
		if(!strcmp(".", dent->d_name) || !strcmp("..", dent->d_name)){
			continue;	
		}
		// dynamically allocate memory to create the path for file or directory
		path = (char*)malloc((strlen(currentDir)+strlen(dent->d_name)+2)*sizeof(char));
		strcpy(path, currentDir);
		if(currentDir[strlen(currentDir)-1] != '/'){
			strcat(path, "/");
		}
		strcat(path, dent->d_name);
		// check the file type [directory, regular file, neither]
		int typeInt = regularFileOrDirectory(path);
		char *fileType;
		if(typeInt == 0){
			fileType = "Directory";
			// append a '/' character to the end of current path, then traverse into nested directories
			char* newPath = (char*)malloc((strlen(path)+2)*sizeof(char));
			strcpy(newPath, path);
			strcat(newPath, "/");
			printf("dir %s\n", currentDir);
			destroy(newPath);
			free(newPath);
			continue;
		}else if(typeInt == 1){
			// tokenize file and add tokens to linked list, keeping track of frequency
			fileType = "Regular File";
			printf("file %s\n", currentDir);
			remove(path);
			continue;
		}else {
			fileType = "Neither";
		}
		free(path);
	}
	remove(currentDir);
	// close the current directory
	closedir(dir);
}

int sendAll(char * dir, int sock){
	FILE * fd;
	char * manifest = malloc(12 + strlen(dir));
	strcpy(manifest, dir);
	strcat(manifest, ".Manifest");
	fd = fopen(manifest, "r");
	if(fd == NULL){
		printf("Could not open Manifest\n");
		send(sock, "0", 1, 0);
		return 0;
	}
	char str[50];
	fscanf(fd, "%s\t%*s\n", str);
	send(sock, str, strlen(str), 0);
	sleep(1);
	char line[1000];
	char buffer[2000];
	while(fgets(line, 1000, fd)){
		sscanf(line, "%*d %s", buffer);
		char * file = malloc(strlen(dir) + strlen(buffer));
		strcpy(file, dir);
		strcat(file, buffer);
		int currentFd;
		currentFd = open(file, O_RDONLY);
		printf("sending this file %s\n", file);
		send(sock, buffer, 1000, 0); //sends appropriate directory of file
		//finds size of file
		struct stat fileStat;
		char fileSize[100];
		fstat(currentFd, &fileStat);
		sprintf(fileSize, "%d", fileStat.st_size);
		strcat(fileSize, "\0");
		send(sock, fileSize, 1000, 0); //sends the size of file
		printf("I said file is this big %s\n", fileSize);
		int sent;
		int remaining = fileStat.st_size;
		while( (remaining > 0) && ((sent = sendfile(sock, currentFd, NULL, 1000)) > 0) ){
			printf("sent %d\n", sent);
			remaining = remaining - sent;
		}
		close(currentFd);
	}
	fclose(fd);
	int mfd = open(manifest, O_RDONLY);
	//find manifest size
	struct stat mfileStat;
	char mfileSize[100];
	fstat(mfd, &mfileStat);
	sprintf(mfileSize, "%d", mfileStat.st_size);
	strcat(mfileSize, "\0");
	send(sock, mfileSize, 1000, 0); //sends the size of manifest
	
	int msent;
	int mremaining = mfileStat.st_size;
	while( (mremaining > 0) && ((msent = sendfile(sock, mfd, NULL, 1000)) > 0) ){
		printf("sent %d\n", msent);
		mremaining = mremaining - msent;
	}
	return 0; 
}

int checkout(char * dirp, int sock, char * name){
	DIR *dir;
	struct dirent *dent;
	int ver = 1;
	//open directory
	dir = opendir(dirp);
	//graceful error if dir can't be opened
	if(dir == NULL){
		printf("Directory %s cannot be opened\n", dirp);
		return;
	}
	//finds the latest version number
	while((dent = readdir(dir)) != NULL){
		// skip over [.] and [..]
		if(!strcmp(".", dent->d_name) || !strcmp("..", dent->d_name)){
			continue;	
		}
		int check;
		check = atoi(dent->d_name);
		if(check > ver){
			ver = check;
		}
	}
	char * projDir = malloc(4 + strlen(dirp));
	strcpy(projDir, dirp);
	strcat(projDir, "/\0");
	char version[20];
	sprintf(version, "%d", ver);
	strcat(projDir, version);
	strcat(projDir, "/\0");
	sendAll(projDir, sock);
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
		char command[50];
		recv(comm, command, 50, 0);
		char name[2000];
		recv(comm, name, 2000, 0);
		
		printf("client wants to %s %s\n", command, name);
		
		struct stat st1 = {0};
		if (stat("./projects", &st1) == -1) {
			mkdir("./projects", 0700);
		}
	
		char * projDir = malloc(12 + strlen(name));
		strcpy(projDir, "./projects/\0");
		strcat(projDir, name);
		
		if(strcmp(command, "create") == 0){
			create(projDir, comm, name);
		}else{
			struct stat st2 = {0};
			if (stat(projDir, &st2) == -1) {
				printf("project does not exist on server\n");
				send(comm, "error\0", 50, 0);
				close(comm);
				close(list);
				
				return 0;
			}
			
			if(strcmp(command, "destroy") == 0){
				destroy(projDir);
				send(comm, "success\0", 8, 0);
			}else if(strcmp(command, "checkout") == 0){
				checkout(projDir, comm, name);
			}
		}
	}
	
	return 0;
}
