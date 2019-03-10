#include <stdio.h> 
#include <stdlib.h> 
#include <time.h> 
#include <sys/time.h>
#include <ctype.h> 
#include <string.h>
#include "mymalloc.h"

#define malloc(x) myMalloc(x,__FILE__,__LINE__);
#define free(x) myFree(x,__FILE__,__LINE__);

void workloadA();
void workloadB();
void workloadC();
void workloadD();
void workloadE();
void workloadF();

int main(int argc, char** argv){
	struct timeval tvStart, tvAfter;
	int j, a=0, b=0, c=0, d=0, e=0, f=0;
	for(j=0; j<100; j++){
		// grab start time
		gettimeofday(&tvStart, NULL);
		workloadA();
		// grab end time
		gettimeofday(&tvAfter, NULL);
		// get total times in microseconds
		a+=(tvAfter.tv_sec*1000000L+tvAfter.tv_usec)-(tvStart.tv_sec*1000000L+tvStart.tv_usec);
		gettimeofday(&tvStart, NULL);
		workloadB();
		gettimeofday(&tvAfter, NULL);
		b+=(tvAfter.tv_sec*1000000L+tvAfter.tv_usec)-(tvStart.tv_sec*1000000L+tvStart.tv_usec);
		gettimeofday(&tvStart, NULL);
		workloadC();
		gettimeofday(&tvAfter, NULL);
		c+=(tvAfter.tv_sec*1000000L+tvAfter.tv_usec)-(tvStart.tv_sec*1000000L+tvStart.tv_usec);
		gettimeofday(&tvStart, NULL);
		workloadD();
		gettimeofday(&tvAfter, NULL);
		d+=(tvAfter.tv_sec*1000000L+tvAfter.tv_usec)-(tvStart.tv_sec*1000000L+tvStart.tv_usec);	
		gettimeofday(&tvStart, NULL);
		workloadE();
		gettimeofday(&tvAfter, NULL);
		e+=(tvAfter.tv_sec*1000000L+tvAfter.tv_usec)-(tvStart.tv_sec*1000000L+tvStart.tv_usec);
		gettimeofday(&tvStart, NULL);
		workloadF();
		gettimeofday(&tvAfter, NULL);
		f+=(tvAfter.tv_sec*1000000L+tvAfter.tv_usec)-(tvStart.tv_sec*1000000L+tvStart.tv_usec);			
	}
	// print out mean runtimes for 100 loops
	printf("Mean runtime A: %ld microseconds\n", a/100); 
	printf("Mean runtime B: %ld microseconds\n", b/100); 
	printf("Mean runtime C: %ld microseconds\n", c/100); 
	printf("Mean runtime D: %ld microseconds\n", d/100);
	printf("Mean runtime E: %ld microseconds\n", e/100);
	printf("Mean runtime F: %ld microseconds\n", f/100);
}

void workloadA(){
	int i;
	// loop 150 times
	for(i=0; i<150; i++){
		// malloc 1 byte
		void *ptr = malloc(1*sizeof(char)); 
		if(ptr != NULL){
			// free returned pointer 
			free(ptr);
		}
	}
}

void workloadB(){
	int counter = 0, setsOfFifty = 0;
	void *collection[50];
	// keep doing until allocated and freed 150 bytes
	while(setsOfFifty != 3){
		// malloc 1 byte
		void *ptr = malloc(1*sizeof(char));
		if(ptr != NULL){
			// store pointer in array
			collection[counter] = ptr;
			// increase malloc() count
			counter++;
		}
		// every time we reach 50 allocated bytes at a time
		// free all the pointers
		if(counter == 50){
			// free all pointers until all freed
			while(counter > 0){
				free(collection[counter-1]);
				// decrease allocated count
				counter--;
			}
			// increment (finished 1 set of 50); end at 3
			setsOfFifty++;
		}
	}
}

void workloadC(){
	// seed random number
	srand(time(0));
	
	int counter = 0, elements = 0, randomInt;
	void *collection[50];
	
	// keep looping until malloc()ed 50 bytes
	while(counter < 50){
		// generate random number: 0 - free(), 1 - malloc()
		randomInt = rand() % 2;
		// case 1: malloc() 1 byte
		if(randomInt == 1){
			void *ptr = malloc(1*sizeof(char));
			// check if malloc succeeds
			if(ptr != NULL){
				//SUCCESS: 
				// 1) store returned pointer 
				// 2) increment malloc counter by 1
				// 3) increase # of elements in array
				collection[elements] = ptr;
				counter++;
				elements++;
			}

		}else{
		// case 2: free()
			if(elements > 0){
				//free if there exists malloc()ed elements
				free(collection[elements-1]);
				//decrement # of elements by 1
				elements--;
			}
		}
	}
	while(elements > 0){
		//free if there exists malloc()ed elements
		free(collection[elements-1]);
		//decrement # of elements by 1
		elements--;
	}
}

void workloadD(){
	// seed random number 
	srand(time(0));
	int bytesAllocated, timesMalloced = 0, elements = 0, mallocFlag;
	void *collection[61];
	// keep doing until malloc()ed 50 times
	while(timesMalloced < 50){
		// random number: 1 - malloc, 0 - free
		mallocFlag = rand() % 2;
		if(mallocFlag == 1){
			// random size from 1 to 64
			int randomSize = rand()%64 + 1;
			// allocate random sized byte
			void *ptr = malloc(randomSize*sizeof(char));
			if(ptr != NULL){
				// store pointer
				collection[elements] = ptr;
				// increase number of elements in array
				elements++;
				// increase total number of malloc calls
				timesMalloced++;
				// keep track of total bytes allocated
				bytesAllocated+= randomSize;
			}
		}else{
			if(elements > 0){
				//free if there exists malloc()ed elements
				free(collection[elements-1]);
				//decrement # of elements by 1
				elements--;
			}
		}
	}
	// free remaining pointers
	while(elements > 0){
		free(collection[elements-1]);
		elements--;
	}
}
void workloadE(){
	int i;
	void *collection[512];
	//malloc 512 '4-bytes' of memory; total: 2048 bytes, 2048 bytes of metadata
	for(i=0; i<512; i++){
		void *ptr = malloc(4*sizeof(char));
		collection[i] = ptr;
	}
	int lastBlockSize = 4, lastIndex = 511;
	// free last two blocks of memory
	// reallocate combined blocks (metadata size 4 bytes)
	// formula: sizeof(lastBlock) + sizeof(2nd to last block) + sizeof(old metadata) = newSize
	// i.e. first iteration: 4 + 4 + 4 = 12,
	// i.e. second iteration: 12 + 4 + 4 = 20
	while(lastBlockSize < 4092){
		free(collection[lastIndex]);
		free(collection[lastIndex-1]);
		//lastIndex decremented to store return pointer of combined blocks
		lastIndex--;
		lastBlockSize += 4 + 4;
		void *ptr = malloc(lastBlockSize*sizeof(char));
		collection[lastIndex] = ptr;
	}
	// free last block of size 4094 bytes
	free(collection[lastIndex]);
}
void workloadF(){
	void *collection[512];
	int i = 0;
	void * ptr;

	for(i = 0; i <= 512; i++) //fills the memory with 4 byte blocks
	{
		ptr = malloc(4);
		if(ptr != NULL)
		{
			collection[i] = ptr;
		}
	}
	for(i = 511; i >= 0; i--) //frees the blocks from the end to the beginning
	{
		free(collection[i]);
	}
	free(collection[0]);
	free(collection[6]);
	free(collection[0]-10);
	int x;
	free((int*) x);
	for(i = 0; i < 512; i++) //allocates 4 byte blocks again
	{
		ptr = malloc(4);
		if(ptr != NULL)
		{
			collection[i] = ptr;
		}
	}
	for(i = 0; i < 512; i++) //frees the blocks from the beginning to the end
	{
		free(collection[i]);
	}
}