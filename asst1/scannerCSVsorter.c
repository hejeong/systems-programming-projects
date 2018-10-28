/*problems
*****************what if directory ends in .csv? - very difficult have no idea how to proceed because segfaults if check if its a file when its a directory
what if csv is not properly formatted? - done
have to malloc path names - done
****works with malloc path names but bugs out when freed - maybe not important but should still find some time to check
***********have to move everything to header - strange problems when moving without changing code, says sort method does not return a  head instead returns int, maybe super?
print pid - done
have to free everything in sortCSV with loop - done, but cant actually free columns, not sure why
what if a name starts with / - done
check if input directory exists, if it doesnt, use current directory - done
check if output directory exists, if it doesnt, use current directory - done
****what if output directory has two //
makefile shit - done
*/
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stddef.h>
#include <unistd.h>
#include "simpleCSVsorter.h"
int* shared;

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

int sortCSV(char* inputFile, char* columnName, char* outputDir){
	// create file pointer
	FILE* fpointer;
	FILE *fp;
	// file pointer reads from stdin
	fpointer = fopen(inputFile, "r");
	  
	char* val;
	char line[512]; 
	node* categories = malloc(sizeof( node));
	 
	fgets(line, sizeof line, fpointer);
	  
	char *token = strsplit(line);
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
	node* temp;
	node* tempPtr = categories;
	while(token){
		if (strcmp(token, columnName) == 0){
			sort_by = count;
		}
		temp = malloc(sizeof( node));
		val = malloc((strlen(token) + 1) * sizeof(char));
			if(val != NULL)
			{
				strcpy(val, token);
			}
		temp -> value = val;
		tempPtr -> next = temp;
		tempPtr = tempPtr -> next;
		token = strsplit(NULL);
		count++;
	}
	  
	  // if sort_by still equals -1, then attribute given is not valid
	if (sort_by == -1){
		return -1;
	}
	 
	 
	int row_count = 0;
	// create top node for row
	head* topRow;
	head* prevRow;
	head* newHead;
	node* head_per_row;
	node* prev;
	int catCount; //counter to see if this node is the category to sort by
	char* headVal; //remembers value to store in head in the future for easier comparisons
	// reads from stdin until end of file
	while(fgets(line, sizeof line, fpointer) != NULL){
		catCount = 0;
		token = strsplit(line); //calls custom string tokenizer function
		head_per_row = ( node*)malloc(sizeof( node));
		val = malloc((strlen(token) + 1) * sizeof(char)); //copies token into a separate string so it doesnt get overwritten in the future
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
		
		if(catCount == sort_by) //checks if this is the value to sort by and puts it into the head 
		{
			headVal = val;
		}
		catCount++;
		while(token) {
		 // find next token and add to linked list
		 token = strsplit(NULL);
		 if(token != NULL)
		 { 
		  node* nextNode = ( node*)malloc(sizeof( node));
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
		//checks if row has correct number of categories
		if(catCount != count + 1)
		{
			return -1;
		}
		
		row_count++;

		 head* ptr = malloc(sizeof(head));  //creates the head  to hold the row of data
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
	  head* final = malloc(sizeof(head));
	 //sorts the data
	if(final != NULL){
		final = sort(topRow, row_count, sort_by);
	}
	//the file to output
	char* dot = strrchr(inputFile, '.');
	*dot = '\0';
	strcat(inputFile, "-sorted-");
	strcat(inputFile, columnName);
	strcat(inputFile, ".csv");
	char* fileOnly = strrchr(inputFile, '/');
	char* filename = malloc(sizeof(char)*(strlen(fileOnly)+strlen(outputDir)+1));
	strcat(filename, outputDir);
	strcat(filename, fileOnly);

	fp=fopen(filename,"w+"); 
	//prints the categories to the top of the csv
	while(categories != NULL){
		if(categories -> next == NULL)
		{
			fprintf(fp,"%s",categories -> value);
		}
		else 
		{
			fprintf(fp,"%s,",categories -> value);
		}
		categories = categories -> next;
	}
		head* rows = final;
		node* col;
		head* freeRow;
		node* freeCol;
	 //code to iterate through all linked list nodes and print everything into the csv
	while(rows != NULL)
	{
		col = rows -> row;
		while(col != NULL)
		{
			if(col -> next == NULL)
			{
				fprintf(fp,"%s",col -> value);
			}
			else 
			{
				fprintf(fp,"%s,",col -> value);
			}
			col = col -> next;
		}
		freeRow = rows;
		rows = rows -> next;
		free(freeRow);
	}
	 fclose(fpointer);
	 fclose(fp);
	 free(filename);
	 while(categories != NULL)
	 {
		 freeCol = categories;
		 categories = categories -> next;
		 free(freeCol);
	 }
	 free(val);
	 return 0;
}
const char *getExt(char *filename) {
    const char *dot = strrchr(filename, '.');
	if(!dot || dot == filename)
	{
		return "";
	}
    if(dot != NULL)
	{
		return dot + 1;
	}
	return NULL;
}

void traverse(char* name, char* column, char* outputDir){
		char* path;
        DIR* dir;
         struct dirent *ent;
         struct stat states;
		
	//copies a local copy of the name so the name resets with a new directory
        dir = opendir(name);
		
	//checks if its the end of the dir stream
	if(dir == NULL)
	{
		return;
	}
	//reads the dirent of the current file or directory
        while((ent=readdir(dir)) != NULL)
	{
		//checks if its a . or .. directory
            if(!strcmp(".", ent->d_name) || !strcmp("..", ent->d_name))
			{
                continue;
            }
            else{
			path = malloc((strlen(name)+1+1+1+strlen(ent->d_name))*sizeof(char));
			if(path == NULL)
			{
			return;
			}
			strcpy(path,name);
			//concatenates the names into a single path and calls traverse again
			strcat(path,"/");
            strcat(path,ent->d_name);
			if((stat(path,&states)) != 0)
			{
				return;
			}
			//checks if its a csv
			if(strcmp(getExt(ent->d_name),"csv") == 0)
			{
				int pid = fork();
				if(pid == 0){
					printf(" %d,", getpid());
					fflush(stdout);
					*shared = *shared + 1;
					char relPath[100];
					strcat(relPath, "./");
					strcat(relPath, path);
					sortCSV(relPath,column, outputDir);
					exit(0);
				}else if(pid > 0){
					
					wait(NULL);
				}
	//			printf("%s is csv file\n", ent->d_name);
			}
			else if(S_ISDIR(states.st_mode))
			{
				int pid = fork();
				if(pid == 0){
					printf(" %d,", getpid());
					fflush(stdout);
					*shared = *shared + 1;
					traverse(path,column, outputDir);
					exit(0);
				}else if(pid > 0){
					wait(NULL);
				}
				//printf("is dir\n");
            }
            }
        }
        closedir(dir);
}

int main(int argc, char* argv[]){
  
	int i;
	int cflag = 0;
	int dflag = 0;
	int oflag = 0;
	char* column = "";
	char* inputDir = "";
	char* outputDir = ""; 
	shared = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	*shared = *shared + 1;
	if(argc != 7){
		//error: incorrect number of arguments
	}
	for(i = 0; i < argc; i++){
		if(cflag == 0 && strcmp(argv[i], "-c") == 0){
			cflag = 1;
			continue;
		}
		if(cflag == 1 && strlen(column) == 0){
			column = malloc((strlen(argv[i])+1)*sizeof(char));
			strcpy(column, argv[i]);
			continue;
		}
		if(strcmp(argv[i],"-d") == 0){
			dflag = 1;
			continue;
		}
		if(dflag == 1 && strlen(inputDir) == 0){
			inputDir = malloc((strlen(argv[i])+1)*sizeof(char));
			strcpy(inputDir, argv[i]);
			continue;
		}
		if(oflag == 0 && strcmp(argv[i],"-o") == 0){
			oflag = 1;
			continue;
		}
		if(oflag == 1 && strlen(outputDir) == 0){
			outputDir = malloc((strlen(argv[i])+1)*sizeof(char));
			strcpy(outputDir, argv[i]);
			continue;
		}
	}
  
	struct stat st = {0};
	//check if there is a path, if there is then call
	printf("Initial PID: %d\n", getpid());
	printf("PIDs of all children processes: ");
	fflush(stdout);
	if(stat(inputDir, &st) == -1)
	{
		if(stat(outputDir, &st) == -1)
		{
			traverse("./\0", column, "./\0");
		}
		else
		{
			traverse("./\0", column, outputDir);
		}
	}
	else
	{
		if(stat(outputDir, &st) == -1)
		{
			traverse(inputDir, column, "./\0");
		}
		else
		{
			traverse(inputDir, column, outputDir);
		}	
	}
	printf("\nTotal number of processes: %d\n", *shared);
	free(inputDir);
	free(outputDir);
	free(column);
	return 0;
}


