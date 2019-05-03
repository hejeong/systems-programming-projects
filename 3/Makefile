all: WTF WTFserver

WTF: WTF.c
	gcc -O WTF.c -o WTF -lssl -lcrypto

WTFserver: WTFserver.c
	gcc -O WTFserver.c -o WTFserver -lpthread
	
clean:
	rm -rf WTF WTFserver
