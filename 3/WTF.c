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
	char * UMAD;
	int version;
	char * filePath;
	char * hashcode;
	struct node * next;
};
int numFiles;
int versionNum;
int versionNum_s;
int numFiles_s;
int updateNumFiles;

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

struct node* addFileToList(int version, char* filePath, char* hashcode, struct node* head, char *UMAD){
	struct node * current = head;
	if(UMAD == NULL){
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
	}else{
		updateNumFiles++;
		while(current != NULL){
			if(current->next != NULL){
				current = current->next;
			}else{
				break;
			}
		}
	}
	struct node *newNode = (struct node *) malloc(sizeof(struct node));
	if(UMAD != NULL){
		newNode->UMAD = malloc((strlen(UMAD)+1)*sizeof(char));
		strcpy(newNode->UMAD, UMAD);
	}
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
		if(current->UMAD == NULL){
			printf("%d %s %s\n", current->version, current->filePath, current->hashcode);
		}else {
			printf("%s %d %s %s\n", current->UMAD, current->version, current->filePath, current->hashcode);
		}
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
	
	int len = strlen(manifestPath);
	char *last_two = &manifestPath[len-2];
	if(strcmp(last_two, "_s") == 0){
		numFiles_s = num;
		versionNum_s = vNum;	
	}else{
		numFiles = num;
		versionNum = vNum;
	}
	while(fscanf(fp, "%d %s %s", &version, filePath, hashcode) != EOF){
		head = addFileToList(version, filePath, hashcode, head, NULL);
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

void writeToUpdate(char* updatePath, struct node * head){
	FILE *fp;
	struct node * current = head;
	fp = fopen(updatePath , "w");	
	if(fp == NULL) {
     	  perror("Error opening file");
     	  return;
  	}
	fprintf(fp,"%d\t%d\n", updateNumFiles, versionNum);
	while(current != NULL){
	   fprintf(fp, "%s\t%d\t%s\t%s\n", current->UMAD, current->version, current->filePath, current->hashcode);
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

void addCommand(char* filePath, char* filePathWithoutProj, char* manifestPath){
	// get linked list of manifest file
	struct node * head;
	head = createManifestList(manifestPath, head);
	char hashcode[2*SHA256_DIGEST_LENGTH];
	strcpy(hashcode, readFileAndHash(filePath, hashcode));
	head = addFileToList(1,filePathWithoutProj,hashcode, head, NULL);
	writeToManifest(manifestPath, head, 1);
}

void removeCommand(char * filePathWithoutProj, char* manifestPath){
	// get linked list of manifest file
	struct node * head;
	head = createManifestList(manifestPath, head);
	head = removeFileFromList(filePathWithoutProj, head);
	writeToManifest(manifestPath, head, -1);
}

struct node * compareManifests(char * project, struct node * c_head, struct node * s_head, struct node * updatedList){
	struct node * updateList = updatedList;
	struct node * s_curr;
	struct node * c_curr = c_head;
	int found = 0;
	while(c_curr != NULL){
		s_curr = s_head;
		while(s_curr != NULL){
			if(strcmp(c_curr->filePath, s_curr->filePath) == 0){
				if(versionNum == versionNum_s){
					if(strcmp(c_curr->hashcode, s_curr->hashcode) != 0){
						//UPLOADED
						updateList = addFileToList(s_curr->version, s_curr->filePath, s_curr->hashcode, updateList, "U");
						s_head = removeFileFromList(c_curr->filePath, s_head);
					}
					found = 1;
					break;
				}else if(versionNum != versionNum_s && c_curr->version != s_curr->version){
					char liveHashcode[2*SHA256_DIGEST_LENGTH];
					char * fullFilePath = malloc((strlen(c_curr->filePath) + strlen(project) + 2)*sizeof(char));
					strcpy(fullFilePath, project);
					strcat(fullFilePath, "/");
					strcat(fullFilePath, c_curr->filePath);
					strcpy(liveHashcode,readFileAndHash(fullFilePath, liveHashcode));
					free(fullFilePath);
					if(strcmp(liveHashcode, c_curr->hashcode) == 0){
						//MODIFIED
						updateList = addFileToList(s_curr->version, s_curr->filePath, s_curr->hashcode, updateList, "M");
						s_head = removeFileFromList(c_curr->filePath, s_head);
						found = 1;
						break;
					}
				}
			}
			s_curr = s_curr->next;
		}
		if(found == 0){
			if(versionNum == versionNum_s){
				updateList = addFileToList(c_curr->version, c_curr->filePath, c_curr->hashcode, updateList, "U");
			}else if(versionNum != versionNum_s){
				//DELETED
				updateList = addFileToList(c_curr->version, c_curr->filePath, c_curr->hashcode, updateList, "D");
			}
		}
		found = 0;
		c_curr = c_curr->next;
	}
	s_curr = s_head;
	while(s_curr != NULL){
		if(versionNum != versionNum_s){
			//ADDED
			updateList = addFileToList(s_curr->version, s_curr->filePath, s_curr->hashcode, updateList, "A");
		}
		s_curr = s_curr->next;
	}
	return updateList;
}


/* ----------------END JON-------------------*/


int create(char * name, int sock){
	
	send(sock, "create", 50, 0);
	sleep(1);
	send(sock, name, 2000, 0);
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
	send(sock, "destroy", 50, 0);
	sleep(1);
	send(sock, name, 2000, 0);
	char buffer[1000];
	recv(sock, buffer, 1000, 0);
	if(strcmp(buffer, "error") == 0){
		printf("This project does not exist on the server\n");
	}else{
		printf("Successfully destroyed project: %s\n", name);
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

void updateCommand(char * project, char* clientManifestPath){
	char* serverManifestPath = malloc((strlen(clientManifestPath)+3)*sizeof(char));
	strcpy(serverManifestPath, clientManifestPath);
	strcat(serverManifestPath, "_s");
	struct node* c_head;
	struct node* s_head;
	struct node* updatedList;
	updateNumFiles = 0;
	c_head = createManifestList(clientManifestPath, c_head);
	s_head = createManifestList(serverManifestPath, s_head); 
	updatedList = compareManifests(project, c_head, s_head, updatedList);
	if(updatedList != NULL){
		printList(updatedList);
		char* updatePath = malloc((strlen(project) + strlen("/.Update") + 1)*sizeof(char));
		strcpy(updatePath, project);
		strcat(updatePath, "/.Update");
		writeToUpdate(updatePath, updatedList);
		free(updatePath);
	}else{
		printf("All files up to date.\n");
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
	send(sock, "checkout", 50, 0);
	sleep(1);
	send(sock, name, 2000, 0);
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
			write(fd, inc, written);
			remaining = remaining - written;
		}
		close(fd);
	}
	recv(sock, buffer, 1000, 0);
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
	}
	close(fd);
	return 0;
}

int rollback(char * name, int sock, char * version){
	send(sock, "rollback", 50, 0);
	sleep(1);
	send(sock, name, 2000, 0);
	send(sock, version, 100, 0);
	char buffer[30];
	recv(sock, buffer, 30, 0);
	if(strcmp(buffer, "error") == 0){
		printf("This project does not exist on the server\n");
	}else{
		printf("Successfully rolled back\n");
	}
	return 0;
}

int configure(char * ip, char * port){
	int fd = open("./.configure", O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
	write(fd, ip, strlen(ip));
	write(fd, "\t", 1);
	write(fd, port, strlen(port));
}

int main(int argc, char ** argv){
	char * command = malloc(strlen(argv[1]) + 1);
	strcpy(command, argv[1]);
	if(strcmp(command, "configure") == 0){
		configure(argv[2], argv[3]);
		return 0;
	}
	char * name = malloc(strlen(argv[2]) + 1);
	strcpy(name, argv[2]);
	
	if(strcmp(name, "projects") == 0){
		printf("projects is an invalid project name because the server files are stored in a directory named \"projects\"\n");
		return 0;
	}
	
	if(strcmp(command, "update") == 0){
		char *manifestPath;
		manifestPath = (char*)malloc((strlen(name)+strlen("manifest")+2)*sizeof(char));
		int fileType = regularFileOrDirectory(name);
		if(fileType != 0){
			printf("Invalid project name\n");
			return 1;
		}
		strcpy(manifestPath, name);
		strcat(manifestPath, "/.Manifest");
		updateCommand(name, manifestPath);
		return 0;
	}else if(strcmp(command, "remove") == 0 || strcmp(command, "add") == 0){
		char *manifestPath;
		char *filePath;
		manifestPath = (char*)malloc((strlen(name)+strlen("manifest")+2)*sizeof(char));
		filePath = (char*)malloc((strlen(name)+strlen(argv[3])+2)*sizeof(char));
		int fileType = regularFileOrDirectory(name);
		if(fileType != 0){
			printf("Invalid project name\n");
			return 1;
		}
		strcpy(filePath, name);
		strcat(filePath, "/");
		strcat(filePath, argv[3]);
		strcpy(manifestPath, name);
		strcat(manifestPath, "/.Manifest");
		if(strcmp(command, "add") == 0){
			addCommand(filePath, argv[3], manifestPath);
		}else if(strcmp(command, "remove") == 0){
			removeCommand(argv[3], manifestPath);
		}
		return 0;
	}
	FILE * fd = fopen("./.configure", "r");
	if(fd == NULL){
		printf("please run configure first\n");
		return 0;
	}
	char ip[25];
	int port;
	fscanf(fd, "%s\t%d", ip, &port);
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	//addr.sin_addr.s_addr = inet_addr(ip);

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
	}else if(strcmp(command, "rollback") == 0){
		rollback(name, socketfd, argv[3]);
	}else{
		send(socketfd, "invalid", 50, 0);
		printf("Invalid command given.\n");
	}
	return 0;
}