#include <stdio.h> 
#include <stdlib.h> 
#include <time.h> 
#include <sys/time.h>
#include <ctype.h> 
#include <string.h>
#include "mymalloc.h"

//checks if the given block is allocated or free memory
int compMagic(char first, char second){ 
	if(first == 'U' && second == 'A') //2 byte magic number equivalent to U and A in ascii
	{
		return 1;
	}
	else if(first == 'A' && second == 'L') //2 byte magic number equivalent to A and L in ascii
	{
		return 2;
	}
	return 0;
}

//given the two 1 byte chars of a metadata that represent the size of the current block, converts it to an int for use by other functions
int convertToSize(char first, char second){ 
	int x = first;
	int y = second;
	return ((x * 100) + y);
}

//converts to the first digit of the metadata of how large the given block is
char convertToFirstChar(int num){
	int x = num / 100;
	char c = (char)x;
	return c;
}

//converts to the second digit of the metadata of how large the given block is
char convertToSecondChar(int num){
	int x = num % 100;
	char c = (char)x;
	return c;
}

//finds and returns the block right before the given block 
void * findPreviousBlock(void * ptr){
	void * block = &myblock[0];
	while((char*)block < &myblock[4096]) //checks to make sure the ptr doesnt leave the array
	{
		int blockSize = convertToSize(*(char*)(block+2),*(char*)(block+3));
		
		//continuously moves forward and checks if each blocks next block is the function parameter block
		//if the next block of variable block is the block given to the function, the variable block is returned, being the given blocks previous block
		if(compMagic(*(char*)block,*(char*)(block+1)) == 1 && (block + 4 + blockSize == ptr))
		{
			return block;
		}
		else
		{
			block = block + 4 + blockSize; //moves onto the next block to check
		}
	}
	return NULL;
}

//returns a pointer to a block of memory that is the size required
void * myMalloc(int size, char * file, int line){
	if(size <= 0) //makes sure the memory required is a valid size
	{
		printf("invalid size malloc request from file %s on line %d\n", file, line);
		return NULL;
	}
	if(compMagic(myblock[0], myblock[1]) == 0) //"initializes" the array if this is the firs time malloc has been called
	{
		myblock[0] = 'U';
		myblock[1] = 'A';
		myblock[2] = convertToFirstChar(4096);
		myblock[3] = convertToSecondChar(4096);
	}
	
	int i = 0;
	
	while(i <= 4093) //iterates through each existing block 1 by one and finds a block with enough memory for the required size
	{
		int blockSize = convertToSize(myblock[i+2],myblock[i+3]); //the size of the current block
		
		if(compMagic(myblock[i],myblock[i+1]) == 1 && blockSize >= size) //checks if the block is unallocated and big enough for the required size
		{
			myblock[i] = 'A';//*------------------------
			myblock[i+1] = 'L';//*sets the current block to be allocated and of the size that is required
			myblock[i+2] = convertToFirstChar(size);//*
			myblock[i+3] = convertToSecondChar(size);//*-----------------
			int next = i + size + 4;
			myblock[next] = 'U';//*------------------------------------------
			myblock[next+1] = 'A';//*sets the rest of the block that is not needed for the given malloc to be unallocated
			myblock[next+2] = convertToFirstChar(blockSize - 4 - size);//*
			myblock[next+3] = convertToSecondChar(blockSize - 4 - size);//----------------------------------
			void * ptr = &myblock[i+4];
			return ptr;
		}
		else
		{
			i = i + 4 + blockSize;
		}
	}
	
	//prints error if no block of memory is found to satisfy the conditions
	printf("not enough memory for %d bytes from file %s on line %d\n", size, file, line);
	return NULL;
}

//unallocates the given pointer in the myblock array
char * myFree(void * ptr, char * file, int line){
	ptr = ptr - 4; //moves the pointer to the first byte of metadata
	if((char*)ptr <= &myblock[4092] && (char*)ptr >= &myblock[0]) //makes sure the given pointer is inside the myblock array
	{
		char firstNum = *((char *)ptr); //the first magic number byte
		char secondNum = *((char *)(ptr + 1)); //second magic number byte
		if(compMagic(firstNum, secondNum) == 2) //checks if the given memory block is allocated and thus freeable
		{
			*((char *)ptr) = 'U';//sets the block to unallocated
			*((char *)(ptr + 1)) = 'A';
			int blockSize = convertToSize( *((char *)(ptr + 2)) , *((char *)(ptr + 3)) );
			char secondBlock1 = *((char *)(ptr + 4 + blockSize)); //first magic number of the block immediately after
			char secondBlock2 = *((char *)(ptr + 5 + blockSize)); //second magic number of the block immediately after
			if(compMagic(secondBlock1, secondBlock2) == 1) //checks if the block after is unallocated, if it is, combines that block with the current block
			{
				blockSize = blockSize + 4 + convertToSize(*((char *)(ptr + 6 + blockSize)), *((char *)(ptr + 7 + blockSize)));
			}
			void * prevBlock = findPreviousBlock(ptr);
			if(prevBlock != NULL) //checks if a previous block exists
			{
				if(compMagic(*(char*)prevBlock,*(char*)(prevBlock+1)) == 1) //checks if the previous block is unallocated, if it is, combines it with the current block
				{
					blockSize = blockSize + 4 + convertToSize(*(char*)(prevBlock+2),*(char*)(prevBlock+3));
					ptr = prevBlock;
				}
			}
			
			*((char *)(ptr + 2)) = convertToFirstChar(blockSize); //sets the current blocks new blocksize
			*((char *)(ptr + 3)) = convertToSecondChar(blockSize);
		}
		else
		{
			printf("invalid pointer from file %s on line %d\n", file, line);
		}
	}
	else 
	{
		printf("not a pointer or out of bounds from file %s on line %d\n", file, line);
	}
	return NULL;
}
