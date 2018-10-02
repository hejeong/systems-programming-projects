struct head
{
	struct node* row;
	int index;
	struct head* next;
	char* value;
};

struct node 
{
	struct node* next;
	int pos;
	char* value;
};

struct node* createNode(){
 struct node* newNode = (struct node*)malloc(sizeof(struct node));

 newNode->next = NULL;
 return newNode;
}

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
 comma = strpbrk(src, ",");
  if((comma) != NULL){
     quote1 = strpbrk(src, "\"");
     if(quote1 != NULL && quote1 < comma){
       		quote2 = strpbrk(comma, "\"");
      	 	comma = strpbrk(quote2, ",");
     }
    *comma = 0;
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
