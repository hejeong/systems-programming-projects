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
	int version;
	char filePath[256];
	char hashcode[256];
	fp = fopen(manifestPath, "r");	
	if(fp == NULL) {
     	  perror("Error opening file");
     	  return 0;
  	}
	while(fscanf(fp, "%d %s %s", &version, filePath, hashcode) != EOF){
		head = addFileToList(version, filePath, hashcode, head);
	}
	fclose(fp);
	return head;
}

void writeToManifest(char* manifestPath, struct node * head){
	FILE *fp;
	struct node * current = head;
	fp = fopen(manifestPath , "w");	
	if(fp == NULL) {
     	  perror("Error opening file");
     	  return;
  	}
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

void addCommand(char* filePath, char* manifestPath){
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
	head = addFileToList(1,filePath,hashcode, head);
	writeToManifest(manifestPath, head);
}

void removeCommand(char* filePath, char* manifestPath){
	// get linked list of manifest file
	struct node * head;
	head = createManifestList(manifestPath, head);
	head = removeFileFromList(filePath, head);
	writeToManifest(manifestPath, head);
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
		printf("haha\n");
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
	printf("%s\n", buffer);
	return 0;
}

int main(int argc, char ** argv){
	int port = atoi(argv[1]);
	char * command = malloc(strlen(argv[2]) + 1);
	strcpy(command, argv[2]);
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	//addr.sin_addr.s_addr = inet_addr("1.1.1.1.1");

	int socketfd = socket(AF_INET,SOCK_STREAM, 0);
	if (socketfd <= 0){
		printf("ERROR: Failed to open socket.\n");
	}
	
	char * name = malloc(strlen(argv[3]) + 1);
	strcpy(name, argv[3]);

	/* ---------- Used for Add and Remove etc.----------- */
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
	/* ---------------------------------------------------*/


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
	}else if(strcmp(command, "add") == 0){
		addCommand(filePath, manifestPath);
	}else if(strcmp(command, "remove") == 0){
		removeCommand(filePath, manifestPath);
	}else{
		printf("No command given.\n");
	}

	
	return 0;
}