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
  // reads from stdin until end of file
  while(fgets(line, sizeof line, fpointer) != NULL){
    token = strtok(line, ",");
    struct node* head_per_row;
    struct node* prev;
    struct node* newNode = (struct node*)malloc(sizeof(struct node));
    newNode->next = NULL;
    newNode->value = token;
    head_per_row = newNode;
    prev = head_per_row; 
   // printf("Row %d: %s, %p\n",row_count, (*head_per_row)->value, (*head_per_row)); 
    while(token) {
      // find next token and add to linked list
      token = strtok(NULL,",");
      struct node* nextNode = (struct node*)malloc(sizeof(struct node));
      nextNode->next = NULL;
      nextNode->value = token;
      prev->next = nextNode;
      prev = nextNode; 
    } 
    row_count++;
/*    printf("Row %d: ", row_count);
    struct node* current = head_per_row;
    while(current->next != NULL){
      printf("%s ", current->value);
      current = current->next;
    }
    printf("\n"); */
    
    struct head* newHead = (struct head*)malloc(sizeof(struct head));
    newHead->next = NULL;
    newHead->row = head_per_row;
    //printf("Row %d: %s\n"  , row_count, newHead->row->value);
    if(row_count == 1){
     topRow = newHead;
     prevRow = newHead; 
    }else{
      prevRow->next = newHead;
      prevRow = prevRow->next;
    } 
 }



 free(topRow); 
 return 0;
}
