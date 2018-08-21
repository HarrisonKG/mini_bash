
#include "functions.h"





/* initialize foreground-only mode to false */
int foreground_only = 0;



/* expand '$$' in command into process ID of the shell 
several lines adapted from https://www.linuxquestions.org/questions/programming-9/replace-a-substring-with-another-string-in-c-170076/ */
void expandPID(char command[], char expanded_cmd[])
{
	/* point to first instance of "$$" in string */
	char* pidInsertPt;
	
	/* if command contains "$$" substring */
	while ((pidInsertPt = strstr(command, "$$"))){
		/* pointer arithmetic to find length from start to $$ */
		int firstChunk = pidInsertPt - command;
		
		/* copy characters from start of cmd string to the 
		beginning of the $$ substring */
		strncpy(expanded_cmd, command, firstChunk);
		expanded_cmd[firstChunk] = '\0';

		/* append pid and rest of command string past the $$ */ 
		sprintf(expanded_cmd + firstChunk, "%d%s", getpid(), pidInsertPt + 2);

		/* update command with the expansion for next loop */
		strcpy(command, expanded_cmd);
	}
}




/* updates array of user commands and returns the number of arguments*/
int promptUser(char command[], char* argArray[], sigset_t* sigtstp_set)
{
	/* number of user-entered command arguments */
	int numArgs = 0;
	
	/* set up buffer for getline */
	char* line = NULL;
	size_t lineSize = 0;
	
	/* prompt for input */
	printf(": "); fflush(stdout);
	/* get user line and save to command string */
	int numCharsEntered = getline(&line, &lineSize, stdin);
	
	/* loop to get user input */
	while(numCharsEntered == -1){
		/* if getline was interrupted, clear error and re-prompt */
		clearerr(stdin); 
		printf(": "); fflush(stdout);
		numCharsEntered = getline(&line, &lineSize, stdin);
	}

	/* save to char array */
	strncpy(command, line, numCharsEntered - 1);
	command[numCharsEntered - 1] = '\0';

	/* string copy for pid expansion and strtok */
	char expanded_cmd[2048];
	strcpy(expanded_cmd, command);

	/* convert "$$" in command string to PID */
	expandPID(command, expanded_cmd);	

	/* divide copy string on whitespace into array of arguments */
	argArray[0] = strtok(expanded_cmd, " ");

	while (argArray[numArgs] != NULL){
		numArgs++;
		argArray[numArgs] = strtok(NULL, " ");
		/* make next arg NULL to create stopping point for exec */
		argArray[numArgs + 1] = NULL;
	}

	/* make sure argArray[0] is not null pointer */
	if(!argArray[0]){
		argArray[0] = " ";
		numArgs++;
	}

	/* block SIGTSTP so that it won't kill foreground proc */
	sigprocmask(SIG_BLOCK, sigtstp_set, NULL);

	free(line);
	return numArgs;
}




/* built-in cd command */
int changeDir(char* argArray[], int numArgs)
{
	int result; 

	/* cd by itself changes to HOME directory */
	if (numArgs == 1){
		if (getenv("HOME") != NULL){
			result = chdir(getenv("HOME"));
		} else {
			perror("HOME variable couldn't be accessed");
		}
	} 
	/* or takes one argument: the directory path to change to */
	else {
		result = chdir(argArray[1]);
	}
	return result;
}




/* initialize the Ctrl-C and Ctrl-Z signal action structs and handlers */
void initSigHandlers(struct sigaction* sigint_action, struct sigaction* sigtstp_action)
{
	/* set sigint to be ignored and custom handler for sigtstp */
	sigint_action->sa_handler = SIG_IGN;
	sigtstp_action->sa_handler = catchSIGTSTP;

	/* block signals while executing handler */
	sigfillset(&sigint_action->sa_mask);
	sigfillset(&sigtstp_action->sa_mask);

	/* restart interrupted process for sig int */
	sigint_action->sa_flags = SA_RESTART;
	sigtstp_action->sa_flags = 0;

	/* register sigint ignore and sigtstp handler */
	sigaction(SIGINT, sigint_action, NULL);
	sigaction(SIGTSTP, sigtstp_action, NULL);
}




/* Ctrl-Z signal handler */
void catchSIGTSTP()
{
	/* toggle mode */
	foreground_only = 1 - foreground_only;

	/* print informative messages to stdout */
	if (foreground_only == 1){
		char* msg1 = "\nEntering foreground-only mode (& is now ignored)\n";
		write(2, msg1, 50);
	}	
	else if (foreground_only == 0){
		char* msg2 = "\nExiting foreground-only mode\n";
		write(2, msg2, 30);
	}
}

