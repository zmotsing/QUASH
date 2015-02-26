#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

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
			if(strcmp(input, "head") == 0)
				execvp("head", argv);
			else if(strcmp(input, "sort") == 0)
				execvp("sort", argv);
			else if(strcmp(input, "grep") == 0)
				execvp("grep", argv);
			else if(strcmp(input, "xargs") == 0)
				execvp("xargs", argv);
			else if(strcmp(input, "find") == 0)
				execvp("find", argv);
			else if(strcmp(input, "bash") == 0)
				execvp("bash", argv);
			else if(strcmp(input, "ls") == 0)
				execvp("ls", argv);
			else
				exit(0);
		}
	}
	
	return 0;
}
