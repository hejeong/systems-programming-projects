//"The server will opena  port and wait for connection requests. On connection, it will spawn a service thread to handle that connection and go back to waiting for requests. Each service thread should read in a client request, if it is a sort request, it should perform the sort and store the results at the server. If it is a dump request, it should merge the current collection of sorted results into one sorted list and send the result back to the client. You may want/need to make use of synchronization constructs like mutex_locks, semaphores, and/or condition variables in your implementation to prevent memory corruption.
//The server will run until stopped by a SIGKILL (i.e. kill <pid of server>)
//To STDOUT, output a list of ip addresses of all the clients that have connected (?-> when>) Received connections from: <ipaddress>,<ipaddress>,<ipaddress>,..
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

int main(int argc, char** argv){
	struct sockaddr_in address;
	int socketfd = socket(AF_INET,SOCK_STREAM,0);
	
	if (socketfd <= 0){
		printf("Can't open socket\n");
		return 0;
	}	
	if (argc != 2){
		printf("Need port number\n");
		return 0;
	}
	int port = atoi(argv[1]);
	if(port <= 0){
		printf("Invalid port\n");
		return 0;
	}
	memset(&address,0,sizeof(address));

	address.sin_family =AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(socketfd,(struct sockaddr*)&address,sizeof(address)) < 0){
		printf("Bind failed\n");
		return 0;
	}
	
	if(listen(socketfd, 20) < 0){
		printf("Can't listen\n");
		return 0;
	}
	
	while(1){
		struct sockaddr_in incoming;
		memset(&incoming,0,sizeof(incoming));
		socklen_t incomSize = sizeof(incoming);
		int newfd = accept(socketfd,(struct sockaddr*)&incoming,&incomSize);
		/*char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET,&(incoming.sin_addr),str,INET_ADDRSTRLEN);
		printf("%s,",str);*/
		send(newfd, "haha", 4, 0);	
	}
	return 0;
}
