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
#include <unistd.h>


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
				numFiles = numFiles - 1;
				printf("File already exists. Changes were made.\n");
				strcpy(current->hashcode, hashcode);
				return head;
			}else if(strcmp(current->filePath, filePath) == 0 ){
				numFiles = numFiles - 1;
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
			numFiles = numFiles + 1;
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
	fscanf(fp, "%d\t%d\n", &num, &vNum);
	
	int len = strlen(manifestPath);
	char *last_two = &manifestPath[len-2];
	if(strcmp(last_two, "_S") == 0){
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
	struct node * head = NULL;
	head = createManifestList(manifestPath, head);
	char hashcode[2*SHA256_DIGEST_LENGTH];
	strcpy(hashcode, readFileAndHash(filePath, hashcode));
	head = addFileToList(1,filePathWithoutProj,hashcode, head, NULL);
	writeToManifest(manifestPath, head, 1);
}

void removeCommand(char * filePathWithoutProj, char* manifestPath){
	// get linked list of manifest file
	struct node * head = NULL;
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
					// do i add this here?
					found = 1;
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



int create(char * name, int sock){
	
	send(sock, "create", 50, 0);
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
	while( (remaining > 0) && ((written = recv(sock, buffer, size, 0)) > 0) ){
		write(fd, buffer, written);
		remaining = remaining - written;
	}
	close(fd);
	return 0;
}

int destroy(char * name, int sock){
	send(sock, "destroy", 50, 0);
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
	strcpy(test, "\0");
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

void updateCommand(char * project, char* clientManifestPath, int sock){
	send(sock , "update", 50, 0);
	sleep(1);
	send(sock, project, 2000, 0);
	char buffer[1000];
	recv(sock, buffer, 1000, 0);
	int size = atoi(buffer);
	char * manifest = malloc(14 + strlen(project));
	strcpy(manifest, project);
	strcat(manifest, "/.Manifest_S");
	int fd = open(manifest, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
	
	int remaining = size;
	int written;
	while( (remaining > 0) && ((written = recv(sock, buffer, 1000, 0)) > 0) ){
		write(fd, buffer, written);
		remaining = remaining - written;
	}
	close(fd);
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

struct node * updateManifest(int version, char * filePath, char * hashcode, struct node * head){
	struct node * current = head;
	while(current != NULL){
		if(strcmp(current->filePath, filePath) == 0){
			current->version = version;
			strcpy(current->hashcode, hashcode);
			return head;
		}
		if(current->next != NULL){
			current = current->next;
		}else{
			break;
		}
	}
	return head;
}

void fetchFilesAndWrite(char * project, char * filePath, int sock){
	send(sock, "upgrade", 50, 0);
	sleep(1);
	send(sock, project, 2000, 0);
	sleep(1);
	send(sock, filePath, 1000, 0);
	char buffer[1000];
	recv(sock, buffer, 1000, 0);
	int size = atoi(buffer);
	char * fullFilePath = malloc((strlen(project) + strlen(filePath) + 2)*sizeof(char));
	strcpy(fullFilePath, project);
	strcat(fullFilePath, "/");
	strcat(fullFilePath, filePath);
	int fd = open(fullFilePath, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
	int remaining = size;
	int written;
	while( (remaining > 0) && ((written = recv(sock, buffer, 1000, 0)) > 0) ){
		write(fd, buffer, written);
		remaining = remaining - written;
	}
	close(fd);
	return;
}

void upgrade(char* project, char* clientManifestPath, int sock){
	char* updatePath = malloc((strlen(project) + strlen("/.Update") + 1)*sizeof(char));
	strcpy(updatePath, project);
	strcat(updatePath, "/.Update");
	
	FILE *fp;
	char UMAD[2];
	int version, num, vNum;
	char filePath[256];
	char hashcode[256];
	fp = fopen(updatePath , "r");	
	if(fp == NULL) {
     	  perror("No update file");
     	  return;
  	}
	struct node* c_head;
	c_head = createManifestList(clientManifestPath, c_head);
	fscanf(fp, "%d\t%d", &num, &vNum);
	numFiles = num;
	versionNum= vNum;
	while(fscanf(fp, "%s %d %s %s", UMAD, &version, filePath, hashcode) != EOF){
		if(strcmp(UMAD, "U") == 0){
				// what do
		}else if(strcmp(UMAD, "M") == 0){
			c_head = updateManifest(version, filePath, hashcode, c_head);
			fetchFilesAndWrite(project, filePath, sock);
		}else if(strcmp(UMAD, "A") == 0){
			c_head = updateManifest(version, filePath, hashcode, c_head);
			fetchFilesAndWrite(project, filePath, sock);
		}else if(strcmp(UMAD, "D") == 0){
			c_head = removeFileFromList(filePath, c_head);
		}
		printf("close \n");
	}
	writeToManifest(clientManifestPath, c_head, 0);
	fclose(fp);
	return;
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
		send(sock, "invalid", 50, 0);
		return 0;
	}
	send(sock, "checkout", 50, 0);
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
		char * file = malloc(strlen(projDir) + strlen(directory) + 4);
		strcpy(file, "./\0");
		strcat(file, projDir);
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
		char * inc = malloc(size + 1);
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
	int ver = atoi(version);
	if(ver < 1){
		printf("Invalid version number\n");
		send(sock, "invalid", 50, 0);
		return 0;
	}
	send(sock, "rollback", 50, 0);
	send(sock, name, 2000, 0);
	send(sock, version, 100, 0);
	char buffer[30];
	recv(sock, buffer, 30, 0);
	if(strcmp(buffer, "error") == 0){
		printf("This project does not exist on the server\n");
	}else if(strcmp(buffer,"success") == 0){
		printf("Successfully rolled back\n");
	}else{
		printf("Version number is higher than existing version, no changes have been made\n");
	}
	return 0;
}

int configure(char * ip, char * port){
	int fd = open("./.configure", O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
	write(fd, ip, strlen(ip));
	write(fd, "\t", 1);
	write(fd, port, strlen(port));
}

int createManifestS(char * projDir, int sock, int size){
	char * manifestS = malloc(14 + strlen(projDir));
	strcpy(manifestS, projDir);
	strcat(manifestS, "/.ManifestS");
	int fd = open(manifestS, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
	char * incoming = malloc(size + 1);
	int remaining = size;
	int written;
	while( (remaining > 0) && ((written = recv(sock, incoming, size, 0)) > 0) ){
		printf("received %d\n", written);
		write(fd, incoming, written);
		remaining = remaining - written;
	}
	close(fd);
	return 0;
}

struct node * rehash(struct node * cHead, char * projDir){
	struct node * ptr = cHead;
	while(ptr != NULL){
		char * path = malloc(strlen(projDir) + strlen(ptr->filePath) + 3);
		strcpy(path, projDir);
		strcat(path, "/\0");
		strcat(path, ptr->filePath);
		char hashcode[2*SHA256_DIGEST_LENGTH];
		strcpy(hashcode,readFileAndHash(path, hashcode));
		if(strcmp(ptr->hashcode, hashcode) != 0){
			ptr->version = ptr->version + 1;
			strcpy(ptr->hashcode, hashcode);
		}
		ptr = ptr->next;
	}
	return cHead;
}

int compCommit(struct node * cHead, struct node * sHead, char * projDir){
	char * path = malloc(strlen(projDir) + 10);
	strcpy(path, projDir);
	strcat(path, "/.commit");
	struct node * cptr = rehash(cHead, projDir);
	struct node * sptr = sHead;
	FILE * fd = fopen(path, "w");
	if(fd == NULL){
		printf("Cannot write commit file\n");
		return 0;
	}
	struct node * cprev = cptr;
	struct node * sprev = sptr;
	while(cptr != NULL && sptr != NULL){
		while(sptr != NULL){
			if(strcmp(cptr->filePath, sptr->filePath) == 0){
				if(strcmp(cptr->hashcode, sptr->hashcode) != 0){
					if(cptr->version > sptr->version){
						fprintf(fd, "U\t%d\t%s\t%s\n", cptr->version, cptr->filePath, cptr->hashcode);
					}else{
						printf("Please sync with the repository first\n");
						fclose(fd);
						remove(path);
						return 0;
					}
				}
				if(cptr == cHead){
					cHead = cptr->next;
				}else{
					cprev->next = cptr->next;
				}
				if(sptr == sHead){
					sHead = sptr->next;
				}else{
					sprev->next = sptr->next;
				}
				break;
			}
			sprev = sptr;
			sptr = sptr->next;
		}
		sptr = sHead;
		sprev = sptr;
		cptr = cptr->next;
	}
	while(cHead != NULL){
		fprintf(fd, "A\t%d\t%s\t%s\n", cHead->version, cHead->filePath, cHead->hashcode);
		cHead = cHead->next;
	}
	while(sHead != NULL){
		fprintf(fd, "D\t%d\t%s\t%s\n", sHead->version, sHead->filePath, sHead->hashcode);
		sHead = sHead->next;
	}
	fclose(fd);
	return 1;
}

int commit(char * name, int sock){
	char * projDir = malloc(3 + strlen(name));
	strcpy(projDir, "./\0");
	strcat(projDir, name);
	
	struct stat st = {0};
	if (stat(projDir, &st) == -1) {
		printf("Project does not exist in local, nothing to commit\n");
		send(sock, "invalid", 50, 0);
		return 0;
	}
	
	char * update = malloc(10 + strlen(projDir));
	strcpy(update, projDir);
	strcat(update, "/.Update");
	st = {0};
	if (stat(update, &st) != -1) {
		printf("Cannot commit with an active update file\n");
		send(sock, "invalid", 50, 0);
		return 0;
	}
	
	send(sock, "commit", 50, 0);
	send(sock, name, 2000, 0);
	char sizeStr[100];
	recv(sock, sizeStr, 100, 0);
	if(strcmp(sizeStr, "error") == 0){
		printf("This project does not exist on the server\n");
		return 0;
	}
	
	int size = atoi(sizeStr);
	
	if(size == 0){
		printf("Could not get manifest for this project\n");
		return 0;
	}
	printf("manifest is this big %d\n", size);
	createManifestS(projDir, sock, size);
	
	FILE * cfd;
	char * clientMan = malloc(14 + strlen(projDir));
	strcpy(clientMan, projDir);
	strcat(clientMan, "/.Manifest");
	FILE * sfd;
	char * serverMan = malloc(14 + strlen(projDir));
	strcpy(serverMan, projDir);
	strcat(serverMan, "/.ManifestS");
	
	cfd = fopen(clientMan, "r");
	sfd = fopen(serverMan, "r");
	
	int sVer, cVer;
	
	fscanf(cfd, "%*d\t%d", &cVer);
	fscanf(sfd, "%*d\t%d", &sVer);
	if(cVer != sVer){
		printf("Please update local repository\n");
		return 0;
	}
	fclose(cfd);
	fclose(sfd);
	struct node * cHead = NULL;
	struct node * sHead = NULL;
	
	cHead = createManifestList(clientMan, cHead);
	sHead = createManifestList(serverMan, sHead);
	
	if(compCommit(cHead, sHead, projDir) == 0){
		send(sock, "error", 100, 0);
		return 0;
	}
	
	char * path = malloc(strlen(projDir) + 10);
	strcpy(path, projDir);
	strcat(path, "/.commit\0");
	int fd = open(path, O_RDONLY);
	struct stat fileStat;
	char fileSize[100];
	fstat(fd, &fileStat);
	sprintf(fileSize, "%d", fileStat.st_size);
	send(sock, fileSize, 100, 0); 
	int sent;
	int remaining = fileStat.st_size;
	while( (remaining > 0) && ((sent = sendfile(sock, fd, NULL, 1000)) > 0) ){
		remaining = remaining - sent;
	}
}

char * updateManifestS(char * projDir){
	char * commit = malloc(strlen(projDir) + 15);
	char * manifest = malloc(strlen(projDir) + 15);
	
	strcpy(commit, projDir);
	strcat(commit, "/.commit\0");
	strcpy(manifest, projDir);
	strcat(manifest, "/.ManifestS\0");
	
	struct node * head = NULL;
	
	head = createManifestList(manifest, head);
	
	int file = open(commit, O_RDONLY);
	struct stat fileStat;
	char fileSize[100];
	fstat(file, &fileStat);
	int size = fileStat.st_size;
	close(file);
	
	FILE * fd = fopen(commit, "r");
	char * line = malloc(size + 1);
	char * command = malloc(2);
	char * hash = malloc(2*SHA256_DIGEST_LENGTH + 1);
	char * filePath = malloc(size + 1);
	char * version = malloc(10);
	
	while(fgets(line, size, fd) != NULL){
		printf("read %s\ntotal file is this big %d\n", line, size);
		sscanf(line, "%s\t%s\t%s\t%s", command, version, filePath, hash);
		if(strcmp(command, "D") == 0){
			numFiles = numFiles - 1;
			head = removeFileFromList(filePath, head);
			continue;
		}
		if(strcmp(command, "A") == 0){
			head = addFileToList(atoi(version), filePath, hash, head, NULL);
			numFiles = numFiles + 1;
			continue;
		}
		
		struct node * ptr = head;
		while(ptr != NULL){
			if(strcmp(filePath, ptr->filePath) == 0){
				ptr->version = ptr->version + 1;
				ptr->hashcode = hash;
				break;
			}
			ptr = ptr->next;
		}
	}
	versionNum = versionNum + 1;
	writeToManifest(manifest, head, 0);
	return manifest;
}

int push(char * name, int sock){
	char * projDir = malloc(3 + strlen(name));
	strcpy(projDir, "./\0");
	strcat(projDir, name);
	
	struct stat st1 = {0};
	if (stat(projDir, &st1) == -1) {
		printf("Project does not exist in local, nothing to push\n");
		send(sock, "invalid", 50, 0);
		return 0;
	}
	
	char * commitDir = malloc(12 + strlen(projDir));
	strcpy(commitDir, projDir);
	strcat(commitDir, "/.commit\0");
	 
	struct stat st2 = {0};
	if (stat(commitDir, &st2) == -1) {
		printf("Commit does not exist, please commit first before pushing\n");
		send(sock, "invalid", 50, 0);
		return 0;
	}
	
	send(sock, "push", 50, 0);
	send(sock, name, 2000, 0);
	char buffer[50];
	recv(sock, buffer, 50, 0);
	printf("received %s\n", buffer);
	if(strcmp(buffer, "send") == 0){
		char hashcode[2*SHA256_DIGEST_LENGTH];
		strcpy(hashcode,readFileAndHash(commitDir, hashcode));
		printf("sending %s\n", hashcode);
		send(sock, hashcode, 256, 0);
	}else if(strcmp(buffer, "dont") == 0){
		printf("Please commit to server first\n");
		return 0;
	}else{
		printf("Project does not exist on the server\n");
		return 0;
	}
	
	recv(sock, buffer, 50, 0);
	if(strcmp(buffer, "no match") == 0){
		printf("The current commit does not exist on the server, please commit first\n");
		return 0;
	}else if(strcmp(buffer, "success") == 0){
		printf("got success\n");
		char * path = malloc(strlen(projDir) + 10);
		strcpy(path, projDir);
		strcat(path, "/.commit\0");
		
		int file = open(path, O_RDONLY);
		struct stat fileStat;
		char fileSize[100];
		fstat(file, &fileStat);
		sprintf(fileSize, "%d", fileStat.st_size);
		send(sock, fileSize, 100, 0);
		int size = fileStat.st_size;
		close(file);
		
		FILE * fd = fopen(path, "r");
		char * line = malloc(size + 1);
		char * command = malloc(2);
		char * hash = malloc(2*SHA256_DIGEST_LENGTH + 1);
		char * filePath = malloc(size + 1);
		char * version = malloc(10);
		while(fgets(line, size, fd) != NULL){
			printf("read %s\ntotal file is this big %d\n", line, size);
			sscanf(line, "%s\t%s\t%s\t%s", command, version, filePath, hash);
			
			send(sock, command, 2, 0);
			send(sock, version, 10, 0);
			send(sock, filePath, size, 0);
			send(sock, hash, 2*SHA256_DIGEST_LENGTH, 0);
			if(strcmp(command, "D") != 0){
				char * fullPath = malloc(strlen(projDir) + strlen(filePath) + 3);
				strcpy(fullPath, projDir);
				strcat(fullPath, "/\0");
				strcat(fullPath, filePath);
				printf("sending %s\n", fullPath);
				file = open(fullPath, O_RDONLY);
				struct stat fStat;
				char indSize[100];
				fstat(file, &fStat);
				sprintf(indSize, "%d", fStat.st_size);
				send(sock, indSize, 100, 0);
				int iSize = fStat.st_size;
				
				int sent;
				int remaining = size;
				while( (remaining > 0) && ((sent = sendfile(sock, file, NULL, iSize)) > 0) ){
					remaining = remaining - sent;
				}
				close(file);
			}
		}
		close(file);
		fclose(fd);
		send(sock, "M", 2, 0);
		send(sock, "0", 10, 0);
		send(sock, "0", size, 0);
		send(sock, "0", 2*SHA256_DIGEST_LENGTH, 0);
		char * manifestS = updateManifestS(projDir);
		int mfd = open(manifestS, O_RDONLY);
		struct stat mfStat;
		char mSize[50];
		fstat(mfd, &mfStat);
		sprintf(mSize, "%d", mfStat.st_size);
		
		int n = send(sock, mSize, 50, 0);
		int manifestSize = mfStat.st_size;
		printf("%d     manifest is this big %s\n", n, mSize);
		
		int num;
		int left = manifestSize;
		while( (left > 0) && ((num = sendfile(sock, mfd, NULL, manifestSize)) > 0) ){
			printf("sent %d\n", num);
			left = left - num;
		}
		close(mfd);
		send(sock, "Z", 2, 0);
		char * originalManifest = malloc(strlen(projDir) + 10);
		strcpy(originalManifest, projDir);
		strcat(originalManifest, "/.Manifest");
		FILE * fdms = fopen(manifestS, "r");
		FILE * fdm = fopen(originalManifest, "w");
		char c;
		while((c = fgetc(fdms)) != EOF){
			fwrite(&c, 1, 1, fdm);
		}
		fclose(fdms);
		fclose(fdm);
	}
}

int currentVersion(char * name, int sock){
	send(sock, "currentVersion", 50, 0);
	send(sock, name, 2000, 0);
	char sizeStr[10];
	recv(sock, sizeStr, 50, 0);
	int size = atoi(sizeStr);
	if(size == 0){
		printf("Project does not exist on the server\n");
		return 0;
	}
	while(1){
		char inc[2];
		recv(sock, inc, 2, 0);
		if(strcmp(inc, "R") == 0){
			char * line = malloc(size + 1);
			char * version = malloc(10);
			char * filePath = malloc(size);
			recv(sock, line, size, 0);
			sscanf(line, "%s\t%s\t%*s", version, filePath);
			printf("File: %s ----------- Version: %s\n", filePath, version);
		}else{
			break;
		}
	}
	
}

int main(int argc, char ** argv){
	if(!(argc >= 2)){
		printf("Please enter a command\n");
		return 0;
	}
	char * command = malloc(strlen(argv[1]) + 1);
	strcpy(command, argv[1]);
	if(strcmp(command, "configure") == 0){
		if(argc != 4){
			printf("Invalid number of arguments for configure\n");
			return 0;
		}
		configure(argv[2], argv[3]);
		return 0;
	}
	
	if(!(argc >= 3)){
		printf("Please enter a project name\n");
		return 0;
	}
	char * name = malloc(strlen(argv[2]) + 1);
	strcpy(name, argv[2]);
	
	if(strcmp(name, "projects") == 0){
		printf("projects is an invalid project name because the server files are stored in a directory named \"projects\"\n");
		return 0;
	}
	
	if(strcmp(command, "remove") == 0 || strcmp(command, "add") == 0){
		if(argc != 4){
			printf("Invalid number of arguments for %s\n", command);
			return 0;
		}
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
		if(!(argc >= 4)){
			printf("Invalid number of arguments for rollback\n");
			send(socketfd, "invalid", 50, 0);
			return 0;
		}
		rollback(name, socketfd, argv[3]);
	}else if(strcmp(command, "commit") == 0){
		commit(name, socketfd);
	}else if(strcmp(command, "currentversion") == 0){
		currentVersion(name, socketfd);
	}else if(strcmp(command, "push") == 0){
		push(name, socketfd);
	}else if(strcmp(command, "update") == 0){
		char *manifestPath;
		manifestPath = (char*)malloc((strlen(name)+strlen("manifest")+2)*sizeof(char));
		int fileType = regularFileOrDirectory(name);
		if(fileType != 0){
			printf("Invalid project name\n");
			return 1;
		}
		strcpy(manifestPath, name);
		strcat(manifestPath, "/.Manifest");
		updateCommand(name, manifestPath, socketfd);
		return 0;
	}else if(strcmp(command, "upgrade") == 0){
		char *manifestPath;
		manifestPath = (char*)malloc((strlen(name)+strlen("manifest")+2)*sizeof(char));
		int fileType = regularFileOrDirectory(name);
		if(fileType != 0){
			printf("Invalid project name\n");
			return 1;
		}
		strcpy(manifestPath, name);
		strcat(manifestPath, "/.Manifest");
		upgrade(name, manifestPath, socketfd);
		return 0;
	}else{
		send(socketfd, "invalid", 50, 0);
		printf("Invalid command given.\n");
	}
	return 0;
}