#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
typedef struct nodeCol
{
	int pos;
	struct nodeCol* next;
	char* value;
}node;

typedef struct headRow
{
	struct nodeCol* row;
	struct headRow* next;
	int index;
	char* value; //value for easier comparison and sorting in merge sort
}head;


// tokenizer for string
char* strsplit(char* str){
  static char * src = NULL;
  char * comma;
  char * quote1;
  char * quote2; 
  char * ret=0; 
  if(str != NULL){
    src = str;
  }

  if (src == NULL){
    return NULL;
  }
 // locate the first comma by matching it with src 
 comma = strpbrk(src, ",");
  if((comma) != NULL){
	 // locate the first quotation mark 
     quote1 = strpbrk(src, "\"");
	 // if the quotation mark comes before the comma
	 // look for the closing quotation mark
	 // When the second quotation mark is found,
     //	find the comma after it to end the string
     if(quote1 != NULL && quote1 < comma){
       		quote2 = strpbrk(comma, "\"");
      	 	comma = strpbrk(quote2, ",");
     }
	 // set to null
    *comma = 0;
    // ret is now the beginning of src to the end before comma
	ret = src;
    src = ++comma;
  } else if (*src){
    ret = src;
    src = NULL;
  }
  return ret;
}

// remove leading and trailling whitepspaces
char* strip(char *string){
  
  char* end;
  end = string + (strlen(string) - 1); 
  //increment until no whitespace
  while(isspace((unsigned char)*string) || *string == '\"'){
     string++;
  }

  //decrement until no whitespace
  while ((isspace((unsigned char)*end) || *end == '\"') && end > string) {
     end = end - sizeof(char);
  }
  //add on null termination character
  *(end+2) = '\0';
  return string;
}
