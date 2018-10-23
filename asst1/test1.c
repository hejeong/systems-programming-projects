#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
  
  int i;
  int cflag = 0;
  int dflag = 0;
  int oflag = 0;
  char* column, inputDir, outputDir; 
  for(i = 0; i < argc; i++){
    if(strcmp(argv[i], "-c") == 0){
      cflag = 1;
      continue;
    }
    if(cflag == 1 && column == NULL){
      column = (char *)malloc(strlen(argv[i])*sizeof(char));
      strcpy(column, argv[i]);
      continue;
    }
    if(strcmp(argv[i],"-d") == 0){
      dflag = 1;
      continue;
    }
    if(dflag == 1 && inputDir == NULL){
      column = (char *)malloc(strlen(argv[i])*sizeof(char));
      strcpy(inputDir, argv[i]);
      continue;
    }
    if(strcmp(argv[i],"-o") == 0){
      oflag = 1;
      continue;
    }
	if(oflag == 1 && outputDir == NULL){
      column = (char *)malloc(strlen(argv[i])*sizeof(char));
      strcpy(outputDir, argv[i]);
      continue;
    }
  }
  printf("cflag = %d ; dflag = %d ; oflag = %d\n", cflag, dflag, oflag);
  printf("column = %s ; inputDir = %s ; outputDir = %s \n", column, inputDir, outputDir); 
  return 0;
}
