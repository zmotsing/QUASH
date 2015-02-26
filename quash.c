#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

extern char **environ;

char* prompt_user()
{
	int bytes_in;
	size_t bsize = 256;
	char *input;

	printf("[QUASH]$ ");
	input = (char *) malloc (bsize + 1);
	bytes_in = getline(&input, &bsize, stdin);

	if(bytes_in == -1)
		printf("Error reading input.\n");
	else
		input[strlen(input)-1] = 0;

	return input;
}

int main(int argc, char *argv[])
{
	char* input;

	input = prompt_user();

	bool run = true;
	while(run)
	{
		pid_t child_pid;
		child_pid = fork();

		//Parent process
		if(child_pid!=0)
		{
			int status;
			waitpid(child_pid, &status, 0);

			if(status == 0)
				input = prompt_user();
		}

		//Child process
		if(child_pid==0)
		{
			if(execvpe(input, argv, environ) == -1)
				printf("Error: command not on path.\n");
			
			exit(0);
		}
	}
	
	return 0;
}
