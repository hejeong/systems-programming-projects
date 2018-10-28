
scannerCSVsorter: scannerCSVsorter.o mergesort.o simpleCSVsorter.o
	gcc -Wall -fsanitize=address -o scannerCSVsorter scannerCSVsorter.o mergesort.o simpleCSVsorter.o
	
scannerCSVsorter.o: scannerCSVsorter.c mergesort.c
	gcc -Wall -fsanitize=address -c scannerCSVsorter.c mergesort.c
	
mergesort.o: mergesort.c
	gcc -Wall -fsanitize=address -c mergesort.c 

clean:
	rm -f scannerCSVsorter.o mergesort.o simpleCSVsorter.g.gch