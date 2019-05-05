#include "define.h"
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int exist = 0;
void substr(char* str, char* sub, int start, int len)
{
        memcpy(sub, &str[start], len);
        sub[len] = '\0';
}
void *create(void *ptr)
{
	char * string;
	char * sub;
	char* manifest = ".Manifest";
	char cwd[256];
	getcwd(cwd, sizeof(cwd));
	strcat(cwd, "/repo");
	string = ptr;
	substr(string, sub, 7, strlen(string));

	DIR *dir;
	struct dirent *ent;
	if((dir= opendir(cwd))!=NULL)
	{
		while((ent= readdir(dir))!=NULL)
		{
			if(strcmp(sub, ent->d_name)==0)
			{
				exist =1;
				break;
			}
		}
		if(exist==0)
		{
			strcat(cwd, "/");
			strcat(cwd, sub);
			int result = mkdir(cwd, 0777);
			strcat(cwd, "/1");
			strcat(cwd, "/");
			strcat(cwd, manifest);
			result = mkdir(cwd, 0777);
			int fd = open(cwd, O_WRONLY | O_APPEND|O_CREAT,0644);
			write(fd, "1\n", 3);
			close(fd);
		}
		closedir(dir);
	}
	return;
}
void *socketThread(void *arg)
{
	printf("New thread created\n");
    int n = -1;
    char buffer[256];
    char *sub;
    char*token;
    char* temp;
    int i = 0;
    const char s[2] = " ";

    int newsockfd = *((int*)arg);
    bzero(buffer,256);
    printf("NEWSOCKFD: %d\n", newsockfd);
    n = read(newsockfd,buffer,256);
    printf("passed read\n");
    if(n < 0)
    {
            error("ERROR reading from socket");
    }
    else
    {
            printf("Successfully read message: %s\n", buffer);
    }
    //n = write(newsockfd, "I got your message", 18);
    token = strtok(buffer, s);
    token = strtok(NULL, s);
    if(strcmp(token, "create")==0)
    {
            create(buffer);
            if(exist==0)
            {
                    pthread_mutex_lock(&lock);
                    substr(buffer, sub, 7, strlen(buffer));
                    temp = "sendfile:1:3:";
                    pthread_mutex_unlock(&lock);
                    write(newsockfd,temp,strlen(temp));
            }
    }
    if(n < 0)
    {
            error("ERROR writing to socket");
    }
    exist = 0;

}


int main(int argc, char* argv[])
{
	int sockfd = -1;
	int newsockfd = -1;
	int portno = -1;
	int clilen = -1;
	int i=0, j=0;
	struct sockaddr_in serverAddressInfo;
	struct sockaddr_in clientAddressInfo;
	struct sockaddr_storage serverStorage;
	int check;
	socklen_t addr_size;

	if(argc < 2)
	{
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	portno = atoi(argv[1]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		error("ERROR opening socket");
	}
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
	{
		error("setsockopt(SO_REUSEADDR) failed");
	}
	bzero((char *)&serverAddressInfo, sizeof(serverAddressInfo));
	serverAddressInfo.sin_port = htons(portno);
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_addr.s_addr = INADDR_ANY;
	if(bind(sockfd, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo)) < 0)
	{
		error("ERROR on binding");
	}
	if(listen(sockfd, 50)==0)
	printf("Listening\n");
	pthread_t tid[60];
	//clilen = sizeof(clientAddressInfo);
	mkdir("repo", S_IRWXU);
	while(1)
	{
		addr_size = sizeof(serverStorage);
		if (newsockfd = accept(sockfd, (struct sockaddr *)&serverStorage, &addr_size) < 0)
		{
			error("ERROR on accept\n");
		}
		else
		{
			printf("Accepted connection from client\n");
		}
		if(pthread_create(&tid[i],NULL,socketThread,&newsockfd)!=0)
			printf("failed to create thread\n");

    	if(i >= 2)
    	{
      		j=0;
      		while(j < i)
      		{
        			pthread_join(tid[j++],NULL);
      		}
		}
		i++;
	}
	return 0;

}
