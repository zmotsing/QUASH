#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Struct to track all background jobs */
struct job 
{
	int jid;
	int pid;
	char* cmd;
	struct job* prev;
};

struct job* last_job;

/* Cleaning up background jobs after they have ended */
void remove_job(struct job* bg_job)
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

/* User input */
void prompt_user(char* arr_input[], int is_redirected)
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
		
	/* Replace env vars with their values */
	for(i = 0; arr_input[i] != NULL; i++)
	{
		if(arr_input[i][0] == '$')
			arr_input[i] = getenv(++arr_input[i]);
	}
}

/* Generate conditions */
void fill_conditions(char* input[], int &out_fd, bool &bg_proc)
{
	int i=0;
	while(input[i])
	{
		/* Background process */
		if(!strcmp(input[i], "&"))
		{
			bg_proc = true;
			input[i] = NULL;
		}
		/* Output redirection */
		else if(!strcmp(input[i], ">"))
		{
			/* Open file or create it if necessary with read/write access */
			out_fd  = open(input[i+1], O_RDWR | O_CREAT, 0666);
			input[i] = NULL;
		}
		
		i++;
	}
}

/* cd */
void cd(char* input[])
{
	if(input[1] == NULL)
		chdir(getenv("HOME"));
	else if(chdir(input[1]) == -1)
			printf("No such file or directory \"%s\"\n", input[1]);
}

/* jobs */
void jobs(char* input[])
{
	struct job* temp = last_job;
	while(temp)
	{
		printf("[%i] %i %s\n", temp->jid, temp->pid, temp->cmd);		
		temp = temp->prev;
	}
}

/* set */
void set(char* input[])
{
	char* target = strtok(input[1], "=");
	char* vars = strtok(NULL, "");
	
	setenv(target, vars, 1);
}

/* Child Process */
void child(char* input[], int out_fd)
{
	/* If output has been redirected */
	dup2(out_fd, STDOUT_FILENO);

	/* Execute the command */
	if(execvpe(input[0], input, environ) == -1)
		printf("Error: command \"%s\" not on path.\n", input[0]);

	close(out_fd);
	exit(0);
}

/* Parent process */
void parent(char* input[], pid_t child_pid, bool bg_proc)
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
			struct job* bg_job = last_job;
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

/* Execute */
void execute(char* input[], int &out_fd, bool &bg_proc)
{
	if(!strcmp(input[0],"quit") || !strcmp(input[0],"exit"))
		exit(0);
	else if(!strcmp(input[0], "cd"))
		cd(input);
	else if(!strcmp(input[0], "jobs"))
		jobs(input);
	else if(!strcmp(input[0], "set"))
		set(input);
	else
	{
		pid_t child_pid;
		child_pid = fork();

		if(child_pid == 0)
			child(input, out_fd);
		else
			parent(input, child_pid, bg_proc);
	}
}

/* Pipes */
bool split_pipe(char* input[])
{
	/* Check if we need to split */
	int pipe_loc = 0;
	int i = 0;
	while(input[i])
	{
		if(!strcmp(input[i], "|"))
		{
			pipe_loc = i;
			break;
		}

		i++;
	}

	if(pipe_loc == 0)
		return false;

	/* Split the data into two pipes */
	char* left_pipe[1000];
	char* right_pipe[1000];

	for(i=0; i<pipe_loc; i++)
		left_pipe[i] = input[i];

	for(i=pipe_loc+1; input[i]; i++)
		right_pipe[i-pipe_loc-1] = input[i];

	int left_out_fd = STDOUT_FILENO;
	int right_out_fd = STDOUT_FILENO;

	bool left_bg_proc = false;
	bool right_bg_proc = false;

	fill_conditions(left_pipe, left_out_fd, left_bg_proc);
	fill_conditions(right_pipe, right_out_fd, right_bg_proc);

	/* Execute both pipes */
	int pipe_fd[2];
	pipe(pipe_fd);
	pid_t pid_1, pid_2;
	
	pid_1 = fork();
	if(pid_1 == 0)
	{
		close(pipe_fd[0]);
		dup2(pipe_fd[1], STDOUT_FILENO);
		execute(left_pipe, left_out_fd, left_bg_proc);
		close(pipe_fd[1]);
		exit(0);
	}
	
	pid_2 = fork();
	if(pid_2 == 0)
	{
		close(pipe_fd[1]);
		dup2(pipe_fd[0], STDIN_FILENO);
		execute(right_pipe, right_out_fd, right_bg_proc);
		close(pipe_fd[0]);
		exit(0);
	}

	return true;
}

int main(int argc, char *argv[], char *envp[])
{
	/* True if redirected to a file for input */
	int is_redirected = isatty(STDIN_FILENO)-1;

	while(true)
	{
		/* User input */
		char* input[1000];
		prompt_user(input, is_redirected);

		/* No input */
		if(input[0] == NULL)
			return 0;

		/* Check for piping */
		if(split_pipe(input))
			continue;

		/* Conditions */
		int out_fd = STDOUT_FILENO;
		bool bg_proc = false;
		fill_conditions(input, out_fd, bg_proc);

		/* Execute */
		execute(input, out_fd, bg_proc);

		/* Reset the input */
		int i=0;
		while(input[i] != NULL)
			input[i++] = NULL;
	}
	
	return 0;
}
