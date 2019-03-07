#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static char myblock[4096];

struct data //metadata, first is the first two digits, second is last two digits of how much space the following block takes up
{
	char Num1;
	char Num2;
	char Size1;
	char Size2;
};

void initialize(){
	struct data meta;
	meta.Num1 = 'A';
	meta.Num2 = 'B';
	meta.Size1 = convertToFirstChar(4096);
	meta.Size2 = convertToSecondChar(4096);
	void * ptr = &myblock[0];
	*(struct data*)ptr = meta;
}

int compMagic(char first, char second){
	if((first == 'A' && second == 'B') || (first == 'C' && second == 'D'))
	{
		return 1;
	}
	return 0;
}

int convertToSize(char first, char second){
	int x = first;
	int y = second;
	return ((x * 100) + y);
}

char convertToFirstChar(int num){
	int x = num / 100;
	char c = (char)x;
	return c;
}

char convertToSecondChar(int num){
	int x = num % 100;
	char c = (char)x;
	return c;
}

void * myMalloc(int size){
	if(!compMagic(myblock[0], myblock[1]))
	{
		myblock[0] = 'A';
		myblock[1] = 'B';
		myblock[2] = convertToFirstChar(4096);
		myblock[3] = convertToSecondChar(4096);
	}
	
	return NULL;
}

char * myFree(void * ptr){
	return NULL;
}

int main(int argc, char* argv[]){ //test
	int i = 232;
	int j = 4124;
	struct data dat;
	dat.size = 41245;
	void * ptr = &myblock[0];
	*(struct data*)ptr = dat;
	printf("%d\n",((struct data*)ptr)->size);
}
