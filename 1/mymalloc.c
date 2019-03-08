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

int compMagic(char first, char second){
	if(first == 'U' && second == 'A')
	{
		return 1;
	}
	else if(first == 'A' && second == 'L')
	{
		return 2;
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

void initialize(){
	struct data meta;
	meta.Num1 = 'A';
	meta.Num2 = 'B';
	meta.Size1 = convertToFirstChar(4096);
	meta.Size2 = convertToSecondChar(4096);
	void * ptr = &myblock[0];
	*(struct data*)ptr = meta;
}

void * myMalloc(int size, int file, int line){
	if(!compMagic(myblock[0], myblock[1]))
	{
		myblock[0] = 'U';
		myblock[1] = 'A';
		myblock[2] = convertToFirstChar(4096);
		myblock[3] = convertToSecondChar(4096);
	}
	
	int i = 0;
	
	while(i <= 4093)
	{
		int blockSize = convertToSize(myblock[i+2],myblock[i+3]);
		
		if(compMagic(myblock[i],myblock[i+1]) == 1 && blockSize >= size)
		{
			myblock[i] = 'A';
			myblock[i+1] = 'L';
			myblock[i+2] = convertToFirstChar(size);
			myblock[i+3] = convertToSecondChar(size);
			i = i + blockSize + 4;
			myblock[i] = 'U';
			myblock[i+1] = 'A';
			myblock[i+2] = convertToFirstChar(blockSize - 4 - size);
			myblock[i+3] = convertToSecondChar(blockSize - 4 - size);
			void * ptr = &myblock[i+4];
			return ptr;
		}
		else
		{
			i = i + 4 + blockSize;
		}
	}
	//error goes here
	return NULL;
}

char * myFree(void * ptr){
	ptr = ptr - 4;
	char firstNum = *((char *)ptr);
	char secondNum = *((char *)(ptr + 1));
	if(ptr <= &myblock[4092] && ptr >= &myblock[0])
	{
		if(compMagic(firstNum, secondNum) == 2)
		{
			*((char *)ptr) = 'U';
			*((char *)(ptr + 1)) = 'A';
			int blockSize = convertToSize( *((char *)(ptr + 2)) , *((char *)(ptr + 3)) );
			char secondBlock1 = *((char *)(ptr + 4 + blockSize));
			char secondBlock2 = *((char *)(ptr + 5 + blockSize));
			if(compMagic(secondBlock1, secondBlock2) == 2)
			{
				blockSize = blockSize + convertToSize(*((char *)(ptr + 6 + blockSize)), *((char *)(ptr + 7 + blockSize)));
			}
			*((char *)(ptr + 2)) = convertToFirstChar(blockSize);
			*((char *)(ptr + 3)) = convertToFirstChar(blockSize);
		}
		//error
	}
	//error
	return NULL;
}

int main(int argc, char* argv[]){ //test
	int i = 232;
	int j = 4124;
	myblock[0] = 'g';
	myblock[1] = 'j';
	void * ptr = &myblock[0];
	printf("%c\n", *(char*)ptr);
	printf("%c\n", *(char*)(ptr+1));
}
