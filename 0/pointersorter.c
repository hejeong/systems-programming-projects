#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

struct node //linked list node
{
	struct node* next;
	char* word;
};

int compare(char* str1, char* str2){ //returns which string should come first
	int i;
	for(i = 0; i < strlen(str1); i++)
	{
		if((str1[i] <= 90 && str2[i] <= 90) || (str1[i] >= 97 && str2[i] >= 97)) //checks if both are uppercase or both are lowercase
		{
			if(str1[i] > str2[i])
			{
				return 1;
			}
			else if(str1[i] < str2[i])
			{
				return 2;
			}
		}
		else if(str1[i] <= 90)
		{
			return 1;
		}
		else
		{
			return 2;
		}
		if((i < strlen(str1) - 1) && i == (strlen(str2) - 1)) //if string 1 is longer than string 2
		{
			return 1;
		}
	}
	if(i < strlen(str2)) //if string 2 is longer than string 1
	{
		return 2;
	}
	return 1;
}

char* tokenize(char* str){
	static char* line;
	char* start = NULL;
	char* end;
	char* token;
	
	if(str != NULL) //makes sure to not overwrite the line
	{
		line = str;
	}
	
	if(line == NULL)
	{
		return line;
	}
	int i = 0;
	for(i = 0; i < strlen(line); i++) //finds first letter
	{
		if(isalpha(line[i]))
		{
			start = &line[i];
			break;
		}
	}
	for(i = i; i < strlen(line); i++) //find last letter
	{
		if(!isalpha(line[i]))
		{
			end = &line[i];
			break;
		}
	}
	if(start != NULL)
	{
		if(end != NULL)
		{
			*end = '\0'; //sets the end of the token
			token = start; //copies token
			line = end + 1; //continues line from after the token
		}
		else if(line != NULL)
		{
			token = start;
			line = NULL;
		}
	}
	else
	{
		return NULL;
	}
	return token;
}

int main(int argc, char* argv[]){
	char* line = malloc(strlen(argv[1]) + 1);  //the full input
	char* token;
	char* word; //holds the current token
	struct node* head = malloc(sizeof(struct node)); //start of the linked list
	if(argc != 2)
	{
		printf("invalid number of arguments");
		return 0;
	}
	
	strcpy(line, argv[1]);
	token = tokenize(line); //calls custom string tokenizer function
	if(token == NULL)
	{
		return 0;
	}
	word = malloc((strlen(token) + 1)); //allocates new string for first word
	if(word != NULL)
	{
		strcpy(word, token);
	}
	head->word = word;
	head->next = NULL;
	struct node* ptr;
	while(token) {
		// find next token and add to linked list
		token = tokenize(NULL);
		if(token != NULL)
		{ 
			struct node* nextNode = malloc(sizeof(struct node));
			word = malloc(strlen(token) + 1); 
			if(word != NULL)
			{
				strcpy(word, token);
			}
			nextNode->word = word;
			nextNode->next = NULL;
			ptr = head;
			//insertion sort
			if(compare(ptr->word,word) == 2)
			{
				head = nextNode;
				head->next = ptr;
			}
			else
			{

				while(ptr->next != NULL)
				{
					if(compare((ptr->next)->word,word) == 2)
					{
						nextNode->next = ptr->next;
						ptr->next = nextNode;
						break;
					}
					ptr = ptr->next;
				}
				if(ptr->next == NULL)
				{
					ptr->next = nextNode;
				}
			}
		}
	}
	ptr = head;
	struct node* prev;
	while(ptr != NULL) //prints out all nodes
	{
		prev = ptr;
		printf("%s\n", ptr->word);
		ptr = ptr->next;
		free(prev->word);
		free(prev);
	}
	return 1;
}

