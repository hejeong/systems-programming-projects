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
char* strsplit(char* str);

// remove leading and trailling whitepspaces
char* strip(char *string);
	
head* sort( head* root, int size, int position);