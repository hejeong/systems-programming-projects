#include <stdio.h>
int main(int argc, char* argv[]){
  FILE* fpointer;
  fpointer = stdin;
  char* str;
  printf("Argument count is %d\n", argc);
  while(scanf("%[^ ]%*s", str) != EOF){
    printf("Output: %s\n", str);
  } 
 return 0;
}
