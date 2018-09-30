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
  node** rows = malloc(sizeof(node *));
  // reads from stdin until end of file
  while(fgets(line, sizeof line, fpointer) != NULL){
    // find first link
    node* head;
    node* prev;
    token = strtok(line, ",");
    head = (node *)malloc(sizeof(node));
    head->value = token;
    head->next = NULL;
    prev = head;
 
 //   printf("Row %d: %s\n",row_count, token);
    while(token) {
      // find next token and add to linked list
      token = strtok(NULL,",");
      node* newNode = (node *)malloc(sizeof(node));
      prev->next = newNode;
      newNode->value = token;
      newNode->next = NULL;
      prev = newNode;
   }
   row_count++;
   node* current = head;
   rows = realloc(rows,sizeof(node *)*row_count);
   rows[row_count-1] = current;
   }
 
   int i;
   for(i = 0; i < row_count; i++){
     printf("Row %d: %s\n", row_count, rows[i]->value); 
   }
free(rows);
   
 return 0;
}
