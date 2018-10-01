#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simpleCSVsorter.h"


int main(int argc, char* argv[]){
  // create file pointer
  FILE* fpointer;
  // file pointer reads from stdin
  fpointer = stdin;
  
  char line[512]; 
  
  // check for correct number of arguments and -c argument
  if (argc != 3){
    printf("Incorrect number of arguments");
    return -1;
  } else if (strcmp(argv[1], "-c") != 0){
      printf("Missing -c argument.\n");
    return -1;
  }
  fgets(line, sizeof line, fpointer);
  
  char *token = strtok(line, ",");
  // count number of columns starting at 0
  int count = 0;
  // sort_by is the index number that represents which attribute to sort by
  int sort_by = -1;

  while(token){
    if (strcmp(token, argv[2]) == 0){
       sort_by = count;
    }
    token = strtok(NULL, ",");
    count++;
  }
  // if sort_by still equals -1, then attribute given is not valid
  if (sort_by == -1){
    printf("Not a valid attribute.\n");
    return -1;
  }
 
  int row_count = 0;
  // create top node for row
  struct head* topRow;
  struct head* prevRow;
  struct head* newHead;
  struct node* head_per_row;
  struct node* prev;
  struct node* newNode;
  char* val;
  // reads from stdin until end of file
  while(fgets(line, sizeof line, fpointer) != NULL){
    token = strsplit(line, ",");
    head_per_row = (struct node*)malloc(sizeof(struct node));
	val = malloc((strlen(token) + 1) * sizeof(char));
	if(val != NULL)
	{
		strcpy(val, token);
        }
	else
	{
		val = NULL;
	}
    head_per_row->next = NULL;
    head_per_row->value = val;
    prev = head_per_row; 
   // printf("Row %d: %s, %p\n",row_count, (*head_per_row)->value, (*head_per_row)); 
    while(token) {
     // find next token and add to linked list
	 
     token = strsplit(NULL,",");
	 if(token != NULL)
	 { 
     struct node* nextNode = (struct node*)malloc(sizeof(struct node));
	 val = malloc((strlen(token) + 1) * sizeof(char));
		if(val != NULL)
		{
		strcpy(val, token);
		}
		nextNode->next = NULL;
		nextNode->value = val;
		prev->next = nextNode;
		prev = nextNode;
	 }
    } 
    row_count++;
/*    printf("Row %d: ", row_count);
    struct node* current = head_per_row;
    while(current->next != NULL){
      printf("%s ", current->value);
      current = current->next;
    }
    printf("\n"); */
    

    //printf("Row %d: %s\n"  , row_count, newHead->row->value);
	struct head* ptr = malloc(sizeof(struct head));
	newHead = ptr;
	newHead->next = NULL;
	newHead->row = head_per_row;
	newHead->index = row_count;
	
    if(row_count == 1){
     topRow = newHead;
     prevRow = topRow;
    }else{
		//printf("%s", (prevRow -> row) -> next -> value);
      prevRow->next = newHead;
      prevRow = prevRow->next;
		
    } 
 }
 	struct head* haha2 = topRow;
	struct node* haha = topRow -> row;
	
        int cnt = 0;
	while(haha2 != NULL)
	{  cnt++; 
           if(cnt == 275){    
           printf("Row %d:", cnt);
		haha = haha2 -> row;
                while(haha != NULL)
		{       if(haha->next != NULL){
			   printf("%s,", haha -> value);
                        }else{
                           printf("%s", haha->value);
                        }
			haha = haha -> next;
		}}
		//printf("%d\n", haha2 -> index);
                haha2 = haha2->next;
	}

 free(topRow); 
 return 0;
}
