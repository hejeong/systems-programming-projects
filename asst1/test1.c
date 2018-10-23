#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

const char *getExt(char *filename) {
    const char *dot = strrchr(filename, '.');
	printf("found\n");
	if(!dot || dot == filename)
	{
		return "";
	}
    if(dot != NULL)
	{
		printf("good\n");
		return dot + 1;
	}
	printf("bad");
	return NULL;
}

void traverse(char name[100]){
		char path[100];
        DIR* dir;
        struct dirent *ent;
        struct stat states;
		
		if(path == NULL)
		{
			return;
		}
		
		//copies a local copy of the name so the name resets with a new directory
		strcpy(path, name);
        dir = opendir(path);
		
		//checks if its the end of the dir stream
		if(dir == NULL)
		{
			return;
		}
		//reads the dirent of the current file or directory
        while((ent=readdir(dir)) != NULL)
		{
				stat(ent->d_name,&states);
				//checks if its backing out, NOT SURE IF THIS IS NECESSARY, JUST COPIED FROM STACK OVERFLOW
                if(!strcmp(".", ent->d_name) || !strcmp("..", ent->d_name))
				{
						//printf("you loser");
                        continue;
                }
                else{
						strcpy(path,name);
						//if the stream is on a directory, concatenates the names into a single path and calls traverse again
						strcat(path,"/");
                        strcat(path,ent->d_name);
						if((stat(path,&states)) != 0)
						{
							return;
						}
						printf("%s\n", path);
						printf("%d\n",states.st_mode);
						if(strcmp(getExt(ent->d_name),"csv") == 0)
						{
							printf("is file\n");
						}
						else if(S_ISDIR(states.st_mode))
						{
							printf("worked");
                            traverse(path);
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
  printf("cflag = %d ; dflag = %d ; oflag = %d\n", cflag, dflag, oflag);
  printf("column = %s ; inputDir = %s ; outputDir = %s \n", column, inputDir, outputDir); 
  
  //check if there is a path, if there is then call
  traverse(inputDir);
  
  free(inputDir);
  free(outputDir);
  free(column);
  return 0;
}


