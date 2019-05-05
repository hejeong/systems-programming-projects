#include "define.h"

char ipHost[100];
int port;
int sock = -1;

void configure(char* host, char* port){
	int fd = open(".configure", O_WRONLY | O_CREAT | O_TRUNC, 0700); 
	write(fd, host, strlen(host));
	write (fd, "\n", 1);
	write(fd, port, strlen(port));
	return;
}

void createClient(char* projName) {
	int n=-1;
	char message[256];
	sprintf(message, "./WTF create %s", projName);
	write(sock, message, 256);
	printf("Message: %s\n",message);
	//n = read(sock, message, 256);
	//printf("Message read: %s\n", message);
}

void destroyClient(char* projName) {
	char message[256];
	sprintf(message, "./WTF destroy %s", projName);
	write(sock, message, 256);
	printf("Message: %s\n",message);
}

void connectToServer(char* ipHost, int port) {
	int valread;
	char buffer[1024] = {0};
	struct sockaddr_in serv_addr;

	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\nSocket creation error \n"); 
		return;
	}
	memset(&serv_addr, '0', sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port); 

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) { 
		printf("\nInvalid address/ Address not supported \n"); 
		return; 
    } 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) { 
        printf("\nConnection Failed \n"); 
        return; 
    } 
    else {
    	printf("Successfully connected to server.\n");
    	printf("SOCK: %d\n", sock);
    	return;
    }
    return; 
}

int main(int argc, char* argv[]) {
	if (strcmp(argv[1], "configure") == 0) {
		if (argc != 4) {
			printf("Wrong number of arguments.\n");
			return 0;
		}
		strncpy(ipHost, argv[2], sizeof(ipHost)-1);
		port = atoi(argv[3]);
		printf("Successfully configured.\nHost: %s\tPort: %d\n",ipHost, port);
		configure(argv[2], argv[3]);
		return 0;
	}
	else {
		char line[100];
		FILE* fp = fopen(".configure", "r");
		if (fp == NULL) {
			printf("Configure file does not exist\n");
			return 0;
		}
		fgets(line, sizeof(line), fp);
		strcpy(ipHost,line);
		strtok(ipHost, "\n");
		fgets(line, sizeof(line), fp);
		port = atoi(line);
		printf("IPHOST: %s\nPORT: %d\n", ipHost, port);
	}

	if (strcmp(argv[1], "add") == 0 || strcmp(argv[1], "remove") == 0 || strcmp(argv[1], "rollback") == 0) {
		printf("ARGV[1]: %s\n",argv[1]);
		if (argc != 4) {
			printf("Error. Too many or too few arguments.\n");
			return 0;
		}
	}

	else {
		if(argc != 3) {
			printf("Error. Too many or too few arguments.\n");
			return 0;
		}
		else {
			if (strcmp(argv[1], "create") == 0) {
				connectToServer(ipHost, port);
				//printf("Sock: %d\n", sock);
				createClient(argv[2]);
			}
			else if (strcmp(argv[1], "destroy") == 0) destroyClient(argv[2]);
		}
	}

}
