#include <stdio.h> 
#include <stdlib.h> 
#include <time.h> 
#include <sys/time.h>
#include <ctype.h> 
#include <string.h>
#include "mymalloc.h"
// hi
/*struct data //metadata, first is the first two digits, second is last two digits of how much space the following block takes up
{
	char Num1;
	char Num2;
	char Size1;
	char Size2;
}; unused for now*/

void trigger(){
	printf("%c\n%c\n%c\n%c\n", myblock[0],myblock[1],myblock[2],myblock[3]);
}

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

void * findPreviousBlock(void * ptr){
	void * block = &myblock[0];
	while((char*)block < &myblock[4096])
	{
		int blockSize = convertToSize(*(char*)(block+2),*(char*)(block+3));
		
		if(compMagic(*(char*)block,*(char*)(block+1)) == 1 && (block + 4 + blockSize == ptr))
		{
			return block;
		}
		else
		{
			block = block + 4 + blockSize;
		}
	}
	return NULL;
}

void * myMalloc(int size, int file, int line){
	if(size <= 0)
	{
		printf("invalid size\n");
	}
	if(compMagic(myblock[0], myblock[1]) == 0)
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
			int next = i + size + 4;
			myblock[next] = 'U';
			myblock[next+1] = 'A';
			myblock[next+2] = convertToFirstChar(blockSize - 4 - size);
			myblock[next+3] = convertToSecondChar(blockSize - 4 - size);
			void * ptr = &myblock[i+4];
			return ptr;
		}
		else
		{
			i = i + 4 + blockSize;
		}
	}
	
	printf("not enough memory for %d bytes\n", size);
	return NULL;
}

char * myFree(void * ptr, int file, int line){
	ptr = ptr - 4;
	if((char*)ptr <= &myblock[4092] && (char*)ptr >= &myblock[0])
	{
		char firstNum = *((char *)ptr);
		char secondNum = *((char *)(ptr + 1));
		if(compMagic(firstNum, secondNum) == 2)
		{
			*((char *)ptr) = 'U';
			*((char *)(ptr + 1)) = 'A';
			int blockSize = convertToSize( *((char *)(ptr + 2)) , *((char *)(ptr + 3)) );
			char secondBlock1 = *((char *)(ptr + 4 + blockSize));
			char secondBlock2 = *((char *)(ptr + 5 + blockSize));
			if(compMagic(secondBlock1, secondBlock2) == 1)
			{
				blockSize = blockSize + 4 + convertToSize(*((char *)(ptr + 6 + blockSize)), *((char *)(ptr + 7 + blockSize)));
			}
			void * prevBlock = findPreviousBlock(ptr);
			if(prevBlock != NULL)
			{
				if(compMagic(*(char*)prevBlock,*(char*)(prevBlock+1)) == 1)
				{
					blockSize = blockSize + 4 + convertToSize(*(char*)(prevBlock+2),*(char*)(prevBlock+3));
					ptr = prevBlock;
				}
			}
			
			*((char *)(ptr + 2)) = convertToFirstChar(blockSize);
			*((char *)(ptr + 3)) = convertToSecondChar(blockSize);
		}
		else
		{
			printf("invalid pointer\n");
		}
	}
	else 
	{
		printf("not a pointer or out of bounds\n");
	}
	return NULL;
}

/*int main(int argc, char* argv[]){ //test
	myblock[4] = 'j';
	char * ptr = myMalloc(4,4,4);
	printf("%c\n%c\n%c\n%c\n", myblock[0],myblock[1],myblock[2],myblock[3]);
	printf("%c\n%c\n%c\n%c\n", myblock[8],myblock[9],myblock[10],myblock[11]);
	if(ptr != NULL)
	{
		printf("%c\n",*(char*)ptr);
		int x;
		myFree(ptr,4,4);
	}
	printf("%c\n%c\n%c\n%c\n", myblock[0],myblock[1],myblock[2],myblock[3]);
	return 0;
}*/
