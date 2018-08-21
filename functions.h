
#ifndef FUNCTIONS_H
#define FUNCTIONS_H


/* enable getline */
#define _GNU_SOURCE 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


/* bool flag indicates whether we are in foreground only mode (Cntl-Z) */
extern int foreground_only;


/* expand '$$' in command into process ID of the shell */
void expandPID(char command[], char expanded_cmd[]);

/* updates array of user commands and returns the number of arguments*/
int promptUser(char command[], char* argArray[], sigset_t* sigtstp_set);

/* built-in cd command */
int changeDir(char* argArray[], int numArgs);

/* initialize the Ctrl-C and Ctrl-Z signal action structs and handlers */
void initSigHandlers(struct sigaction* sigint_action, struct sigaction* sigtstp_action);

/* Ctrl-Z signal handler */
void catchSIGTSTP();


#endif