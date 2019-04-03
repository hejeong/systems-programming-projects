#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "header.h"


// return 1 if given file path is for a regular file
// return 0 if given file path is for a directory
// return -1 if given file path is for neither a directory or file
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

// add a token to linked list, creating a new node or incrementing freq if exists already
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

// find the next delimiter/escape character if any, and add both the token, and escape character to token linked list
struct node* tokenize(char* string, int totalBytes, struct node* head){
	// return immediately if file is empty: no tokens
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
		// if file does not end in escape code, just add the remaining token if there exists one
		if(totalBytes == 0){
			head = addToken(startToken, head);
			break;
		}
		// check for escape codes
		if((*str >= 7 && *str <= 13) || (*str == 26) || (*str == 27) || (*str == 0) || (*str == ' ')){
			char special[2] = "\0";
			special[0] = *str;
			nextString = str + 1;
			*str = '\0';
			// if token isn't null, add to linked list
			if(strlen(startToken) != 0){
				head = addToken(startToken, head);
			}
			// add escape code to linked list
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
		if(currentDir[strlen(currentDir)-1] != '/'){
			strcat(path, "/");
		}
		strcat(path, dent->d_name);
		printf("path: %s\n", path);
		// check the file type [directory, regular file, neither]
		int typeInt = regularFileOrDirectory(path);
		char *fileType;
		if(typeInt == 0){
			fileType = "Directory";
			// append a '/' character to the end of current path, then traverse into nested directories
			char* newPath = (char*)malloc((strlen(path)+2)*sizeof(char));
			strcpy(newPath, path);
			strcat(newPath, "/");
			// recursively go through nested directories and find tokens
			top = traverse(newPath, top, action, huffmanCodebookPath);
			free(newPath);
			continue;
		}else if(typeInt == 1){
			// tokenize file and add tokens to linked list, keeping track of frequency
			fileType = "Regular File";
			if(action == 'b'){
				// find all tokens
				top = tokenizeFile(path, top);
			}else if(action == 'c'){
				// compress given file with the given huffman codebook
				compress(path, huffmanCodebookPath);
			}else if(action == 'd'){
				// decode given file with the given huffman codebook
				decode(path, huffmanCodebookPath);
			}
			continue;
		}else {
			fileType = "Neither";
		}
		free(path);
	}
	// close the current directory
	if(action == 'b'){
		// after all tokens found, generate codebook
		publish(genBook(top), "\0", 0);
	}
	closedir(dir);
	return top;
}

int main(int argc, char* argv[]){
	char * fileOrDirPath;
	char * huffmanPath;
	int counter = 1, bFlag = 0, cFlag = 0, dFlag = 0, rFlag = 0;
	// check for b, d, c, and R flags
	// stop when reached neither of them
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
	// b c and d flags are mutually exclusive
	if(bFlag+cFlag+dFlag == 0){
		printf("Invalid first flag. Please use -b, -c, -d or -R. \n");
		return 0;
	}else if(bFlag+cFlag+dFlag != 1){
		printf("Must pick at most one of the following flags: -b, -c, or -d \n");
		return 0;
	}
	
	// file path always required
	if(counter >= argc){
		printf("No file or directory path present.\n");
		return 0;
	}else{
		fileOrDirPath = (char*)malloc((strlen(argv[counter])+1)*sizeof(char));
		strcpy(fileOrDirPath, argv[counter]);
		counter++;
	}
	
	// require huffman codebook path if trying to compress or decode
	if(dFlag+cFlag==1){
		if(counter >= argc){
			printf("Huffman Codebook path not present.\n");
			return 0;
		}else{
			huffmanPath = (char*)malloc((strlen(argv[counter])+1)*sizeof(char));
			strcpy(huffmanPath, argv[counter]);
			counter++;
		}
	}else {
		huffmanPath = "";
	}
	// reject extra arguments
	if(counter != argc){
		printf("Too many arguments\n");
		return 0;
	}

	
	
	/* ----- BEGIN OPERATION HERE ----- */
	struct node* head;
	struct stat path_stat;
	stat(fileOrDirPath, &path_stat);
	
	// if regular file, only operate on that single file
	if(S_ISREG(path_stat.st_mode)){
		// supply a warning if given -R flag for a file path and continue operation
		if(rFlag == 1){
			printf("The flag -R should only be used on directories. Proceeding with action for the current file... \n");
		}
		printf("%s\n",fileOrDirPath);
		if(bFlag == 1){
			head = tokenizeFile(fileOrDirPath, head);
			publish(genBook(head), "\0", 0);
		}else if(cFlag == 1){
			compress(fileOrDirPath, huffmanPath);
		}else if(dFlag == 1){
			decode(fileOrDirPath, huffmanPath);
		}
	}else if(S_ISDIR(path_stat.st_mode)){
		// check for -R
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
		// traverse through directories with given method
		head = traverse(fileOrDirPath, head, action, huffmanPath);
	}else{
		// if neither directory or file, fail gracefully
		printf("Not a valid file or directory path\n");
		return 0;
	}
	free(fileOrDirPath);
	return 1;
}