#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

int main (){
	while (true){
		sleep(2);
		printf("I'm still here\n");
	}
	return 0;
}
