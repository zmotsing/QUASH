#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

extern char **environ;

bool prompt_user(char *arr_input[])
{
	int bytes_in;
	size_t bsize = 256;
	char* str_input;

	/* Get the input from the user */
	printf("$ ");
	str_input = (char *) malloc (bsize + 1);
	bytes_in = getline(&str_input, &bsize, stdin);

	if(bytes_in == -1)
		printf("Error reading input.\n");

	/* Tokenize the string and remove whitespace */
	int i=0;
	arr_input[i] = strtok(str_input," \n");

	while(arr_input[i] != NULL)
		arr_input[++i] = strtok(NULL," \n");
		
	if(strcmp(arr_input[0],"quit")==0 || strcmp(arr_input[0],"exit")==0)
		exit(0);

	/* Check to see if this is a background process */
	bool bg_proc = false;

	i=0;
	while(arr_input[i] != NULL)
	{
		if(strcmp(arr_input[i], "&") == 0)
		{
			bg_proc = true;
			arr_input[i] = 0;	
			break;
		}
		
		i++;
	}	
		
	return bg_proc;
}

int main(int argc, char *argv[])
{
	char* input[100];

	bool bg_proc = prompt_user(input);	

	bool run = true;
	while(run)
	{
		pid_t child_pid;
		child_pid = fork();

		/* Parent process */
		if(child_pid!=0)
		{
			/* Wait for child to finish unless running in the background */
			int status = 0;
			
			if(!bg_proc)
				waitpid(child_pid, &status, 0);
			
			if(status == 0)
			{			
				/* Reset the input */
				/* TODO: Fix memory leak here */
				int i;
				for(i = 0; i<100; i++)
					input[i] = 0;

				bg_proc = prompt_user(input);
		
				/* Stop child to quit */
				//if(!run)
					//kill(getppid(),SIGCHLD);
			}
		}

		/* Child process */
		if(child_pid==0)
		{
			/* Execute the command */
			if(execvpe(input[0], input, environ) == -1)
				printf("Error: command \"%s\" not on path.\n", input[0]);
			
			exit(0);
		}
	}
	
	return 0;
}
