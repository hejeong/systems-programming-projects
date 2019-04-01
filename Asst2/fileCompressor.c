#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "header.h"

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

void printTokens(struct node* head){
	struct node* current = head;
	while(current != NULL){
		printf("[%s]  -  %d\n", current->token, current->freq);
		current = current->next;
	}
	return;
}

struct node* addToken(char* token, struct node* head){
	struct node* current = head;
	if(head != NULL){
		while(current != NULL){
			if(strcmp(current->token, token) == 0){
				current->freq += 1;
				return head;
			}
			if(current->next != NULL){
				current = current->next;
			}else{
				break;
			}
		}
	}
	// if we reach this point, token doesn't exist, and so we create a new node and append to end
	struct node * newNode = (struct node *) malloc(sizeof(struct node));
	newNode->token = malloc((strlen(token) + 1)*sizeof(char));
	strcpy(newNode->token, token);
	newNode->freq = 1;
	newNode->next = NULL;
	if(head == NULL){
		head = newNode;
	}else {
		current->next = newNode;
	}
	return head;
}

struct node* tokenize(char* string, int totalBytes, struct node* head){
	if(totalBytes == 0){
		return head;
	}
	char * str = (char *)malloc((strlen(string)+1)*sizeof(char));
	str[strlen(string)] = '\0';
	char * top = str;
	strcpy(str, string);
	char *nextString;
	char *startToken = str;
	while(totalBytes >= 0){
		if(totalBytes == 0){
			head = addToken(startToken, head);
			break;
		}
		if((*str >= 7 && *str <= 13) || (*str == 26) || (*str == 27) || (*str == 0) || (*str == ' ')){
			char special[2] = "\0";
			special[0] = *str;
			nextString = str + 1;
			*str = '\0';
			if(strlen(startToken) != 0){
				head = addToken(startToken, head);
			}
			head = addToken(special, head);
			str = nextString;
			startToken = nextString;
			totalBytes--;
			if(totalBytes == 0){
				break;
			}
			continue;
		}
		totalBytes--;
		str++;
	}
	free(top);
	return head;
}
// parameter: file path
struct node * tokenizeFile(char * path, struct node* head){
	struct node* top = head;
	int fileBytes = getFileSizeInBytes(path);
	printf("   >> File Size: %d bytes\n", fileBytes);
	int fileDesc = open(path, O_RDONLY);
	char* stream = malloc((fileBytes+1)*sizeof(char));
	read(fileDesc, stream, fileBytes);
	stream[fileBytes] = '\0';
	top = tokenize(stream, fileBytes,top);
	free(stream);
	int closeStatus = close(fileDesc);
	return top;
}

// parameter: current path
struct node* traverse(char* currentDir, struct node* head, char action, char * huffmanCodebookPath){
	DIR *dir;
	struct dirent *dent;
	char buffer[50];
	char *path;
	struct node* top = head;
	
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
		strcat(path, dent->d_name);
		
		// check the file type [directory, regular file, neither]
		int typeInt = regularFileOrDirectory(path);
		char *fileType;
		if(typeInt == 0){
			fileType = "Directory";
			printf("%s\n   >> %s\n", path, fileType); 
			// append a '/' character to the end of current path, then traverse into nested directories
			char* newPath = (char*)malloc((strlen(path)+2)*sizeof(char));
			strcpy(newPath, path);
			strcat(newPath, "/");
			top = traverse(newPath, top, action, huffmanCodebookPath);
			free(newPath);
			continue;
		}else if(typeInt == 1){
			fileType = "Regular File";
			printf("%s\n   >> %s\n", path, fileType); 
			if(action == 'b'){
				top = tokenizeFile(path, top);
				publish(genBook(top), "\0", 0);
			}else if(action == 'c'){
				compress(path, huffmanCodebookPath);
			}else if(action == 'd'){
				decode(path, huffmanCodebookPath);
			}
			continue;
		}else {
			fileType = "Neither";
		}
		printf("%s\n   >> %s\n", path, fileType); 
		free(path);
	}
	// close the current directory
	closedir(dir);
	return top;
}

int main(int argc, char* argv[]){
	char * fileOrDirPath;
	char * huffmanPath;
	int counter = 1, bFlag = 0, cFlag = 0, dFlag = 0, rFlag = 0;
	// check for b, d, c, and R flags
	while(counter < argc){
		if(strcmp(argv[counter],"-b")==0){
			bFlag = 1;
		}else if(strcmp(argv[counter], "-d") == 0){
			dFlag = 1;
		}else if(strcmp(argv[counter], "-c") == 0){
			cFlag = 1;
		}else if(strcmp(argv[counter], "-R") == 0){
			rFlag = 1;
		}else{
			break;
		}
		counter++;
	}
	if(bFlag+cFlag+dFlag != 1){
		printf("MUST BE AT LEAST AND AT MOST 1 -b, -c, or -d FLAG\n");
		return 0;
	}
	
	// file path always required
	if(counter >= argc){
		printf("NEED FILE PATH\n");
		return 0;
	}else{
		fileOrDirPath = (char*)malloc((strlen(argv[counter])+1)*sizeof(char));
		strcpy(fileOrDirPath, argv[counter]);
		counter++;
	}
	
	if(dFlag+cFlag==1){
		if(counter >= argc){
			printf("NEED HUFFMAN CODEBOOK\n");
			return 0;
		}else{
			huffmanPath = (char*)malloc((strlen(argv[counter])+1)*sizeof(char));
			strcpy(huffmanPath, argv[counter]);
			counter++;
		}
	}else {
		huffmanPath = "";
	}
	if(counter != argc){
		printf("Too many arguments\n");
		return 0;
	}
	
	// b c and d flags are mutually exclusive
	
	
	/* ----- BEGIN OPERATION HERE ----- */
	struct node* head;
	struct stat path_stat;
	stat(fileOrDirPath, &path_stat);
	if(S_ISREG(path_stat.st_mode)){
		if(rFlag == 1){
			printf("Cannot complete recursive operation on file with -R flag\n");
			return 0;
		}
		if(bFlag == 1){
			head = tokenizeFile(fileOrDirPath, head);
			publish(genBook(head), "\0", 0);
		}else if(cFlag == 1){
			compress(fileOrDirPath, huffmanPath);
		}else if(dFlag == 1){
			decode(fileOrDirPath, huffmanPath);
		}
	}else if(S_ISDIR(path_stat.st_mode)){
		if(rFlag != 1){
			printf("Cannot complete operation on directory without -R flag\n");
			return 0;
		}
		char action;
		if(bFlag == 1){
			action = 'b';
		}else if(cFlag == 1){
			action = 'c';
		}else if(dFlag == 1){
			action = 'd';
		}
		head = traverse(fileOrDirPath, head, action, huffmanPath);
	}else{
		printf("Not a valid file or directory path\n");
		return 0;
	}/*
	struct node* head;
	printf("\n-------Open Root Directory-------\n");
	head = traverse(fileOrDirPath, head);
	printf("-------Close Root Directory-------\n\n");
	printTokens(head);*/
	//publish(genBook(head), "\0");
	//compress("./SecondDir/a.txt", "./HuffmanCodebook");
	//decode("./SecondDir/a.txt.hcz", "./HuffmanCodebook");
	/*printf("Command Line Argument: %s\n",argv[1]);
	if(strcmp(argv[1], "-b") == 0){
		printf("I'M GOING TO BUILD A CODEBOOK\n");
	}*/
	free(fileOrDirPath);
	return 1;
}