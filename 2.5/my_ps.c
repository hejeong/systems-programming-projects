#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <math.h>
#include <pwd.h>

int isNumber(char* dir){
	int i;
	for(i = 0; i < strlen(dir); i++){
		if(isdigit(dir[i]) == 0){
			return 0;
		}
	}
	return 1;
}

long getVSZ(char* dir){
	FILE * fd;
	long VSZ;
	
	char* path = (char*)malloc((strlen(dir)+9)*sizeof(char));
	strcpy(path, dir);	
	strcat(path, "statm");
	fd = fopen(path, "r");
	
	fscanf(fd, "%ld", &VSZ);
	fclose(fd);
	free(path);
	return (VSZ * 4);
}

double getMem(long RSS){
	FILE * fd;
	long total;
	fd = fopen("/proc/meminfo", "r");
	
	fscanf(fd, "%*s %ld", &total);
	return (double)RSS / (double)total;
}

int getData(char* dir){
	
	int n = 0;
	struct dirent *d;
	DIR *currentDir = opendir(dir);
	if (currentDir == NULL) //Not a directory or doesn't exist
		return 1;
	while ((d = readdir(currentDir)) != NULL) {
		if(++n > 2)
			break;
	}
	closedir(currentDir);
	if (n <= 2){//Directory Empty
		return 1;
	}
	
	char line[1000];
	int pid;
	char state;
	int groupId;
	int sessionId;
	int foregroundId;
	double uptime;
	long threads;
	unsigned long utime;
	unsigned long stime;
	long niceness;
	unsigned long startTime;
	FILE * fd;
	FILE * ufd;
	FILE * status;
	
	char* path = (char*)malloc((strlen(dir)+9)*sizeof(char));
	
	strcpy(path, dir);	
	strcat(path, "stat");
	fd = fopen(path, "r");
	ufd = fopen("/proc/uptime", "r");
	fscanf(ufd, "%lf", &uptime);
	fscanf(fd, "%d %*s %c %*d %d %d %*d %d %*u %*lu %*lu %*lu %*lu %lu %lu %*ld %*ld %*ld %ld %ld %*ld %llu", &pid, &state, &groupId, &sessionId, &foregroundId, &utime, &stime, &niceness, &threads, &startTime);

	char* statusPath = (char*)malloc((strlen(dir)+9)*sizeof(char));
	
	strcpy(statusPath, dir);	
	strcat(statusPath, "status");
	status = fopen(statusPath,"r");
	
	struct passwd *pass;
	long id, check;
	char bump[1000];
	
	while(fgets(line, 1000, status)){
		sscanf(line, "%s %ld", bump, &check);
		if(strcmp("Uid:", bump) == 0){
			id = check;
			break;
		}
	}
	pass = getpwuid(id);
	char * user = malloc(strlen(pass->pw_name)+1);
	strcpy(user, pass->pw_name);
	
	long mLock;

	
	while(fgets(line, 1000, status)){
		sscanf(line, "%s %ld", bump, &check);
		if(strcmp("VmLck:", bump) == 0){
			mLock = check;
			break;
		}
	}
	
	long RSS;
	
	while(fgets(line, 1000, status)){
		sscanf(line, "%s %ld", bump, &check);
		if(strcmp("VmRSS:", bump) == 0){
			RSS = check;
			break;
		}
	}
	
	double mem = getMem(RSS) * 100;
	
	

	char * stat = malloc(8);
	stat[0] = state;
	stat[1] = '\0';
	
	if(niceness > 0){
		strcat(stat, "N\0");
	}else if(niceness < 0){
		strcat(stat, "<\0");
	}
	
	if(strcmp(line, "VmLck:") == 0 && mLock > 0){
		strcat(stat, "L\0");
	}
	
	if(sessionId == pid){
		strcat(stat, "s\0");
	}
	
	if(threads > 1){
		strcat(stat,"l\0");
	}
	
	if(groupId == foregroundId){
		strcat(stat, "+\0");
	}
	
	
	int hertz = sysconf(_SC_CLK_TCK);
	double secs = uptime - (((double)startTime)/100);
	double CPU = 100*((((double)(stime + utime))/100)/secs);
	CPU = ((int)(10*CPU)) / 10.0;
	long VSZ = getVSZ(dir);
	long time = (utime + stime)/hertz;
	char * timeStr = malloc(20);
	sprintf(timeStr, "%d", time/60);
	strcat(timeStr, ":\0");
	char * temp = malloc(4);
	sprintf(temp, "%d", time%60);
	strcat(timeStr, temp);
	
	char CPUstr[7];
	sprintf(CPUstr, "%lf", CPU);
	if(CPU >= 10){
		CPUstr[4] = '\0';
	}else{
		CPUstr[3] = '\0';
	}
	char memStr[7];
	sprintf(memStr, "%lf", mem);
	if(mem >= 10){
		memStr[4] = '\0';
	}else{
		memStr[3] = '\0';
	}
	int overflow;
	char buffer[20];
	if(strlen(user) > 8){
		user[7] = '+';
		user[8] = '\0';
	}
	printf("%-8s ", user);
	printf("%5d ", pid);
	printf("%4s ", CPUstr);
	printf("%4s ", memStr);
	sprintf(buffer, "%ld", VSZ);
	if(strlen(buffer) > 6){
		overflow = strlen(buffer) - 6;
	}
	printf("%6ld ", VSZ);
	sprintf(buffer, "%ld", RSS);
	if(strlen(buffer) > 5){
		overflow = overflow + (strlen(buffer) - 5);
	}
	printf("%5ld ", RSS);
	printf("%-*s", 9 - overflow, "?");
	printf("%-5s", stat);
	printf("START ");
	printf("%6s ", timeStr);
	printf("COMMAND\n");
	
	fclose(fd);
	fclose(ufd);
	fclose(status);
	return 0;
}

int traverse(char* currentDir){
	DIR *dir;
	struct dirent *dent;
	char buffer[50];
	char *path;
	
	strcpy(buffer, currentDir);
	//open directory
	dir = opendir(buffer);
	//graceful error if dir can't be opened
	if(dir == NULL){
		printf("Directory %s cannot be opened\n", currentDir);
		return;
	}
	while((dent = readdir(dir)) != NULL){
		// skip over [.] and [..]
		if(!strcmp(".", dent->d_name) || !strcmp("..", dent->d_name)){
			continue;	
		}
		if(isNumber(dent->d_name) == 0){
			continue;
		}
		// dynamically allocate memory to create the path for file or directory
		path = (char*)malloc((strlen(currentDir)+strlen(dent->d_name)+2)*sizeof(char));
		strcpy(path, currentDir);
		if(currentDir[strlen(currentDir)-1] != '/'){
			strcat(path, "/");
		}
		strcat(path, dent->d_name);
		// append a '/' character to the end of current path, then traverse into nested directories
		char* newPath = (char*)malloc((strlen(path)+2)*sizeof(char));
		strcpy(newPath, path);
		strcat(newPath, "/");
		//printf("%s\n", newPath);
		getData(newPath);
		// recursively go through nested directories and find tokens
		free(newPath);
		free(path);
	}
	// close the current directory
	closedir(dir);
	return 0;
}

int main(int argc, char* argv[]){
	printf("%-8s ", "USER");
	printf("%5s ", "PID");
	printf("%3s ", "%CPU");
	printf("%4s ", "%MEM");
	printf("%6s ", "VSZ");
	printf("%5s ", "RSS");
	printf("TTY      ");
	printf("STAT ");
	printf("START ");
	printf("%6s ", "TIME");
	printf("COMMAND\n");
	traverse("/proc");
	return 1;
}