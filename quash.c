#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>

extern char **environ;

struct job 
{
	int jid;
	int pid;
	char* cmd;
	job* prev;
};

job* last_job;

void remove_job(job* bg_job)
{
	if(bg_job == last_job)
	{
		last_job = bg_job->prev;
		free(bg_job);
		return;
	}

	struct job* tmp_job = last_job;
	while(tmp_job->prev != NULL)
	{
		if(tmp_job->prev == bg_job)
		{
			tmp_job->prev = bg_job->prev;
			free(bg_job);
			return;
		}		

		tmp_job = tmp_job->prev;
	}
}

bool prompt_user(char *arr_input[], int is_redirected)
{
	int bytes_in;
	size_t bsize = 256;
	char* str_input;

	/* Input is coming from a file */
	if(is_redirected)
	{		
		str_input = (char *) malloc (bsize + 1);
		bytes_in = getline(&str_input, &bsize, stdin);
		
		/* Check for eof and terminate if met */
		if(bytes_in == EOF)
			exit(0);
	}
	/* Input is coming from the user */
	else
	{
		/* Get the input from the user */
		printf("[%s]$ ", get_current_dir_name());
		str_input = (char *) malloc (bsize + 1);
		bytes_in = getline(&str_input, &bsize, stdin);

		/* Check for erroneous input */
		if(bytes_in == -1)
			printf("Error reading input.\n");
	}

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
			arr_input[i] = NULL;	
			break;
		}
		
		i++;
	}	
		
	return bg_proc;
}

int main(int argc, char *argv[], char *envp[])
{
	/* Will evaluate to true if redirected to a file for input */
	int is_redirected = isatty(STDIN_FILENO)-1;
	
	while(true)
	{
		char* input[100];
		bool bg_proc = prompt_user(input, is_redirected);	

		/* quit and exit */
		if(strcmp(input[0],"quit") == 0 || strcmp(input[0],"exit") == 0)
		{
			exit(0);
		}
		/* cd */
		else if(strcmp(input[0], "cd") == 0)
		{
			if(input[1] == NULL)
				chdir(getenv("HOME"));
			else
				if(chdir(input[1]) == -1)
					printf("No such file or directory \"%s\"\n", input[1]);
		}
		/* echo */
		else if(strcmp(input[0], "echo") == 0)
		{
			if(input[1] != NULL)
			{
				char* target = input[1];
			
				if(target[0] == '$')
					target = getenv(++target);
				
				printf("%s\n", target);
			}
			else
				printf("\n");
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
/* Parent process */
			else
			{
				/* Wait for child to finish unless running in the background */
				int status = 0;
				if(!bg_proc)
					waitpid(child_pid, &status, 0);
				else
				{
					int id = 1;
					if(last_job)
						id = last_job->jid + 1;

					struct job* next = (struct job *)malloc(sizeof (struct job));
					next->jid = id;
					next->pid = child_pid;
					next->cmd = input[0];
					next->prev = last_job;					

					last_job = next;

					printf("[%i] %i running in background.\n", last_job->jid, last_job->pid);
				}

				/* Check if background processes have finished */
				int id = 0;
				do
				{
					id = waitpid(-1, &status, WNOHANG);
					if(id > 0)
					{
						job* bg_job = last_job;
						while(bg_job)
						{
							if(bg_job->pid == id)
							{					
								printf("[%i] %i finished %s\n", bg_job->jid, bg_job->pid, bg_job->cmd);
								remove_job(bg_job);
								break;
							}
						
							bg_job = bg_job->prev;
						}
					}
				} while (id > 0);
			}
		}

		/* Reset the input */
		int i=0;
		while(input[i] != NULL)
			input[i++] = NULL;
	}
	
	return 0;
}

