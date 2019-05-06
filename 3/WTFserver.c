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
#include <openssl/sha.h>
struct node{
	pthread_mutex_t lock;
	char * name;
	struct node * next;
};

struct multiArgs{
	char * name;
	char * dir;
	int socket;
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

int getFileSizeInBytes(const char* path){
	struct stat fileStat;
	stat(path, &fileStat);
	int size = fileStat.st_size;
	return size;
}

char * createHashcode(char * fileStream, size_t length, char* buffer){
	int i = 0;
	char shaConverted[2];
	unsigned char hash[SHA256_DIGEST_LENGTH];
	
	SHA256(fileStream, length, hash);
	strcpy(buffer, "");
	for(i = 0; i < SHA256_DIGEST_LENGTH; i++){
        sprintf(shaConverted, "%02x", hash[i]);
		strcat(buffer, shaConverted);
	}	
	return buffer;
}

char * readFileAndHash(char* filePath, char* hashcode){
	size_t length;
	int fileBytes = getFileSizeInBytes(filePath);
	int fileDesc = open(filePath, O_RDONLY);
	if(fileDesc == -1){
 	  perror("Error opening file");
     	  return;
	}
	char* stream = malloc((fileBytes+1)*sizeof(char));
	read(fileDesc, stream, fileBytes);
	stream[fileBytes] = '\0';
	length = strlen(stream);
	strcpy(hashcode, createHashcode(stream, length, hashcode));
	free(stream);
	int closeStatus = close(fileDesc);
	return hashcode;
}

int makeDirectories(char * dir){
	char * test = malloc(strlen(dir) + 3);
	printf("making directories for %s\n", dir);
	strcpy(test, "\0");
	int i;
	for(i = 2; i < strlen(dir); i++){
		if(dir[i] == '/'){
			struct stat st = {0};
			if (stat(test, &st) == -1) {
				printf("making this directory %s\n", test);
				mkdir(test, 0700);
			}
		}
		char * c = malloc(2);
		c[0] = dir[i];
		c[1] = '\0';
		strcat(test, c);
	}
}

void * upgrade(void * tArgs){
	printf("SUH\n");
	struct multiArgs * args = (struct multiArgs *) tArgs;
	char * projDir = malloc(strlen(args->dir) + 1);
	char fileName[1000];
	int sock = args->socket;
	strcpy(projDir, args->dir);
	recv(sock, fileName, 1000, 0);
	DIR *dir;
	struct dirent *dent;
	struct dirent *last_dent;
	dir = opendir(projDir);
	//find most recent dir
	while((dent = readdir(dir)) != NULL){
		last_dent = dent;
	}
	char * filePath = malloc((strlen(projDir) + strlen(last_dent->d_name) + strlen(fileName) + 3)*sizeof(char));
	strcpy(filePath, projDir);
	strcat(filePath, "/");
	strcat(filePath, last_dent->d_name);
	strcat(filePath, "/");
	strcat(filePath, fileName);
	int fd = open(filePath, O_RDONLY);

	struct stat fileStat;
	char fileSize[1000];
	fstat(fd, &fileStat);
	sprintf(fileSize, "%d", fileStat.st_size);
	send(sock, fileSize, strlen(fileSize), 0);
	sleep(1);
	int sent;
	int remaining = fileStat.st_size;
	while( (remaining > 0) && ((sent = sendfile(sock, fd, NULL, 1000)) > 0) ){
		remaining = remaining - sent;	
	}
	close(fd);
	return NULL;
}
void * update(void * tArgs){
	struct multiArgs * args = (struct multiArgs *) tArgs;
	char * projDir = malloc(strlen(args->dir) + 1);
	int sock = args->socket;
	strcpy(projDir, args->dir);
	
	DIR *dir;
	struct dirent *dent;
	struct dirent *last_dent;
	dir = opendir(projDir);
	//find most recent dir
	while((dent = readdir(dir)) != NULL){
		last_dent = dent;
	}
	char * server_manifest = malloc((strlen(projDir) + strlen(last_dent->d_name) + strlen("/.Manifest") + 2)*sizeof(char));
	strcpy(server_manifest, projDir);
	strcat(server_manifest, "/");
	strcat(server_manifest, last_dent->d_name);
	strcat(server_manifest, "/.Manifest");
	int fd = open(server_manifest, O_RDONLY);
	struct stat fileStat;
	char fileSize[1000];
	fstat(fd, &fileStat);
	sprintf(fileSize, "%d", fileStat.st_size);
	send(sock, fileSize, strlen(fileSize), 0);
	sleep(1);
	int sent;
	int remaining = fileStat.st_size;
	
	while( (remaining > 0) && ((sent = sendfile(sock, fd, NULL, 1000)) > 0) ){
		remaining = remaining - sent;
	}
	close(fd);
	return NULL;
}

void * create(void * tArgs){
	if(pthread_mutex_lock(&masterLock) != 0){
		printf("broken thread lock\n");
		return NULL;
	}
	struct multiArgs * args = (struct multiArgs *) tArgs;
	char * projDir = malloc(strlen(args->dir) + 1);
	char * name = malloc(strlen(args->name) + 1);
	int sock = args->socket;
	
	strcpy(projDir, args->dir);
	strcpy(name, args->name);
	
	struct stat st = {0};
	if (stat(projDir, &st) == -1) {
		mkdir(projDir, 0700);
	}else{
		printf("project already exists\n");
		send(sock, "0", 1, 0);
		pthread_mutex_unlock(&masterLock);
		return NULL;
	}
	
	struct node * ptr = malloc(sizeof(struct node));
	ptr->name = malloc(strlen(name) + 1);
	strcpy(ptr->name, name);
	ptr->next = keychain;
	keychain = ptr;
	
	char * verDir = malloc(1 + strlen(projDir));
	strcpy(verDir, projDir);
	strcat(verDir, "/1");
	mkdir(verDir, 0700);
	
	char * manifest = malloc(12 + strlen(verDir));
	strcpy(manifest, verDir);
	strcat(manifest, "/.Manifest");
	printf("%s\n", manifest);
	int fd = open(manifest, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG);
	if(fd == -1){
		printf("unable to write manifest\n");
		send(sock, "0", 1, 0);
		pthread_mutex_unlock(&masterLock);
		return NULL;
	}
	write(fd, "0\t1\n", 4);
	
	struct stat fileStat;
	char fileSize[1000];
	fstat(fd, &fileStat);
	sprintf(fileSize, "%d", fileStat.st_size);
	send(sock, fileSize, strlen(fileSize), 0);
	sleep(1);
	int sent;
	int remaining = fileStat.st_size;
	close(fd);
	fd = open(manifest, O_RDONLY);
	while( (remaining > 0) && ((sent = sendfile(sock, fd, NULL, 1000)) > 0) ){
		remaining = remaining - sent;
	}
	
	close(fd);
	free(manifest);
	free(projDir);
	free(name);
	free(verDir);
	pthread_mutex_unlock(&masterLock);
	return NULL;
}

void * destroy(char * currentDir){
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
		return NULL;
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
			destroy(newPath);
			free(newPath);
			continue;
		}else if(typeInt == 1){
			// tokenize file and add tokens to linked list, keeping track of frequency
			fileType = "Regular File";
			remove(path);
			continue;
		}else {
			fileType = "Neither";
		}
		free(path);
	}
	closedir(dir);
	remove(currentDir);
	return NULL;
}

void * destroyP(void * tArgs){
	if(pthread_mutex_lock(&masterLock) != 0){
		printf("broken thread lock\n");
		return NULL;
	}

	struct multiArgs * args = (struct multiArgs *) tArgs;
	char * projDir = malloc(strlen(args->dir) + 1);
	char * name = malloc(strlen(args->name) + 1);
	int sock = args->socket;
	
	strcpy(projDir, args->dir);
	strcpy(name, args->name);
	
	int locked = 0;
	struct node * ptr = keychain;
	struct node * prev = keychain;
	while(ptr != NULL){
		if(strcmp(ptr->name, name) == 0){
			if(pthread_mutex_lock(&(ptr->lock)) != 0){
				printf("unable to lock this project\n");
				send(sock, "error", 8, 0);
				pthread_mutex_unlock(&masterLock);
				return NULL;
			}
			locked = 1;
			break;
		}
		prev = ptr;
		ptr = ptr->next;
	}
	if(locked != 1){
		printf("Could not find the lock for this project\n");
		send(sock, "error", 8, 0);
		return NULL;
	}
	destroy(projDir);
	send(sock, "success\0", 8, 0);
	pthread_mutex_unlock(&(ptr->lock));
	
	if(ptr == keychain){
		keychain = ptr->next;
	}else{
		prev->next = ptr->next;
	}
	
	pthread_mutex_unlock(&masterLock);
	
	return NULL;
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
		int sent;
		int remaining = fileStat.st_size;
		while( (remaining > 0) && ((sent = sendfile(sock, currentFd, NULL, 1000)) > 0) ){
			remaining = remaining - sent;
		}
		close(currentFd);
		free(file);
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
		mremaining = mremaining - msent;
	}
	free(manifest);
	
	return 0; 
}

void * checkout(void * tArgs){
	struct multiArgs * args = (struct multiArgs *) tArgs;
	char * dirp = malloc(strlen(args->dir) + 1);
	char * name = malloc(strlen(args->name) + 1);
	int sock = args->socket;
	
	strcpy(dirp, args->dir);
	strcpy(name, args->name);
	
	int locked = 0;
	struct node * ptr = keychain;
	while(ptr != NULL){
		if(strcmp(ptr->name, name) == 0){
			if(pthread_mutex_lock(&(ptr->lock)) != 0){
				printf("unable to lock this project\n");
				send(sock, "0\0", 8, 0);
				return NULL;
			}
			locked = 1;
			break;
		}
	}
	
	if(locked != 1){
		printf("Could not find the lock for this project\n");
	}
	
	DIR *dir;
	struct dirent *dent;
	int ver = 1;
	//open directory
	dir = opendir(dirp);
	//graceful error if dir can't be opened
	if(dir == NULL){
		printf("Directory %s cannot be opened\n", dirp);
		return NULL;
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
	char version[20];
	sprintf(version, "%d", ver);
	char * projDir = malloc(2 + strlen(version) + strlen(dirp));
	strcpy(projDir, dirp);
	strcat(projDir, "/\0");
	strcat(projDir, version);
	strcat(projDir, "/\0");
	sendAll(projDir, sock);
	free(projDir);
	free(dirp);
	free(name);
	pthread_mutex_unlock(&(ptr->lock));
	return NULL;
}

void * rollback(void * tArgs){
	struct multiArgs * args = (struct multiArgs *) tArgs;
	char * name = malloc(strlen(args->name) + 1);
	int sock = args->socket;
	
	strcpy(name, args->name);
	
	int locked = 0;
	struct node * ptr = keychain;
	while(ptr != NULL){
		if(strcmp(ptr->name, name) == 0){
			if(pthread_mutex_lock(&(ptr->lock)) != 0){
				printf("unable to lock this project\n");
				return NULL;
			}
			locked = 1;
			break;
		}
	}
	
	if(locked != 1){
		printf("Could not find the lock for this project\n");
		return NULL;
	}
	char version[100];
	recv(sock, version, 100, 0);
	char * projDir = malloc(strlen(name) + 14);
	strcpy(projDir, "./projects/\0");
	strcat(projDir, name);
	int ver = atoi(version);
	
	DIR *dir;
	struct dirent *dent;
	//open directory
	dir = opendir(projDir);
	//graceful error if dir can't be opened
	if(dir == NULL){
		printf("Directory %s cannot be opened\n", projDir);
		send(sock, "error", 50, 0);
		free(name);
		free(projDir);
		pthread_mutex_unlock(&(ptr->lock));
		return NULL;
	}
	int rolled = 0;
	while((dent = readdir(dir)) != NULL){
		// skip over [.] and [..]
		if(!strcmp(".", dent->d_name) || !strcmp("..", dent->d_name)){
			continue;	
		}
		int check = atoi(dent->d_name);
		if(check > ver){
			rolled = 1;
			char * delete = malloc(strlen(projDir) + strlen(dent->d_name) + 3);
			strcpy(delete, projDir);
			strcat(delete, "/\0");
			strcat(delete, dent->d_name);
			destroy(delete);
			free(delete);
		}
	}
	if(rolled == 0){
		send(sock, "nochange", 30, 0);
	}else{
		send(sock, "success", 30, 0);
	}
	free(name);
	free(projDir);
	pthread_mutex_unlock(&(ptr->lock));
	return NULL;
}

char * getCommitNum(char * path){
	DIR *dir;
	struct dirent *dent;
	int ver = 0;
	
	struct stat st = {0};
	if (stat(path, &st) == -1) {
		mkdir(path, 0700);
	}
	
	//open directory
	dir = opendir(path);
	//graceful error if dir can't be opened
	if(dir == NULL){
		printf("Directory %s cannot be opened\n", path);
		return NULL;
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
	char * version = malloc(20);
	sprintf(version, "%d", ver + 1);
	return version;
}

void * commit(void * tArgs){
	struct multiArgs * args = (struct multiArgs *) tArgs;
	char * name = malloc(strlen(args->name) + 1);
	char * dirp = malloc(strlen(args->dir) + 1);
	int sock = args->socket;
	
	strcpy(name, args->name);
	strcpy(dirp, args->dir);
	
	int locked = 0;
	struct node * ptr = keychain;
	while(ptr != NULL){
		if(strcmp(ptr->name, name) == 0){
			if(pthread_mutex_lock(&(ptr->lock)) != 0){
				printf("unable to lock this project\n");
				return NULL;
			}
			locked = 1;
			break;
		}
	}
	
	if(locked != 1){
		printf("Could not find the lock for this project\n");
		return NULL;
	}
	
	DIR *dir;
	struct dirent *dent;
	int ver = 1;
	//open directory
	dir = opendir(dirp);
	//graceful error if dir can't be opened
	if(dir == NULL){
		printf("Directory %s cannot be opened\n", dirp);
		return NULL;
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
	char version[20];
	sprintf(version, "%d", ver);
	char * manifest = malloc(15 + strlen(version) + strlen(dirp));
	strcpy(manifest, dirp);
	strcat(manifest, "/\0");
	strcat(manifest, version);
	strcat(manifest, "/.Manifest\0");
	
	int mfd = open(manifest, O_RDONLY);
	//find manifest size
	struct stat mfileStat;
	char mfileSize[100];
	fstat(mfd, &mfileStat);
	sprintf(mfileSize, "%d", mfileStat.st_size);
	send(sock, mfileSize, 100, 0); //sends the size of manifest
	int msent;
	int mremaining = mfileStat.st_size;
	while( (mremaining > 0) && ((msent = sendfile(sock, mfd, NULL, 1000)) > 0) ){
		mremaining = mremaining - msent;
	}
	char fileSize[100];
	recv(sock, fileSize, 100, 0);
	if(strcmp(fileSize,"error") == 0){
		printf("Client needs to sync with server first\n");
	}
	int size = atoi(fileSize);
	char * commit = malloc(30 + strlen(version) + strlen(dirp));
	strcpy(commit, dirp);
	strcat(commit, "/\0");
	strcat(commit, version);
	strcat(commit, "/.commits\0");
	char * commitNum = getCommitNum(commit);
	strcat(commit, "/\0");
	strcat(commit, commitNum);
	
	
	int fd = open(commit, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
	char * incoming = malloc(size + 1);
	int remaining = size;
	int written;
	while( (remaining > 0) && ((written = recv(sock, incoming, size, 0)) > 0) ){
		write(fd, incoming, written);
		remaining = remaining - written;
	}
	close(fd);
	pthread_mutex_unlock(&(ptr->lock));
	free(commit);
	free(incoming);
	free(manifest);
	free(dirp);
	free(name);
	return NULL;
}

int duplicate(char * oldDir, char * newDir){
	printf("preparing to duplicate %s\n", oldDir);
	DIR *dir;
	struct dirent *dent;
	//open directory
	dir = opendir(oldDir);
	//graceful error if dir can't be opened
	if(dir == NULL){
		printf("Directory %s cannot be opened\n", oldDir);
		return 0;
	}
	//finds the latest version number
	while((dent = readdir(dir)) != NULL){
		// skip over [.] and [..]
		if(!strcmp(".", dent->d_name) || !strcmp("..", dent->d_name)){
			continue;	
		}
		
		char * path = malloc(strlen(oldDir) + strlen(dent->d_name) + 3);
		strcpy(path, oldDir);
		strcat(path, "/\0");
		strcat(path, dent->d_name);
		printf("copying %s\n", path);
		int type = regularFileOrDirectory(path);
		char * newDirP = malloc(strlen(newDir) + strlen(dent->d_name) + 3);
		strcpy(newDirP, newDir);
		strcat(newDirP, "/\0");
		strcat(newDirP, dent->d_name);
		if(type == 0){
			if(strcmp(dent->d_name, ".commits") != 0){
				struct stat st = {0};
				if (stat(newDirP, &st) == -1) {
					mkdir(newDirP, 0700);
				}
				duplicate(path, newDirP);
			}
		}else if(strcmp(dent->d_name, ".Manifest") != 0){
			char c;
			FILE * fdr;
			FILE * fdw;
			fdr = fopen(path, "r");
			fdw = fopen(newDirP, "w");
			while((c = fgetc(fdr)) != EOF){
				fwrite(&c, 1, 1, fdw);
			} 
			fclose(fdr);
			fclose(fdw);
		}
		free(path);
		free(newDirP);
	}
	closedir(dir);
	return 1;
}

int push(char * dirp, int sock){
	send(sock, "success", 50, 0);
	char * commitSize = malloc(100);
	recv(sock, commitSize, 100, 0);
	int totalSize = atoi(commitSize);
	free(commitSize);
	while(1){
		char command[2];
		char version[10];
		int size;
		recv(sock, command, 2, 0);
		recv(sock, version, 10, 0);
		char * path = malloc(totalSize);
		char * hash = malloc(2*SHA256_DIGEST_LENGTH);
		recv(sock, path, totalSize, 0);
		recv(sock, hash, 2*SHA256_DIGEST_LENGTH, 0);
		char * fullPath = malloc(strlen(path) + strlen(dirp) + 3);
		strcpy(fullPath, dirp);
		strcat(fullPath, "/\0");
		strcat(fullPath, path);
		//printf("%s\t%s\t%s\t%s\n", command, version, fullPath, hash);
		if(strcmp(command, "Z") == 0){
			printf("out\n");
			break;
		}
		
		if(strcmp(command, "D") == 0){
			remove(fullPath);
			continue;
		}
		
		if(strcmp(command, "A") == 0){
			makeDirectories(fullPath);
			char fileSize[100];
			recv(sock, fileSize, 100, 0);
			int size = atoi(fileSize);
			printf("%d\n", size);
			int fd = open(fullPath, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
			char * incoming = malloc(size + 1);
			int remaining = size;
			int written;
			while( (remaining > 0) && ((written = recv(sock, incoming, size, 0)) > 0) ){
				printf("received %d\n", written);
				write(fd, incoming, written);
				remaining = remaining - written;
			}
			close(fd);
			free(incoming);
		}else if(strcmp(command, "M") == 0){
			char manifestSize[50];
			int n = recv(sock, manifestSize, 50, 0);
			int size = atoi(manifestSize);
			char * manifest = malloc(strlen(dirp) + 15);
			strcpy(manifest, dirp);
			strcat(manifest, "/.Manifest\0");
			int file = open(manifest, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
			
			char * incoming = malloc(size + 1);
			printf("matched %d writing %s bytes in %s\n", n, manifestSize, manifest);
			int remaining = size;
			int written;
			while( (remaining > 0) && ((written = recv(sock, incoming, size, 0)) > 0) ){
				write(file, incoming, written);
				remaining = remaining - written;
			}
			close(file);
			free(manifest);
			free(incoming);
		}else if(strcmp(command, "U") == 0){
			char fileSize[100];
			recv(sock, fileSize, 100, 0);
			int size = atoi(fileSize);
			printf("%d\n", size);
			int fd = open(fullPath, O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
			char * incoming = malloc(size + 1);
			int remaining = size;
			int written;
			while( (remaining > 0) && ((written = recv(sock, incoming, size, 0)) > 0) ){
				printf("received %d\n", written);
				write(fd, incoming, written);
				remaining = remaining - written;
			}
			close(fd);
			free(incoming);
		}else{
			printf("invalid command from commit file\n");
		}
		free(path);
		free(hash);
		free(fullPath);
	}
	return 0;
}

int prepPush(char * dirp, int ver, int sock){
	char version[10];
	sprintf(version, "%d", ver);
	char versionInc[10];
	sprintf(versionInc, "%d", ver + 1);
	char * oldDir = malloc(strlen(dirp) + strlen(version) + 3);
	strcpy(oldDir, dirp);
	strcat(oldDir, "/\0");
	strcat(oldDir, version);
	char * newDir = malloc(strlen(dirp) + strlen(versionInc) + 3);
	strcpy(newDir, dirp);
	strcat(newDir, "/\0");
	strcat(newDir, versionInc);
	mkdir(newDir, 0700);
	
	if(duplicate(oldDir, newDir) == 1){
		push(newDir, sock);
	}
	free(newDir);
	free(oldDir);
	return 0;
}

void * checkPush(void * tArgs){
	struct multiArgs * args = (struct multiArgs *) tArgs;
	char * name = malloc(strlen(args->name) + 1);
	char * dirp = malloc(strlen(args->dir) + 1);
	int sock = args->socket;
	
	strcpy(name, args->name);
	strcpy(dirp, args->dir);
	
	int locked = 0;
	struct node * ptr = keychain;
	while(ptr != NULL){
		if(strcmp(ptr->name, name) == 0){
			if(pthread_mutex_lock(&(ptr->lock)) != 0){
				printf("unable to lock this project\n");
				return NULL;
			}
			printf("locked\n");
			locked = 1;
			break;
		}
	}
	
	if(locked != 1){
		printf("Could not find the lock for this project\n");
		return NULL;
	}
	DIR *dir;
	struct dirent *dent;
	int ver = 1;
	//open directory
	dir = opendir(dirp);
	//graceful error if dir can't be opened
	if(dir == NULL){
		printf("Directory %s cannot be opened\n", dirp);
		send(sock, "error", 50, 0);
		free(name);
		free(dirp);
		return NULL;
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
	closedir(dir);
	char version[10];
	sprintf(version, "%d", ver);
	char * path = malloc(strlen(dirp) + strlen(version) + 15);
	strcpy(path, dirp);
	strcat(path, "/\0");
	strcat(path, version);
	strcat(path, "/.commits");
	dir = opendir(path);
	//graceful error if dir can't be opened
	if(dir == NULL){
		printf("Directory %s cannot be opened\n", path);
		send(sock, "dont", 50, 0);
		printf("sent dont\n");
		pthread_mutex_unlock(&(ptr->lock));
		return NULL;
	}
	send(sock, "send", 50, 0);
	
	char * code = malloc(256);
	int received = recv(sock, code, 256, 0);
	printf("%s\n%d\n", code, received);
	while((dent = readdir(dir)) != NULL){
		if(!strcmp(".", dent->d_name) || !strcmp("..", dent->d_name)){
			continue;	
		}
		
		char * commit = malloc(3 + strlen(dent->d_name) + strlen(path));
		strcpy(commit, path);
		strcat(commit, "/\0");
		strcat(commit, dent->d_name);
		
		char hashcode[2*SHA256_DIGEST_LENGTH];
		strcpy(hashcode,readFileAndHash(commit, hashcode));
		printf("%s\n%d\n", code, received);
		if(strcmp(code, hashcode) == 0){
			prepPush(dirp, ver, sock);
			pthread_mutex_unlock(&(ptr->lock));
			free(code);
			free(commit);
			free(dirp);
			free(path);
			free(name);
			return NULL;
		}
	}
	send(sock, "no match", 50, 0);
	free(code);
	free(dirp);
	free(path);
	free(name);
	pthread_mutex_unlock(&(ptr->lock));
	return NULL;
}

void * currentVersion(void * tArgs){
	struct multiArgs * args = (struct multiArgs *) tArgs;
	char * name = malloc(strlen(args->name) + 1);
	char * dirp = malloc(strlen(args->dir) + 1);
	int sock = args->socket;
	
	strcpy(name, args->name);
	strcpy(dirp, args->dir);
	
	int locked = 0;
	struct node * ptr = keychain;
	while(ptr != NULL){
		if(strcmp(ptr->name, name) == 0){
			if(pthread_mutex_lock(&(ptr->lock)) != 0){
				printf("unable to lock this project\n");
				return NULL;
			}
			printf("locked\n");
			locked = 1;
			break;
		}
	}
	
	if(locked != 1){
		printf("Could not find the lock for this project\n");
		return NULL;
	}
	
	DIR *dir;
	struct dirent *dent;
	int ver = 1;
	//open directory
	dir = opendir(dirp);
	//graceful error if dir can't be opened
	if(dir == NULL){
		printf("Directory %s cannot be opened\n", dirp);
		send(sock, "0", 50, 0);
		free(name);
		free(dirp);
		pthread_mutex_unlock(&(ptr->lock));
		return NULL;
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
	closedir(dir);
	char version[10];
	sprintf(version, "%d", ver);
	char * path = malloc(strlen(dirp) + strlen(version) + 15);
	strcpy(path, dirp);
	strcat(path, "/\0");
	strcat(path, version);
	strcat(path, "/.Manifest");
	
	int file = open(path, O_RDONLY);
	struct stat fileStat;
	fstat(file, &fileStat);
	int size = fileStat.st_size;
	close(file);
	
	FILE * fd = fopen(path, "r");
	char * line = malloc(size + 1);
	char * sizeStr = malloc(50);
	sprintf(sizeStr, "%d", size);
	send(sock, sizeStr, 50, 0);
	fgets(line, size, fd);
	while(fgets(line, size, fd) != NULL){
		send(sock, "R", 2, 0);
		int n = send(sock, line, size, 0);
	}
	send(sock, "S", 2, 0);
	free(sizeStr);
	free(line);
	free(path);
	free(name);
	free(dirp);
	pthread_mutex_unlock(&(ptr->lock));
	return NULL;
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
		if(strcmp(command, "invalid") == 0){
			printf("client has input an invalid command\n");
			continue;
		}else{
			recv(comm, name, 2000, 0);
			printf("client wants to %s %s\n", command, name);
		}
		
		struct stat st1 = {0};
		if (stat("./projects", &st1) == -1) {
			mkdir("./projects", 0700);
		}
	
		char * projDir = malloc(12 + strlen(name));
		strcpy(projDir, "./projects/\0");
		strcat(projDir, name);
		
		if(strcmp(command, "create") == 0){
			struct multiArgs * args = malloc(sizeof(struct multiArgs));
			args->name = malloc(strlen(name) + 1);
			strcpy(args->name, name);
			args->dir = malloc(strlen(projDir) + 1);
			strcpy(args->dir, projDir);
			args->socket = comm;
			pthread_t id;
			pthread_create(&id, NULL, create, (void *) args);
			//create(projDir, comm, name);
		}else{
			struct stat st2 = {0};
			if (stat(projDir, &st2) == -1) {
				printf("project does not exist on server\n");
				send(comm, "error", 50, 0);
				close(comm);
				continue;
			}
			
			struct multiArgs * args = malloc(sizeof(struct multiArgs));
			args->name = malloc(strlen(name) + 1);
			strcpy(args->name, name);
			args->dir = malloc(strlen(projDir) + 1);
			strcpy(args->dir, projDir);
			args->socket = comm;
			
			if(strcmp(command, "destroy") == 0){
				pthread_t id;
				pthread_create(&id, NULL, destroyP, (void *) args);
			}else if(strcmp(command, "checkout") == 0){
				pthread_t id;
				pthread_create(&id, NULL, checkout, (void *) args);
			}else if(strcmp(command, "rollback") == 0){
				pthread_t id;
				pthread_create(&id, NULL, rollback, (void *) args);
			}else if(strcmp(command, "commit") == 0){
				pthread_t id;
				pthread_create(&id, NULL, commit, (void *) args);
			}else if(strcmp(command, "push") == 0){
				pthread_t id;
				pthread_create(&id, NULL, checkPush, (void *) args);
			}else if(strcmp(command, "update") == 0){
				pthread_t id;
				pthread_create(&id, NULL, update, (void *) args);
			}else if(strcmp(command, "upgrade") == 0){
				pthread_t id;
				pthread_create(&id, NULL, upgrade, (void *) args);
			}else if(strcmp(command, "currentVersion") == 0){
				pthread_t id;
				pthread_create(&id, NULL, currentVersion, (void *) args);
			}
		}
	}
	
	return 0;
}