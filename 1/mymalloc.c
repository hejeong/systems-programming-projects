#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static char myblock[4096];

struct data //metadata, first is the first two digits, second is last two digits of how much space the following 
{
	char first;
	char second;
	int test;
};

int compMagic(char first, char second){
	if(first == 'A' && second == 'B')
	{
		return 1;
	}
	return 0;
}

char * myMalloc(int size){
	if(!compMagic(myblock[0], myblock[1]))
	{
	}
	return NULL;
}

int main(int argc, char* argv[]){ //test
	int i = 232;
	int j = 412408;
	struct data dat;
	dat.test = 41245;
	//memcpy(&myblock[0],&j,sizeof(int));
	myblock[0] = dat;
	printf("%d\n",myblock[0].test);
	printf("%d\n",(int)myblock[0].test);
}