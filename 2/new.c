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

int publish(struct treeNode * book, char * code ){
	if(book == NULL){
		return 0;
	}
	if(code == "\0"){
		printf("`\n");
	}
	if(book -> token != NULL){
		printf("%s\t", code);
		printf("%s\n",book->token);
	}else{
		char * left = malloc(strlen(code) + 1);
		char * right = malloc(strlen(code) + 1);
		strcpy(left, code);
		strcpy(right, code);
		strcat(left, "0");
		strcat(right,"1");
		publish(book->left, left);
		publish(book->right, right);
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
	
	for(i = 0; i < fileBytes; i++)
    {
		c = stream[i];
		switch(con){
			case 'C' :
				if(c == '0'){
					if(ptr->left = NULL){
						struct treeNode * temp = malloc(sizeof(struct treeNode));
						ptr->left = temp;
					}
					ptr = ptr->left;
				}else if(c == '1'){
					if(ptr->right = NULL){
						struct treeNode * temp = malloc(sizeof(struct treeNode));
						ptr->right = temp;
					}
					ptr = ptr->right;
				}else if(c == '\t'){
					int j;
				
					for(j = i + 1; j < fileBytes; j++){
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
	
    
	struct treeNode * head = malloc(sizeof(struct treeNode));
	head = genTree(bookPath);
	struct treeNode * ptr = head;
    for(i = 0; i < fileBytes; i++)
    {
		c = stream[i];
		if(ptr -> token != NULL){
			printf("%s\n", ptr -> token);
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
	return 0;
}
int search(char * token){
	printf("%s\n", token);
	printf("one token\n");
	return 0;
}

int compress(char * filePath, char * bookPath){
	int fileBytes = getFileSizeInBytes(filePath);
	int fileDesc = open(filePath, O_RDONLY);
	char* str = malloc((fileBytes+1)*sizeof(char));
	read(fileDesc, str, fileBytes);
	str[fileBytes] = '\0';
	
	struct treeNode * tree = genTree(bookPath);
	char * top = str;
	strcpy(str, filePath);
	char *nextString;
	char *startToken = str;
	int i;
	
	for(i = 0; i < strlen(filePath); i++){
		if((*str >= 7 && *str <= 13) || (*str == 26) || (*str == 27) || (*str == 0) || (*str == ' ')){
			char special[2] = "\0";
			special[0] = *str;
			nextString = str + 1;
			*str = '\0';
			search(startToken, tree);
			search(special, tree);
			str = nextString;
			startToken = nextString;
			continue;
		}
		str++;
	}
	free(top);
	return 0;
}

int main(int argc, char* argv[]){
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
	node6->token = "haha6";
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
	publish(genBook(node1), "\0");
	
	printf("\n\n\n\n\n\n");
	
	
	
	
	char * test = "da";
	printf("%s\n", test);
	printf("%d\n", test[2]);
	return 0;
}  