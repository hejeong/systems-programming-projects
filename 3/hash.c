#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <openssl/sha.h>

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

void printList(struct node * head){
	struct node * current = head;
	
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
	   fprintf(fp, "%d %s %s\n", current->version, current->filePath, current->hashcode);
	   current = current->next;
	}
	fclose(fp);
}

int main(int argc, char** argv){
	char *manifestPath;
	char *filePath;
	manifestPath = (char*)malloc((strlen(argv[1])+strlen("manifest")+2)*sizeof(char));
	filePath = (char*)malloc((strlen(argv[1])+strlen(argv[2])+2)*sizeof(char));
	int fileType = regularFileOrDirectory(argv[1]);
	if(fileType != 0){
	  printf("Invalid project name\n");
	  return 1;
	}
	strcpy(filePath, argv[1]);
	strcat(filePath, "/");
	strcat(filePath, argv[2]);

	strcpy(manifestPath, argv[1]);
	strcat(manifestPath, "/.Manifest");



	// get linked list of manifest file
	struct node * head;
	head = createManifestList(manifestPath, head);

	size_t length;
	int i = 0;
	char buffer[2*SHA256_DIGEST_LENGTH]; 
	char* hashcode;
	char shaConverted[2];
	unsigned char hash[SHA256_DIGEST_LENGTH];
	
	int fileBytes = getFileSizeInBytes(filePath);
	int fileDesc = open(filePath, O_RDONLY);
	if(fileDesc == -1){
 	  perror("Error opening file");
     	  return(-1);
	}
	char* stream = malloc((fileBytes+1)*sizeof(char));
	read(fileDesc, stream, fileBytes);
	stream[fileBytes] = '\0';
	length = strlen(stream);
	SHA256(stream, length, hash);
	strcpy(buffer, "");
	for(i = 0; i < SHA256_DIGEST_LENGTH; i++){
        sprintf(shaConverted, "%02x", hash[i]);
		strcat(buffer, shaConverted);
	}	
	free(stream);
	int closeStatus = close(fileDesc);
	head = addFileToList(1,filePath,buffer, head);
	printList(head);
	writeToManifest(manifestPath, head);

	return 0;
}