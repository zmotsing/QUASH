#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

extern char **environ;

void prompt_user(char *arr_input[])
{
	int bytes_in;
	size_t bsize = 256;
	char* str_input;

	printf("[QUASH]$ ");
	str_input = (char *) malloc (bsize + 1);
	bytes_in = getline(&str_input, &bsize, stdin);

	if(bytes_in == -1)
		printf("Error reading input.\n");

	int i=0;
	arr_input[i] = strtok(str_input," \n");

	while(arr_input[i] != NULL)
		arr_input[++i] = strtok(NULL," \n");
}

int main(int argc, char *argv[])
{
	char* input[100];
	prompt_user(input);

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
				prompt_user(input);
		}

		//Child process
		if(child_pid==0)
		{
			if(execvpe(input[0], input, environ) == -1)
				printf("Error: command not on path.\n");
			
			exit(0);
		}
	}
	
	return 0;
}
