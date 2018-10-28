
scannerCSVsorter: scannerCSVsorter.o mergesort.o simpleCSVsorter.o
	gcc -Wall -fsanitize=address -o scannerCSVsorter scannerCSVsorter.o mergesort.o simpleCSVsorter.o
	
scannerCSVsorter.o: scannerCSVsorter.c mergesort.c simpleCSVsorter.h
	gcc -Wall -fsanitize=address -c scannerCSVsorter.c mergesort.c simpleCSVsorter.h
	
mergesort.o: mergesort.c simpleCSVsorter.h
	gcc -Wall -fsanitize=address -c mergesort.c simpleCSVsorter.h

clean:
	rm -f scannerCSVsorter.o mergesort.o simpleCSVsorter.g.gch