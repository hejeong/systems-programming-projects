#include <stdio.h>

int main(int argc, char* argv[]){
  // create file pointer
  FILE* fpointer;
  // file pointer reads from stdin
  fpointer = stdin;
  
  char str[512]; 

  if (argc != 3){
    printf("Incorrect number of arguments");
    return -1;
  } else if (strcmp(argv[1], "-c") != 0){
      printf("Missing -c argument.\n");
    return -1;
  }
  char str[512];
  fgets(str, sizeof str, fpointer);
  
  // reads from stdin until end of file
  while(fgets(str, sizeof str, fpointer) != NULL){
   printf("Output: %s\n", str);
  }
   
 return 0;
}
