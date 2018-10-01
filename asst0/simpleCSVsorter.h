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
char* strsplit(char* str, char const *delim){
  static char * src = NULL;
  char * p, * ret = 0;

  if(str != NULL){
    src = str;
  }

  if (src == NULL){
    return NULL;
  }  
  if((p=strpbrk(src, delim)) != NULL){
    *p = 0;
    ret = src;
    src = ++p;
  } else if (*src){
    ret = src;
    src = NULL;
  }
  return ret;
}

// remove leading and trailling whitepspaces
char* strip(char *string){
  
  char* end;
  end = string + (strlen(string) - 1) * sizeof(char); 
  //increment until no whitespace
  while(isspace((unsigned char)*string) || *string == '\"'){
     string++;
  }

  //decrement until no whitespace
  while ((isspace((unsigned char)*end) || *end == '\"') && end > string) {
     end = end - sizeof(char);
  }
  //add on null termination character
  *(end+1) = '\0';
  return string;
}
