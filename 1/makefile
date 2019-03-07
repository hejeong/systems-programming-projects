memgrind: memgrind.c mymalloc.o
	gcc memgrind.c mymalloc.o -o memgrind

mymalloc.o: mymalloc.c
	gcc  -c mymalloc.c -o mymalloc.o
	
clean:
	rm -rf memgrind mymalloc.o
