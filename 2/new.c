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

struct treeNode
{	
	char * token;
	int freq;
	struct treeNode * left;
	struct treeNode * right;
};

struct node
{
	char * token;
	int freq;
	struct node * next;
};

int getFileSizeInBytes(const char* path){
	struct stat fileStat;
	stat(path, &fileStat);
	int size = fileStat.st_size;
	return size;
}

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
	//turns on regular nodes into tree leafs for combining
	struct treeNode ** arr = malloc(count * sizeof(struct treeNode *));
	int i = 0;
	for(i = 0; i < count; i++){
		arr[i] = malloc(sizeof(struct treeNode));
		arr[i] -> token = malloc(strlen(ptr->token));
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
		//checks to see if there are at least two leafs left to combine
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
		
		if(amt < 2){
			return arr[first];
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
		
		struct treeNode * combine = malloc(sizeof(struct treeNode));
		combine->freq = arr[first]->freq + arr[second]->freq;
		combine->left = arr[first];
		combine->right = arr[second];
		arr[first] = combine;
		arr[second] = NULL;
	}
	
	struct treeNode * book = malloc(sizeof(struct treeNode));
	for(i = 0; i < count; i++){
		if(arr[i] != NULL){
			book = arr[i];
		}
	}
	
	return book;
}

int publish(struct node * list, char * code ){
	struct treeNode * book = genBook(list);
	if(book == NULL){
		printf("no book to publish\n");
		return 0;
	}
	int fd = open("./HuffmanCodebook", O_CREAT | O_RDWR | O_APPEND, S_IWUSR | S_IRUSR);
	
	if(strcmp(code, "\0") == 0){
		write(fd, "`\n", 2);
	}
	if(book -> token != NULL){
		write(fd, code, strlen(code));
		write(fd, "\t", 1);
		write(fd, book->token, strlen(book->token));
		write(fd, "\n", 1);
	}else{
		char * left = malloc(strlen(code) + 2);
		char * right = malloc(strlen(code) + 2);
		strcpy(left, code);
		strcpy(right, code);
		strcat(left, "0");
		strcat(right,"1");
		publish(book->left, left);
		publish(book->right, right);
	}
	if(strcmp(code, "\0") == 0){
		write(fd, "\n", 1);
	}
	return 1;
	
}

struct treeNode * genTree(char * bookPath){
	char c;
	char con = 'C';
	struct treeNode * head = malloc(sizeof(struct treeNode));
	struct treeNode * ptr = head;
	int fileBytes = getFileSizeInBytes(bookPath);
	int fileDesc = open(bookPath, O_RDONLY);
	char* stream = malloc((fileBytes+1)*sizeof(char));
	char* token;
	read(fileDesc, stream, fileBytes);
	stream[fileBytes] = '\0';
	int i, size;
	for(i = 2; i < fileBytes; i++)
    {
		c = stream[i];
		switch(con){
			case 'C' :
				if(c == '0'){
					if(ptr->left == NULL){
						struct treeNode * temp = malloc(sizeof(struct treeNode));
						ptr->left = temp;
					}
					ptr = ptr->left;
				}else if(c == '1'){
					if(ptr->right == NULL){
						struct treeNode * temp = malloc(sizeof(struct treeNode));
						ptr->right = temp;
					}
					ptr = ptr->right;
				}else if(c == '\t'){
					int j;
				
					for(j = (i + 1); j < fileBytes; j++){
						if(stream[j] == '\n'){
							break;
						}
						size++;
					}
					token = malloc((size + 1) * sizeof(char));
					strcpy(token,"\0");
					con = 'T';
				}else if(c == '\n'){
					return head;
				}
				break;
			case 'T' :
				if(c == '\n'){
					ptr->token = token;
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

int decode(char * filePath, char * bookPath){
    char *code;
    int i;
	char c;

	int fileBytes = getFileSizeInBytes(filePath);
	int fileDesc = open(filePath, O_RDONLY);
	char* stream = malloc((fileBytes+1)*sizeof(char));
	read(fileDesc, stream, fileBytes);
	stream[fileBytes] = '\0';
	char * fileDest = malloc(strlen(filePath) + 1);
	strcpy(fileDest, filePath);
	fileDest[strlen(fileDest) - 4] = '\0';
	int fd = open(fileDest, O_CREAT | O_RDWR | O_TRUNC | O_APPEND, S_IWUSR | S_IRUSR);
	
    
	struct treeNode * head = malloc(sizeof(struct treeNode));
	head = genTree(bookPath);
	struct treeNode * ptr = head;
    for(i = 0; i < fileBytes; i++)
    {
		c = stream[i];
		if(ptr -> token != NULL){
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
			return 0;
		}
    }
	if(ptr -> token != NULL){
			write(fd, ptr->token, strlen(ptr->token));
	}
	return 0;
}
char * search(char * token, char * code, struct treeNode * ptr){
	if(token == NULL){
		printf("invalid token to search for\n");
		return NULL;
	}
	if(ptr->token != NULL){
		if(strcmp(ptr->token, token) == 0){
			return code;
		}else {
			return NULL;
		}
	}else{
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
		retRight = search(token, right, ptr->right);
		if(retLeft != NULL){
			return retLeft;
		}else{
			return retRight;
		}
		
	}
}

int compress(char * filePath, char * bookPath){
	int fileBytes = getFileSizeInBytes(filePath);
	int fileDesc = open(filePath, O_RDONLY);
	char* str = malloc((fileBytes+1)*sizeof(char));
	read(fileDesc, str, fileBytes);
	str[fileBytes] = '\0';
	struct treeNode * tree = genTree(bookPath);
	char * top = str;
	char *nextString;
	char *startToken = str;
	int i;
	
	char * fileDest = malloc(strlen(filePath)+5);
	strcpy(fileDest, filePath);
	strcat(fileDest, ".hcz");
	
	int fd = open(fileDest, O_CREAT | O_RDWR | O_TRUNC | O_APPEND, S_IWUSR | S_IRUSR);
	int size = strlen(str);
	for(i = 0; i < size; i++){
		printf("%d of %d\n", i, size);
		printf("%s\n", str);
		if((*str >= 7 && *str <= 13) || (*str == 26) || (*str == 27) || (*str == 0) || (*str == ' ')){
			char special[2];
			special[0] = *str;
			special[1] = '\0';
			nextString = str + 1;
			*str = '\0';
			char * token1 = search(startToken, "\0", tree);
			write(fd, token1, strlen(token1));
			char * token2 = search(special, "\0", tree);
			write(fd, token2, strlen(token2));
			str = nextString;
			startToken = nextString;
			continue;
		}
		str++;
	}
	free(top);
	return 0;
}

/*int main(int argc, char* argv[]){
	struct node * node1 = malloc(sizeof(struct node));
	node1->token = "haha";
	node1->freq = 1;
	struct node * node2 = malloc(sizeof(struct node));
	node2->token = "haha2";
	node2->freq = 2;
	struct node * node3 = malloc(sizeof(struct node));
	node3->token = "haha3";
	node3->freq = 3;
	struct node * node4 = malloc(sizeof(struct node));
	node4->token = "haha4";
	node4->freq = 4;
	struct node * node5 = malloc(sizeof(struct node));
	node5->token = "haha5";
	node5->freq = 5;
	struct node * node6 = malloc(sizeof(struct node));
	node6->token = " ";
	node6->freq = 6;
	struct node * node7 = malloc(sizeof(struct node));
	node7->token = "haha7";
	node7->freq = 7;
	
	node1->next = node2;
	node2->next = node5;
	node3->next = node6;
	node4->next = node7;
	node5->next = node3;
	node6->next = node4;
	publish(node1, "\0");
	decode("./testComp.txt.hcz","./HuffmanCodebook");
	
	//printf("\n\n\n\n\n\n");
	
	return 0;
}  */