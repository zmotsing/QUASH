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
	printf("[%s]$ ", get_current_dir_name());
	str_input = (char *) malloc (bsize + 1);
	bytes_in = getline(&str_input, &bsize, stdin);

	if(bytes_in == -1)
		printf("Error reading input.\n");

	/* Tokenize the string and remove whitespace */
	int i=0;
	arr_input[i] = strtok(str_input," \n");

	while(arr_input[i] != NULL)
		arr_input[++i] = strtok(NULL," \n");
		
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

int main(int argc, char *argv[], char *envp[])
{
	while(true)
	{
		char* input[100];
		bool bg_proc = prompt_user(input);	

		if(strcmp(input[0],"quit") == 0 || strcmp(input[0],"exit") == 0)
		{
			exit(0);
		}
		else if(strcmp(input[0], "cd") == 0)
		{
			if(input[1] == NULL)
				chdir(getenv("HOME"));
			else
				if(chdir(input[1]) == -1)
					printf("No such file or directory \"%s\"\n", input[1]);
		}
		else if(strcmp(input[0], "echo") == 0)
		{
			if(input[1] != NULL)
			{
				char* target = input[1];
			
				if(target[0] == '$')
					target = getenv(++target);
				
				printf("%s\n\n", target);
			}
			else
				printf("\n\n");
		}
		else
		{
			pid_t child_pid;
			child_pid = fork();

			/* Child process */
			if(child_pid == 0)
			{
				/* Execute the command */
				if(execvpe(input[0], input, environ) == -1)
					printf("Error: command \"%s\" not on path.\n", input[0]);

				exit(0);
			}

			/* Wait for child to finish unless running in the background */
			int status = 0;
			if(!bg_proc)
				waitpid(child_pid, &status, 0);
		}

		/* Reset the input */
		int i;
		while(input[i] != NULL)
			input[i++] = NULL;
	}
	
	return 0;
}
