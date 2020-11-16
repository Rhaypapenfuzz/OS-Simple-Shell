#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

char prompt[] = "> ";
char delimiters[] = " \t\r\n";
extern char **environ;
char *token;
int num;
pid_t pid;

void handler(int signum)
{ 
	// signal handler to kill process after ctrl + c is pressed
	kill(pid,SIGKILL);
}

void timer(int signum)
{ 
	// signal handler to kill process after ten seconds
	kill(pid,SIGKILL);
}

int main() {
    // Stores the string typed into the command line.
    char command_line[MAX_COMMAND_LINE_LEN];
    char cmd_bak[MAX_COMMAND_LINE_LEN];
  
    // Stores the tokenized command line input.
    char *arguments[MAX_COMMAND_LINE_ARGS];
		
		// Stores the current working directory
		char cwd[MAX_COMMAND_LINE_LEN];
		
		// Stores the path of the file to open
		char path[MAX_COMMAND_LINE_LEN];
    	
    while (true) {
			
				// 0. Modify the prompt to print the current working directory
				getcwd(cwd, sizeof(cwd));
				printf("%s%s", cwd, prompt);
        do{  
            fflush(stdout);
            // Read input from stdin and store it in command_line. If there's an
            // error, exit immediately. (If you want to learn more about this line,
            // you can Google "man fgets")
        
            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "fgets error");
                exit(0);
            }
        }while(command_line[0] == 0x0A);  // while just ENTER pressed
			
        // If the user input was EOF (ctrl+d), exit the shell.
        if (feof(stdin)) {
            printf("\n");
            fflush(stdout);
            fflush(stderr);
            return 0;
        }
			
        // 1. Tokenize the command line input (split it on whitespace)
				int i = 0;
        token = strtok(command_line, delimiters);
        while (token != NULL)
        {
           arguments[i] = token;
           token = strtok(NULL, delimiters);
					 i++;
        }
			
        // 2. Implement Built-In Commands
				if (strcmp(arguments[0], "pwd") == 0) {
					printf("current working directory: %s\n",cwd);
				} 
				else if(strcmp(arguments[0], "cd") == 0) {
					if (arguments[1] != NULL) {
						chdir(arguments[1]);
					}else {
						printf("Missing arguments. Enter new directory.\n");
					}
				}
				
				else if (strcmp(arguments[0], "echo") == 0) {
					if (arguments[1] != NULL) {
						int j = 2;
						char str[80];
						// Checks for $ and stores env variablevto display
						if (arguments[1][0] == '$') {
							memmove(arguments[1], arguments[1]+1, strlen(arguments[1]));
							strcpy(str, getenv(arguments[1]));
						}
						// Displays string passed in command_line
						else{
							strcpy(str, arguments[1]);
						}
						while (arguments[j]!=NULL){
							if (arguments[j][0] == '$') {
								memmove(arguments[j], arguments[j]+1, strlen(arguments[j]));
								strcat(str, " ");
								strcat(str, getenv(arguments[j]));
								j++;
							} else {
								strcat(str, " ");
								strcat(str, arguments[j]);
								j++;
							}
						}
						printf("%s\n", str);
					} 
					else {
						printf("Missing arguments. Enter new env variables.\n");
					}
				}
				else if (strcmp(arguments[0], "exit") == 0) {
					exit(0);
				}
				else if (strcmp(arguments[0], "env") == 0) {  
					// Iterates over environ to display all env variables
					char **env;
					for (env = environ; *env != 0; env++)
					{
						char *thisEnv = *env;
						printf("%s\n", thisEnv);    
					}
				}
				else if (strcmp(arguments[0], "setenv") == 0) {
					char *env_arguments[MAX_COMMAND_LINE_ARGS];
					char equal_delimiter[] = " =";
					int k = 0;
					char *env_token;
					env_token = strtok(arguments[1], equal_delimiter);
					while (env_token != NULL)
					{
						 env_arguments[k] = env_token;
						 env_token = strtok(NULL, equal_delimiter);
						 k++;
					}
					setenv(env_arguments[0], env_arguments[1], 1);
				}
				else {
					// 3. Create a child process which will execute the command line input
					pid = fork();
					int background = 0;
					// Checks if & was passed and sets background variable
					if (arguments[1] != NULL){
						if (strcmp(arguments[1], "&") == 0) {
							background = 1;
							arguments[1] = NULL;
						}
					}
					// If fork() fails it does not create a child and returns -1
					if (pid < 0){
							perror("Fork error!\n");
						exit(1);
					}
					else if (pid == 0){
						signal(SIGINT, handler);		
						int fd0;
						int out=0;
						char output[64];

						// This finds where '>' occurs and make that argv[i] = NULL , to ensure that command wont't read that

						for(i=0;arguments[i]!='\0';i++)
						{
								if(strcmp(arguments[i],">")==0)
								{        
										arguments[i]=NULL;
										strcpy(output,arguments[i+1]);
										out=2;           
								}                       
						}					
						if (out)
						{
								int fd1 ;
								if ((fd1 = creat(output , 0644)) < 0) {
										perror("Couldn't open the output file");
										exit(0);
								}           

								dup2(fd1, 1);
								close(fd1);
						}
						if (execv(arguments[0], &arguments[0]) < 0){
							// Input is not executable.
							perror("Execution error!\n");  
							exit(1); 
						}  
					}
					// 3. This is the parent process, which should wait for the child to complete.
					else {
						signal(SIGINT, handler);
						//register handler to handle SIGALRM
						signal(SIGALRM,timer);
						//Schedule a SIGALRM for 10 seconds
						alarm(10);
						// Checks for background process being run
						if (background == 1){
							background=0;
						}
						else {
							wait(NULL);
						}
					}
				}
    }
    // This should never be reached.
    return -1;
}

