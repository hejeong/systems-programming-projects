#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  free(inputDir);
  free(outputDir);
  free(column);
  return 0;
}
