all: WTF WTFserver

WTF: WTF.c
	gcc -O WTF.c -o WTF -lssl -lcrypto

WTFserver: WTFserver.c
	gcc -O WTFserver.c -o WTFserver -lpthread -lssl -lcrypto
	
clean:
	rm -rf WTF WTFserver
