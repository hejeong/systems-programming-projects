#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simpleCSVsorter.h"
#include "mergesort.c"


int main(int argc, char* argv[]){
  // create file pointer
  FILE* fpointer;
  FILE *fp;
  // file pointer reads from stdin
  fpointer = stdin;
  
  char* val;
  char line[512]; 
  struct node* categories = malloc(sizeof(struct node));
  
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
  val = malloc((strlen(token) + 1) * sizeof(char));
		if(val != NULL)
		{
		strcpy(val, token);
		}
  categories -> value = val;
struct node* temp;
struct node* tempPtr = categories;
  while(token){
    if (strcmp(token, argv[2]) == 0){
       sort_by = count;
    }
	temp = malloc(sizeof(struct node));
	val = malloc((strlen(token) + 1) * sizeof(char));
		if(val != NULL)
		{
		strcpy(val, token);
		}
	temp -> value = val;
	tempPtr -> next = temp;
	tempPtr = tempPtr -> next;
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
  int catCount; //counter to see if this node is the category to sort by
  char* headVal; //remembers value to store in head in the future for easier comparisons
  // reads from stdin until end of file
  while(fgets(line, sizeof line, fpointer) != NULL){
	catCount = 0;
    token = strsplit(line);
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
	
	if(catCount == sort_by)
	{
		headVal = val;
	}
	catCount++;
    while(token) {
     // find next token and add to linked list
	 
     token = strsplit(NULL);
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
	 if(catCount == sort_by)
		{
		headVal = val;
		}
		catCount++;
    } 
    row_count++;

	struct head* ptr = malloc(sizeof(struct head));
	newHead = ptr;
	newHead->next = NULL;
	newHead->row = head_per_row;
	newHead->index = row_count;
	newHead->value = headVal;
	
    if(row_count == 1){
     topRow = newHead;
     prevRow = topRow;
    }else{
      prevRow->next = newHead;
      prevRow = prevRow->next;
		
    } 
 }
 struct head* final = malloc(sizeof(struct head));
 if(final != NULL){
	final = sort(topRow, row_count, sort_by);
 }

 char filename[] = "sorted.csv";
 
fp=fopen(filename,"w+");
while(categories != NULL)
{
	fprintf(fp,"%s,",categories -> value);
	categories = categories -> next;
}
 struct head* rows = final;
 struct node* col;
/*while(rows != NULL)
{
	col = rows -> row;
	while(col != NULL)
	{
		fprintf(fp,"%s,",col -> value);
		col = col -> next;
	}
	rows = rows -> next;
}*/
while(rows != NULL)
{
	fprintf(fp,"%s\n",rows -> value);
	rows = rows -> next;
}
 free(topRow); 
 free(categories);
 return 0;
}
