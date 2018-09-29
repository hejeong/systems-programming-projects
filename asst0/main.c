#include <stdio.h>
int main(int argc, char* argv[]){
  // create file pointer
  FILE* fpointer;
  // file pointer reads from stdin
  fpointer = stdin;
  
  
  char* str;

  // reads from stdin until end of file
  while(scanf("%[^\n]%*c", str) != EOF){
    printf("Output: %s\n", str);
  } 

 return 0;
}
