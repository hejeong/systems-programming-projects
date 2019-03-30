#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
	return NULL;
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
	node5->token = "haha4";
	node5->freq = 5;
	struct node * node6 = malloc(sizeof(struct node));
	node6->token = "haha4";
	node6->freq = 6;
	struct node * node7 = malloc(sizeof(struct node));
	node7->token = "haha4";
	node7->freq = 7;
	
	node1->next = node2;
	node2->next = node5;
	node3->next = node6;
	node4->next = node7;
	node5->next = node3;
	node6->next = node4;
	genBook(node1);
	
	char * test = malloc(20);
	strcat(test, "\t");
	strcat(test,"t");
	printf("%s", test);
	
	return 0;
}  