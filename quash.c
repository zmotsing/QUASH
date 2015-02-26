#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>

#define BSIZE 256
#define BASH_EXEC  "/bin/bash"
#define FIND_EXEC  "/bin/find"
#define XARGS_EXEC "/usr/bin/xargs"
#define GREP_EXEC  "/bin/grep"
#define SORT_EXEC  "/bin/sort"
#define HEAD_EXEC  "/usr/bin/head"

int main(int argc, char *argv[])
{
	pid_t child_pid;
	printf ("the main program process ID is %d\n", (int)getpid());
	bool done = false;
	while(!done)
		
		/* Duplicate this process. */
		child_pid = fork();
		if(child_pid != 0)
			/* This is the parent process. */
			return child_pid;
		else
		{
			/* Now execute PROGRAM, searching for it in the path. */
			execvp("ls", argv);
			/* The execvp function returns only if an error occurs. */
			fprintf(stderr, "\nError execing find. ERROR#%d\n", errno);
			abort();
		}
	}
	return 0;
}
