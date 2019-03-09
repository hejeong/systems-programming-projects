#include <stdio.h> 
#include <stdlib.h> 
#include <time.h> 
#include <sys/time.h>
#include <ctype.h> 
#include <string.h>
#include "mymalloc.h"

#define malloc(x) myMalloc(x,__FILE__,__LINE__);
#define free(x) myFree(x,__FILE__,__LINE__);
// hi

void workloadA();
void workloadB();
void workloadC();
void workloadD();
void workloadF();
//void runTime();
int main(int argc, char** argv){
	struct timeval tvStart, tvAfter;
	int j, a=0, b=0, c=0, d=0, f=0;
	for(j=0; j<100; j++){
		/*gettimeofday(&tvStart, NULL);
		workloadA();
		gettimeofday(&tvAfter, NULL);
		a+=(tvAfter.tv_sec*1000000L+tvAfter.tv_usec)-(tvStart.tv_sec*1000000L+tvStart.tv_usec);
		gettimeofday(&tvStart, NULL);
		workloadB();
		gettimeofday(&tvAfter, NULL);
		b+=(tvAfter.tv_sec*1000000L+tvAfter.tv_usec)-(tvStart.tv_sec*1000000L+tvStart.tv_usec);
		gettimeofday(&tvStart, NULL);
		workloadC();
		gettimeofday(&tvAfter, NULL);
		c+=(tvAfter.tv_sec*1000000L+tvAfter.tv_usec)-(tvStart.tv_sec*1000000L+tvStart.tv_usec);*/
		gettimeofday(&tvStart, NULL);
		workloadF();
		gettimeofday(&tvAfter, NULL);
		f+=(tvAfter.tv_sec*1000000L+tvAfter.tv_usec)-(tvStart.tv_sec*1000000L+tvStart.tv_usec);
	/*	gettimeofday(&tvStart, NULL);
		workloadD();
		gettimeofday(&tvAfter, NULL);
		d+=(tvAfter.tv_sec*1000000L+tvAfter.tv_usec)-(tvStart.tv_sec*1000000L+tvStart.tv_usec);
		*/
	}
	printf("Mean runtime A: %ld microseconds\n", a/100); 
	printf("Mean runtime B: %ld microseconds\n", b/100); 
	printf("Mean runtime C: %ld microseconds\n", c/100); 
	printf("Mean runtime F: %ld microseconds\n", f/100); 
/*	printf("Mean runtime D: %ld microseconds\n", d/100); */
//	workloadB();
//	workloadC();
//	workloadD();
	return 0;
}
/*	
void runTime(){
	struct timeval tvStart, tvAfter;
	gettimeofday(&tvStart, NULL);
	int i = 0, startTime, endTime, runTime;
	while(i < 10000){
		i++;
	}
	gettimeofday(&tvAfter, NULL);
	startTime = tvStart.tv_sec*1000000L+tvStart.tv_usec;
	endTime = tvAfter.tv_sec*1000000L+tvAfter.tv_usec;
	runTime = endTime-startTime; 
	printf("RUNTIME: %ld microseconds \n", runTime);

}*/

void workloadA(){
	int i;
	for(i=0; i<150; i++){
		void *ptr = myMalloc(sizeof(char),4,4); 
		if(ptr != NULL){
			myFree(ptr,4,4);
		}
	}
	trigger();
}

void workloadB(){
	int i, counter = 0, iterations = 0;
	void *collection[50];
	while(iterations != 3){
		void *ptr = myMalloc(1*sizeof(char),4,4);
		if(ptr != NULL){
			collection[counter] = ptr;
			counter++;
		}
		if(counter == 50){
			while(counter > 0){
				myFree(collection[counter-1],4,4);
				counter--;
			}
			iterations++;
		}
	}
}

void workloadC(){
	// seed random number
	srand(time(0));
	
	int i, counter = 0, elements = 0, randomInt;
	void *collection[50];
	
	// keep looping until malloc()ed 50 bytes
	while(counter < 50){
		// generate random number: 0 - free(), 1 - malloc()
		randomInt = rand() % 2;
		// case 1: malloc() 1 byte
		if(randomInt == 1){
			void *ptr = myMalloc(1*sizeof(char),4,4);
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
				myFree(collection[elements-1],4,4);
				//decrement # of elements by 1
				elements--;
			}
		}
	}
	while(elements > 0){
		//free if there exists malloc()ed elements
		myFree(collection[elements-1],4,4);
		//decrement # of elements by 1
		elements--;
	}
}
/*
void workloadD(){
	srand(time(0));
	int bytesFree, timesMalloced = 0, elements = 0, mallocFlag;
	void *collection[50];
	while(timesMalloced < 50){
		mallocFlag = rand() % 2;
		if(mallocFlag == 1){
			int randomSize = rand()%64 + 1;
			void *ptr = myMalloc(randomSize*sizeof(char),4,4);
			if(ptr != NULL){
				collection[elements] = ptr;
				elements++;
				timesMalloced++;
			}
		}else{
			if(elements > 0){
				//free if there exists malloc()ed elements
				myFree(collection[elements-1],4,4);
				//decrement # of elements by 1
				elements--;
			}
		}
	}
	while(elements > 0){
		myFree(collection[elements-1],4,4);
		elements--;
	}
}

void workloadE(){
	int i;
	void *collection[512];
	//malloc 512 '4-bytes' of memory; total: 2048 bytes
	for(i=0; i<512; i++){
		void *ptr = myMalloc(4*sizeof(char),4,4);
		collection[i] = ptr;
	}
	int lastBlockSize = 4, lastIndex = 511;
	// free last two blocks of memory
	// reallocate combined blocks (metadata size 4 bytes)
	// formula: sizeof(lastBlock) + sizeof(2nd to last block) + sizeof(metadata) = newSize
	// i.e. first iteration: 4 + 4 + 4 = 12, 
	// i.e. second iteration: 12 + 4 + 4 = 20
	while(lastBlockSize < 2048){
		myFree(collection[lastIndex],4,4);
		myFree(collection[lastIndex-1],4,4);
		//lastIndex decremented to store return pointer of combined blocks
		lastIndex--;
		lastBlockSize += 4 + 4;
		void *ptr = myMalloc(lastBlockSize*sizeof(char),4,4);
		collection[lastIndex] = ptr;
	}
}*/

