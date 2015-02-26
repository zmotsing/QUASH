#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int bytes_in;
	size_t bsize = 256;
	char *input;

	bool run = true;
	while(run)
	{
		printf("[QUASH]$ ");
		input = (char *) malloc (bsize + 1);
		bytes_in = getline(&input, &bsize, stdin);
	
		if(bytes_in == -1)
			printf("Error reading input.\n");
		else
			input[strlen(input)-1] = 0;

		pid_t child_pid;
		child_pid = fork();

		if(child_pid!=0)
			return child_pid;
		else
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
				run = false;

			exit(0);
		}
	}
	
	return 0;
}
