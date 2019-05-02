<<<<<<< HEAD
#include <stdio.h>
#include <stdlib.h>
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
#include <openssl/sha.h>
/* --------------- JON ADDED THESE ----------------*/
struct node {
	int version;
	char * filePath;
	char * hashcode;
	struct node * next;
};

int numFiles;
int versionNum;

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

struct node* addFileToList(int version, char* filePath, char* hashcode, struct node* head){
	struct node * current = head;
	while(current != NULL){
		if(strcmp(current->filePath, filePath) == 0 && strcmp(current->hashcode, hashcode) != 0){
			printf("File already exists. Changes were made.\n");
			strcpy(current->hashcode, hashcode);
			return head;
		}else if(strcmp(current->filePath, filePath) == 0 ){
			printf("File already exists. No changes were made.\n");
			return head;
		}
		if(current->next != NULL){
			current = current->next;
		}else{
			break;
		}
	}
	struct node *newNode = (struct node *) malloc(sizeof(struct node));
	newNode->version = version;
	newNode->filePath = malloc((strlen(filePath) + 1)*sizeof(char));
	newNode->hashcode = malloc((strlen(hashcode) + 1)*sizeof(char));
	strcpy(newNode->filePath, filePath);
	strcpy(newNode->hashcode, hashcode);
	newNode->next = NULL; 
	if(head == NULL){
		head = newNode;
	}else{
		current->next = newNode;
	}
	return head;
}
void printList(struct node * head){
	struct node * current = head;
	while(current != NULL){
	   printf("%d %s %s\n", current->version, current->filePath, current->hashcode);
	   current = current->next;
	}
}
struct node* removeFileFromList(char* filePath, struct node * head){
	struct node * prev = head;
	struct node * current = head;
	while(current != NULL){
		if(strcmp(current->filePath, filePath) == 0){
			if(current == head){
				prev = current->next;
				current->next = NULL;
				free(current->filePath);
				free(current->hashcode);
				free(current);
				return prev;
			}else{
				prev->next = current->next;
				current->next = NULL;
				free(current->filePath);
				free(current->hashcode);
				free(current);
				return head;
			}
		}
		if(current->next != NULL){
			prev = current;
			current = current->next;
		}else{
			printf("File does not exist in manifest.\n");
			break;
		}
	}
	return head;
}

struct node* createManifestList(char * manifestPath, struct node * head){
	FILE *fp;
	char str[256];
	int version, num, vNum;
	char filePath[256];
	char hashcode[256];
	fp = fopen(manifestPath, "r");	
	if(fp == NULL) {
     	  perror("Error opening file");
     	  return 0;
  	}
	fscanf(fp, "%d\t%d", &num, &vNum);
	numFiles = num;
	versionNum = vNum;
	while(fscanf(fp, "%d %s %s", &version, filePath, hashcode) != EOF){
		head = addFileToList(version, filePath, hashcode, head);
	}
	fclose(fp);
	return head;
}

void writeToManifest(char* manifestPath, struct node * head, int add){
	FILE *fp;
	struct node * current = head;
	fp = fopen(manifestPath , "w");	
	if(fp == NULL) {
     	  perror("Error opening file");
     	  return;
  	}
	fprintf(fp,"%d\t%d\n", numFiles + add, versionNum);
	while(current != NULL){
	   fprintf(fp, "%d\t%s\t%s\n", current->version, current->filePath, current->hashcode);
	   current = current->next;
	}
	fclose(fp);
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

void addCommand(char* filePath, char* filePathWithoutProj, char* manifestPath){
	// get linked list of manifest file
	struct node * head;
	head = createManifestList(manifestPath, head);

	size_t length;
	char hashcode[2*SHA256_DIGEST_LENGTH];
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
	head = addFileToList(1,filePathWithoutProj,hashcode, head);
	writeToManifest(manifestPath, head, 1);
}

void removeCommand(char * filePathWithoutProj, char* manifestPath){
	// get linked list of manifest file
	struct node * head;
	head = createManifestList(manifestPath, head);
	head = removeFileFromList(filePathWithoutProj, head);
	writeToManifest(manifestPath, head, -1);
}



/* ----------------END JON-------------------*/


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
		write(fd, buffer, written);
		remaining = remaining - written;
		printf("%d remaining\n", remaining);
	}
	close(fd);
	return 0;
}

int destroy(char * name, int sock){
	send(sock, "destroy", 7, 0);
	sleep(1);
	send(sock, name, strlen(name), 0);
	char buffer[1000];
	recv(sock, buffer, 1000, 0);
	if(strcmp(buffer, "error") == 0){
		printf("This project does not exist on the server\n");
	}else{
		printf("Success\n");
	}
	return 0;
}
int makeDirectories(char * dir){
	char * test = malloc(strlen(dir) + 1);
	strcpy(test, "./\0");
	printf("making directories for %s\n", dir);
	int i;
	for(i = 2; i < strlen(dir); i++){
		if(dir[i] == '/'){
			struct stat st = {0};
			if (stat(test, &st) == -1) {
				printf("making this directory %s\n", test);
				mkdir(test, 0700);
			}
		}
		char c = dir[i];
		strcat(test, &c);
	}
}

int checkout(char * name, int sock){
	char * projDir = malloc(3 + strlen(name));
	strcpy(projDir, "./\0");
	strcat(projDir, name);
	
	struct stat st = {0};
	if (stat(projDir, &st) == -1) {
		mkdir(projDir, 0700);
	}else{
		printf("Project already exists in local, please delete local copy of this project and use checkout again\n");
		return 0;
	}
	send(sock, "checkout", 8, 0);
	sleep(1);
	send(sock, name, strlen(name), 0);
	char buffer[1000];
	recv(sock, buffer, 20, 0);
	if(strcmp(buffer, "error") == 0){
		printf("This project does not exists on the server");
		return 0;
	}
	int num = atoi(buffer);
	int size;
	int i = 1;
	for(i = 1; i <= num; i++){
		char directory[1000];
		recv(sock, directory, 1000, 0);
		char * file = malloc(strlen(projDir) + strlen(directory) + 1);
		strcpy(file, projDir);
		strcat(file, "/\0");
		strcat(file, directory);
		makeDirectories(file);
		recv(sock, buffer, 1000, 0);
		printf("writing to %s\n", file);
		size = atoi(buffer);
		int remaining = size;
		int written;
		printf("size is %d\n", size);
		int fd = open(file, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
		char inc[1000];
		while( (remaining > 0) && ((written = recv(sock, inc, size, 0)) > 0) ){
			printf("haha\n");
			write(fd, inc, written);
			remaining = remaining - written;
			printf("%d remaining\n", remaining);
		}
		close(fd);
	}
	recv(sock, buffer, 1000, 0);
	printf("manifest is this big %s\n", buffer);
	size = atoi(buffer);
	int left = size;
	int received;
	char * manifest = malloc(strlen(projDir) + 12);
	strcpy(manifest, projDir);
	strcat(manifest, "/.Manifest");
	int fd = open(manifest, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
	char incoming[1000];
	
	while( (left > 0) && ((received = recv(sock, incoming, size, 0)) > 0) ){
		write(fd, incoming, received);
		left = left - received;
		printf("%d remaining\n", left);
	}
	close(fd);
	return 0;
}

int main(int argc, char ** argv){
	int port = atoi(argv[1]);
	char * command = malloc(strlen(argv[2]) + 1);
	strcpy(command, argv[2]);
	if(strcmp(command, "configure") == 0){
		return 0;
	}
	char * name = malloc(strlen(argv[3]) + 1);
	strcpy(name, argv[3]);
	
	if(strcmp(name, "projects") == 0){
		printf("projects is an invalid project name because the server files are stored in a directory named \"projects\"\n");
		return 0;
	}
	
	if(strcmp(command, "remove") == 0 || strcmp(command, "add") == 0){
		char *manifestPath;
		char *filePath;
		manifestPath = (char*)malloc((strlen(name)+strlen("manifest")+2)*sizeof(char));
		filePath = (char*)malloc((strlen(name)+strlen(argv[4])+2)*sizeof(char));
		int fileType = regularFileOrDirectory(name);
		if(fileType != 0){
			printf("Invalid project name\n");
			return 1;
		}
		strcpy(filePath, name);
		strcat(filePath, "/");
		strcat(filePath, argv[4]);
		strcpy(manifestPath, name);
		strcat(manifestPath, "/.Manifest");
		if(strcmp(command, "add") == 0){
			addCommand(filePath, argv[4], manifestPath);
		}else if(strcmp(command, "remove") == 0){
			removeCommand(argv[4], manifestPath);
		}
		return 0;
	}
	
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
	if(strcmp(command, "create") == 0){
		create(name, socketfd);
	}else if(strcmp(command, "destroy") == 0){
		destroy(name, socketfd);
	}else if(strcmp(command, "checkout") == 0){
		checkout(name, socketfd);
	}else{
		printf("Invalid command given.\n");
	}

	
	return 0;
}