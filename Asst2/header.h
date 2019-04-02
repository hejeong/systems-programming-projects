#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
//struct for avl tree
struct treeNode
{	
	char * token;
	int freq;
	struct treeNode * left;
	struct treeNode * right;
};
//struct for initial tokens
struct node
{
	char * token;
	int freq;
	struct node * next;
};
//gets file size to know how much to read
int getFileSizeInBytes(const char* path){
	struct stat fileStat;
	stat(path, &fileStat);
	int size = fileStat.st_size;
	return size;
}
//creates avl tree based on linked list of tokens
struct treeNode * genBook (struct node * list){
	
	if(list == NULL){
		return NULL;
	}
	
	struct node * ptr = list;
	int count = 0;
	//counts how many nodes there are in the list
	while(ptr != NULL){
		count++;
		ptr = ptr-> next;
	}
	ptr = list;
	//converts regular nodes into tree leafs for combining
	struct treeNode ** arr = malloc(count * sizeof(struct treeNode *));
	int i = 0;
	for(i = 0; i < count; i++){
		arr[i] = malloc(sizeof(struct treeNode));
		arr[i] -> token = malloc(strlen(ptr->token));
		arr[i] -> left = NULL;
		arr[i] -> right = NULL;
		strcpy(arr[i] -> token, ptr -> token);
		arr[i] -> freq = ptr -> freq;
		ptr = ptr -> next;
	}
	if(count == 1){
		return arr[0];
	}
	//combines the individual leafs into a single tree
	while(1 == 1){
		int amt = 0;
		int first = -1;
		int second = -1;
		//finds initial starting point for "pointers" to look for two lowest nodes
		for(i = 0; i < count; i++){
			if(arr[i] != NULL){
				amt++;
				if(first == -1){
					first = i;
				} else if(second == -1){
					second = i;
				}
			}
		}
		//checks to see if there are at least two leafs left to combine
		if(amt < 2){
			return arr[first];
			break;
		}
		//finds the two lowest treeNodes
		for(i = 0; i < count; i++){
			if(arr[i] != NULL){
				if(arr[i]->freq <= arr[first]->freq && i != second){
					first = i;
				}
			}
		}
		for(i = 0; i < count; i++){
			if(arr[i] != NULL){
				if(arr[i]->freq <= arr[second]->freq && i != first){
					second = i;
				}
			}
		}
		//creates the parent node to hold the two nodes that were combined
		//and removes the two nodes that were combined from the array
		struct treeNode * combine = malloc(sizeof(struct treeNode));
		combine->token = NULL;
		combine->freq = arr[first]->freq + arr[second]->freq;
		combine->left = arr[first];
		combine->right = arr[second];
		arr[first] = combine;
		arr[second] = NULL;
	}
}
//turns an escape sequence to a printable string with a backtick instead of slash
char * normalize(char * token){
	//makes sure the given token is short enough to be an escape sequence
	if(strlen(token) != 1){
		return token;
	}
	//makes sure the token is actually an escape sequence
	if((*token >= 7 && *token <= 13) || (*token == 26) || (*token == 27) || (*token == 0)){
		char c = *token;
		char * ret = malloc(3);
		ret[0] = '`';
		ret[2] = '\0';
		//switch for all escape sequences
		switch(c){
			case '\a':
				ret[1] = 'a';
				break;
			case '\b':
				ret[1] = 'b';
				break;
			case '\t':
				ret[1] = 't';
				break;
			case '\n':
				ret[1] = 'n';
				break;
			case '\v':
				ret[1] = 'v';
				break;
			case '\f':
				ret[1] = 'f';
				break;
			case '\r':
				ret[1] = 'r';
				break;
			case '\0':
				ret[1] = '0';
				break;
		}
		return ret;
	}
	return token;
}
//outputs the avl tree of the codebook into an actual file
int publish(struct treeNode * book, char * code, int fd){
	//opens the file if no file has been open yet
	if(fd == 0){
		fd = open("./HuffmanCodebook", O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
		if(fd == -1){
			printf("unable to write\n");
			return 0;
		}
	}
	if(book == NULL){
		printf("book is empty\n");
		return 0;
	}
	//checks if its the first line of the book, if it is, prints the backtick
	if(strcmp(code, "\0") == 0){
		write(fd, "`\n", 2);
	}
	//recursively goes down every branch and keeps passing a string that gets built on depending on which path is taken
	//prints out the path in 1s and 0s and the token
	if(book -> token != NULL){ // if publish finds a token, prints it out along with the path
		if(strcmp(code, "\0") == 0){ //edge case if only a single token exists to be written, just puts it as 0
			write(fd, "0", 1);
		}else{
			write(fd, code, strlen(code));
		}
		write(fd, "\t", 1);
		char * token = normalize(book->token); //calls normalize to make sure no escape sequences are written down in the book file
		write(fd, token, strlen(token));
		write(fd, "\n", 1);
	}else{ //the recursive call
		char * left = malloc(strlen(code) + 2);
		char * right = malloc(strlen(code) + 2);
		strcpy(left, code);
		strcpy(right, code);
		strcat(left, "0");
		strcat(right,"1");
		publish(book->left, left, fd);
		publish(book->right, right, fd);
	}
	if(strcmp(code, "\0") == 0){ //ends the codebook file
		write(fd, "\n", 1);
	}
	return 1;
	
}
//generates a treeNode structure from a given book text
struct treeNode * genTree(char * bookPath){
	char c;
	char con = 'C';
	struct treeNode * head = malloc(sizeof(struct treeNode));
	head->left = NULL;
	head->right = NULL;
	head->token = NULL;
	struct treeNode * ptr = head;
	int fileBytes = getFileSizeInBytes(bookPath);
	int fileDesc = open(bookPath, O_RDONLY);
	if(fileDesc == -1){
		printf("invalid Huffman Codebook\n");
		return NULL;
	}
	char* stream = malloc((fileBytes+1)*sizeof(char));
	char* token;
	read(fileDesc, stream, fileBytes);
	stream[fileBytes] = '\0';
	int i, size;
	if(strlen(stream) < 4){
		printf("invalid Huffman Codebook\n");
		return NULL;
	}
	struct treeNode * temp;
	for(i = 2; i < fileBytes; i++) //iterates file through char by char
    {
		c = stream[i];
		switch(con){ //switch statement for switching between "reading the binary path code", "checking for backticks", and "reading the actual token"
			case 'C' : //reads the binary path and creates the path of treeNodes to the destination if one does not exist already
				if(c == '0'){ 
					if(ptr->left == NULL){ //traverses left or creates a path left if no node is available
						temp = malloc(sizeof(struct treeNode));
						temp->token = NULL;
						ptr->left = temp;
					}
					ptr = ptr->left;
				}else if(c == '1'){
					if(ptr->right == NULL){ //traverses right or creates a path left if no node is available
						temp = malloc(sizeof(struct treeNode));
						temp->token = NULL;
						ptr->right = temp;
					}
					ptr = ptr->right;
				}else if(c == '\t'){ //tab marks the end of the binary path
					int j;
				
					for(j = (i + 1); j < fileBytes; j++){
						if(stream[j] == '\n'){
							break;
						}
						size++;
					}
					token = malloc((size + 2) * sizeof(char));
					token[0] = '\0';
					con = 'E';
				}else if(c == '\n'){
					return head;
				}
				break;
			case 'E' : //checks if the token is supposed to be an escape sequence or not
				if(c == '`'){
					i++;
					c = stream[i];
					switch(c){
						case 'a':
							strcat(token, "\a");
							break;
						case 'b':
							strcat(token, "\b");
							break;
						case 't':
							strcat(token, "\t");
							break;
						case 'n':
							strcat(token, "\n");
							break;
						case 'v':
							strcat(token, "\v");
							break;
						case 'f':
							strcat(token, "\f");
							break;
						case 'r':
							strcat(token, "\r");
							break;
						case '0':
							strcat(token, "\0");
							break;
					}
					break;
				}
				con = 'T';
			case 'T' : //reads the token and puts it on the treeNode created from reading the binary path
				if(c == '\n'){
					ptr->token = malloc((strlen(token)+2)*sizeof(char));
					strcpy(ptr->token,token);
					strcat(ptr->token,"\0");
					ptr->left = NULL;
					ptr->right = NULL;
					size = 0;
					ptr = head;
					con = 'C';
					
				}else {
					char ch[2];
					ch[0] = c;
					ch[1] = '\0';
					strcat(token, ch);
				}
		}
    }
	return head;
}
//reads through binary paths and outputs the corresponding token
int decode(char * filePath, char * bookPath){
    char *code;
    int i;
	char c;

	int fileBytes = getFileSizeInBytes(filePath);
	int fileDesc = open(filePath, O_RDONLY);
	char* stream = malloc((fileBytes)*sizeof(char));
	read(fileDesc, stream, fileBytes);
	char * fileDest = malloc(strlen(filePath) + 1);
	strcpy(fileDest, filePath);
	fileDest[strlen(fileDest) - 4] = '\0';
	int fd = open(fileDest, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
	
    stream[fileBytes] = '\0';
	struct treeNode * head = genTree(bookPath);
	if(head == NULL){
		return 0;
	}
	struct treeNode * ptr = head;
    for(i = 0; i < fileBytes; i++)// loops through the file char by char and traverses a pointer down the tree based on if it reads a 1 or a 0
    {
		c = stream[i]; 
		if(ptr -> token != NULL){ //checks if the ptr has reached an existing token, if it did, output it and reset ptr back to the head of the tree
			write(fd, ptr->token, strlen(ptr->token));
			ptr = head;
		}
        if(c == '0'){
			if(ptr->left == NULL){
				printf("invalid code\n");
				return 0;
			}
			ptr = ptr -> left;
		}else if(c == '1'){
			if(ptr->right == NULL){
				printf("invalid code\n");
				return 0;
			}
			ptr = ptr -> right;
		}else{
			printf("invalid code\n");
			return 0;
		}
    }
	if(ptr -> token != NULL){ //adds the last token
			write(fd, ptr->token, strlen(ptr->token));
	}
	return 0;
}
//recursively searches through a tree looking for the given token and returns the binary path to that token
char * search(char * token, char * code, struct treeNode * ptr){
	if(token == NULL){
		printf("invalid token to search for\n");
		return NULL;
	}
	if(ptr == NULL){
		return NULL;
	}
	if(ptr->token != NULL){
		if(strcmp(ptr->token, token) == 0){ //if and only if the node with the correct token is found, returns the recorded binary path
			if(strcmp(code,"\0") == 0) //if the path doesnt exist yet(only a single node in the tree) adds a 0 as the path
			{
				char * ret = malloc(strlen(code) + 2);
				strcpy(ret, code);
				strcat(ret, "0");
				return ret;
			}				
			return code;
		}else { //returns null in all other cases
			return NULL;
		}
	}else{ //recursive call to go down each branch and find if any of the branches return null, every call appends the appropriate binary path code per direction the call traverses in
		char * left = malloc(strlen(code) + 2);
		char * right = malloc(strlen(code) + 2);
		char * retLeft;
		char * retRight;
		char * ret;
		strcpy(left, code);
		strcpy(right, code);
		strcat(left, "0");
		strcat(right,"1");
		retLeft = search(token, left, ptr->left);
		if(retLeft == NULL){
			retRight = search(token, right, ptr->right);
		}else{
			return retLeft;
		}
		return retRight;
	}
}
//reads through and tokenizes a file to replaces tokens with the appropriate code
int compress(char * filePath, char * bookPath){
	int fileBytes = getFileSizeInBytes(filePath);
	int fileDesc = open(filePath, O_RDONLY);
	char* str = malloc((fileBytes)*sizeof(char));
	read(fileDesc, str, fileBytes);
	struct treeNode * tree = genTree(bookPath);
	char * top = str;
	char *nextString;
	char *startToken = str;
	int i;
	if(tree == NULL){
		return 0;
	}
	
	char * fileDest = malloc(strlen(filePath)+5);
	strcpy(fileDest, filePath);
	strcat(fileDest, ".hcz");
	
	int fd = open(fileDest, O_CREAT | O_RDWR | O_TRUNC, S_IWUSR | S_IRUSR);
	if(fileBytes == 0){
		return 0;
	}
	str[fileBytes] = '\0';
	int size = strlen(str);
	for(i = 0; i < size; i++){ //file is read through char by char
		if((*str >= 7 && *str <= 13) || (*str == 26) || (*str == 27) || (*str == 0) || (*str == ' ')){ // almost same code as tokenizer function in fileCompressor.c
			char special[2];
			special[0] = *str;
			special[1] = '\0';
			nextString = str + 1;
			*str = '\0';
			if(strlen(startToken) != 0){ //searches for each token and writes the binary path that is returned
				char * token1 = search(startToken, "\0", tree);
				write(fd, token1, strlen(token1));
			}
			char * token2 = search(special, "\0", tree);
			write(fd, token2, strlen(token2));
			str = nextString;
			startToken = nextString;
			continue;
		}else if(i == (size)-1){
			str++;
			*str = '\0';
			char * token1 = search(startToken, "\0", tree);
			write(fd, token1, strlen(token1));
			break;
		}
		str++;
	}
	return 0;
}