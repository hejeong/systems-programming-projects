#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
  
int main(int argc, char ** argv){
	// test case 1:
	int pid = fork();
	if (pid == 0) {
		execv("./WTFserver", (char *[]){"./WTFserver", "4326", NULL});
		exit(0);
	}else if(pid > 0) {
		int pid_3 = fork();
		if(pid_3 == 0){
			int pid_1 = fork();
			if(pid_1 == 0){
				int pid_2 = fork();
				if(pid_2 == 0){
					execv("./WTF", (char *[]){"./WTF", "configure", "2123", "4326", NULL});
					exit(0);
				}else if(pid_2 > 0){
					wait();
					execv("./WTF", (char *[]){"./WTF", "create", "p1", NULL});
				}
				exit(0);
			}else if(pid_1 > 0){
				// Test Case 3:
				wait();
				execv("./WTF", (char *[]){"./WTF", "add", "p1", "nonexistent.txt", NULL});
			}
			exit(0);
		}else if(pid_3 > 0){
			wait();
			execv("./WTF", (char *[]){"./WTF", "destroy", "p1", NULL});
			printf("here\n");
		}
	}
	return 0;
	
}