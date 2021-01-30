//------------------------------------
//	CS 270 - A Simple Shell
//------------------------------------
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "shell.h"

// Function for external commands and redirection
// Based on previous code used in Lab 2, call_redirected function
int forkThis(char **args, char *infile, char *outfile){
        pid_t result = fork();

        // error
        if (result < 0) {
                fprintf(stderr, "mysh: fork failed");
                exit(EXIT_FAILURE);
        }
        // child
        if (result == 0) {
		// opens and moves infile to file descriptor using dup2()
		// if there was no infile, does not execute
		if (infile != NULL) {
			int infd = open(infile, O_RDONLY);
			// error message for opening file
			if (infd < 0) {
                	        perror("Error opening file");
        	                exit(EXIT_FAILURE);
	                }

			int dup2ReturnInfile;
			dup2ReturnInfile = dup2(infd, 0);
			// error message for duplicating file
			if (dup2ReturnInfile < 0) {
                	        perror("Error duplicating file");
        	                exit(EXIT_FAILURE);
	                }

			close(infd);	
		}
		// opens and moves outfile to file descriptor using dup2()
		// if there was no outfile, does not execute
		if (outfile != NULL) {
			// extra flags and magic number needed for outfile
                	int outfd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                	// error message for opening file
			if (outfd < 0) {
                        	perror("Error opening file");
                	        exit(EXIT_FAILURE);
                	}
	
        	        int dup2ReturnOutfile;
	                dup2ReturnOutfile = dup2(outfd, 1);
			// error message for duplicating file
                	if(dup2ReturnOutfile < 0) {
                        	perror("Error duplicating file");
                        	exit(EXIT_FAILURE);
                	}

                	close(outfd);
		}
		
		// executes command using args with args[0] describing command
                execvp(args[0], args);
		perror("execvp");
                exit(EXIT_FAILURE);	
        }
        // parent
        else {
		// initializes exitStatus, waitResult, and processID
                int exitStatus = 0;
                pid_t processID = result;
                int waitResult;

		// signal statements cause Ctrl-C to be ignored, restored after waitpid finishes
		signal(SIGINT, SIG_IGN);
                waitResult = waitpid(processID, &exitStatus, WUNTRACED);
		signal(SIGINT, SIG_DFL);

		// error with the waitpid
                if (waitResult < 0) {
                        perror("waitpid");
                        exit(EXIT_FAILURE);
                }
                else {
			// child exited
                        if (WIFEXITED(exitStatus)) {
				// error message for child exit with nonzero status
				if (WEXITSTATUS(exitStatus) != 0) {
					fprintf(stderr, "Command returned %d\n", exitStatus);
				}
			}
			// child killed by signal
			else if (WIFSIGNALED(exitStatus)) {
				// uses psignal with WTERMSIG macro to print to stderr
				psignal(WTERMSIG(exitStatus), "Child was terminated because of:");
			}
			return 1;
                }
        }
}

// used for cd and cd dir built-in commands
int cdCall(char **dir) {
	// called with cd only
	if (dir[1] == NULL) {
		// case for if HOME environment was removed
		if (getenv("HOME") == NULL) {
			fprintf(stderr, "mysh: HOME environment variable has been removed\n");
		}
		// changes back to home directory if HOME was not removed
		else {
			chdir(getenv("HOME"));
		}
	}
	// called with cd dir
	else {
		// changes directory to one specified by dir[1]
		int cdCheck = chdir(dir[1]);
		// error for changing directories
		if (cdCheck != 0) {
			perror("mysh");
		}
	}
	return 1;
}

// used for all setenv commands
int setenvCall(char **args) {
	// case for no arguments supplied, print error message
	if (args[1] == NULL) {
		fprintf(stderr, "Error: no arguments were supplied to setenv\n");
	}
	// case for setenv variable only
	// unsets the passed in variable
	else if (args[2] == NULL) {
		unsetenv(args[1]);
	}
	// case for setenv variable value
	// sets environment variable using supplied arguments
	else {
		int checkSetenv = setenv(args[1], args[2] , 1);
		// error setting the variable
		if (checkSetenv != 0) {
			perror("setenv error");
		}
	}
	return 1;
}

// simple exit call that changes status to zero to end loop and the shell
int exitCall() {
	return 0;
}

// reads in a line from stdin
// really could have done this all in the main but I modified this function to use getline after seeing how simple it was in class
char *readLine() {
	char *line;
	size_t bufferSize;
	getline(&line, &bufferSize, stdin);
	return line;
}

int main(int argc, char **argv) {
	// creates a library of built-in command strings to compare to
	char *builtIns[] = {"cd", "setenv", "exit"};
		
	// line to store user or file input
	char *line;
	// the seperated user input from calling parse_command
	struct command *input;
	// status of the shell, exits shell if ever turned to 0
	int status = 1;

	// only one argument supplied means only ./shell was called
	if (argc == 1) {
        	// reads in input from stdin as long as status is 1
		do {
			// outputs prompt, reads it, and separates it
			fprintf(stderr, "shell> ");
			line = readLine();
			input = parse_command(line);
	
			// if nothing was input, jumps to freeing the command
			if (input->args[0] == NULL) {
				goto free;
			}
			
			// compares the first argument to all built-ins
			// if no built-ins match, must be external command
			if (strcmp(input->args[0], builtIns[0]) == 0) {
				status = cdCall(input->args);
			}
			else if (strcmp(input->args[0], builtIns[1]) == 0) {
				status = setenvCall(input->args);
			}
			else if (strcmp(input->args[0], builtIns[2]) == 0) {
				status = exitCall();
			}
			else {
				status = forkThis(input->args, input->in_redir, input->out_redir);
			}

			// frees the command for next use
			free:
				free_command(input);
		} while (status == 1);	
	}
	// more than one argument means ./shell scriptfile was called
	else {
		// declares necessary variables for reading in file
		// initializes the file pointer to the scriptfile in read mode
		size_t len = 0;
		ssize_t read;
		FILE * fp = fopen(argv[1], "r");

		// kills everything if file cannot be accessed
		if (fp == NULL) {
			exit(EXIT_FAILURE);
		}

		// executes commands from scriptfile as long as there is a command to execute and the status is 1
		// passes line by reference
		while ((read = getline(&line, &len, fp)) != -1 && status == 1) {
			// separates the command
			input = parse_command(line);

			// frees command if no argument was supplied
	               	if (input->args[0] == NULL) {
        	               	goto free2;
                	}

			// compares the first argument to all built-ins
                        // if no built-ins match, must be external command
                	if (strcmp(input->args[0], builtIns[0]) == 0) {
                                status = cdCall(input->args);
                        }
                        else if (strcmp(input->args[0], builtIns[1]) == 0) {
                                status = setenvCall(input->args);
                        }
                        else if (strcmp(input->args[0], builtIns[2]) == 0) {
                                status = exitCall();
                    	}
                        else {
                                status = forkThis(input->args, input->in_redir, input->out_redir);
                        }

			// frees the command for next use
                        free2:
                        	free_command(input);
		}
		fclose(fp); // closes the file pointer
	}
	exit(EXIT_SUCCESS); // ends shell successfully
}
