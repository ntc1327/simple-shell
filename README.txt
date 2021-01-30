AUTHOR:
Noah Cribelar, ntcr222@uky.edu
Date Finished: 4/15/19

CONTENTS:
Makefile - provided file to compile program
shell.h - provided file with function declarations
parser.c - provided file with implementations of shell.h declarations
shell.c - my file with implementation of majority of shell functions
README.txt - this file
run-tests - executible to run necessary tests on shell
tests - directory including expected outputs and error outputs

RUNNING:
To compile, enter make into the command line.
To clean object files, enter make clean into the command line.

To run shell with stdin input, make sure it has been compiled with make then enter ./shell into the command line.

To run with scriptfile, again make sure it has been compiled then enter ./shell scriptfile into the command, replacing scriptfile with the name of the you want to run.

To run the tests, make sure it has been compiled then enter ./run-tests into the command line.

IMPLEMENTATION NOTES:
There is a main loop that takes care of most of the stdin or file input, I decided again separating those loops into separate files due to how much was not actually in main. 

One small readIn function for helping with stdin, check comment in program for extra detail.

The built-ins were made in 3 seperate function with each corresponding to which base command was called (cdCall for both cd and cd dir, setenvCall for setenv variable value and setenv variable). To determine if something was a built-in, I made small library of char * variables to compare to the first argument from the command line. If a variable matched, it called the correct built-in. If not, that meant it was an external command and I sent everything to the fork function.

The external command function was largely carried over from my Lab 2 so I also credit anyone I had listed in my Lab 2 documentation for helping with this one. Changes to this version included making it able to read in an infile, changing to execvp, and signal interruption.

LIMITATIONS:
I believe there are no limitations to this shell for the scope of the project. My own testing revealed nothing of note as well.

REFERENCES:
For learning how to use errno, fprintf, and perror better, I credit the following website: https://www.tutorialspoint.com/cprogramming/c_error_handling.htm

For learning how to use psignal, I credit the Linux manual for PSIGNAL(3).

For learning how to read in a file and use getline with it, I credit the first StackOverflow answer here: https://stackoverflow.com/questions/3501338/c-read-file-line-by-line

I did practically copy a small amount of the code needed to open and read the file so all credit to user mbaitoff on StackOverflow.
