/* Kristen Harrison, CS 344, smallsh 
*/

#include "functions.h"





int main()
{
	/* put sigtstp in a set for later use with sigprocmask */
	sigset_t sigtstp_set;
	sigemptyset(&sigtstp_set);
	sigaddset(&sigtstp_set, SIGTSTP);

	/* define sigint and sigtstp handlers */	
	struct sigaction sigint_action = {{0}}, sigtstp_action = {{0}};
	initSigHandlers(&sigint_action, &sigtstp_action);

	/* store user command arguments into arg array */
	char command[2048];
	memset(command, '\0', sizeof(command));
	char* argArray[512];
	
	/* prompt stores args and returns number of arguments */
	int numArgs = promptUser(command, argArray, &sigtstp_set);

	/* keep track of last exit value */
	int exitStatus = 0;
	int termSignal;
	/* bool for exit type: normal exit 1, sigterm 0 */
	int lastPsExited = 1; 

	/* keep array of all active background PIDs  */
	int bgChildPIDs[100];
	int activeBgPIDs = 0;

	

	/* loop until user enters exit */
	while (strcmp(argArray[0], "exit") != 0)
	{
		int childExitStatus = -5;
		/* track whether last arg is & */
		int backgroundFlag = 0;


		/* ignore comment or blank line */
		if (command[0] == '#' || command[0] == '\0'){
		} 
		/* print out the exit status or the terminating 
		signal of the last foreground process run by shell */
		else if (strcmp (argArray[0], "status") == 0){

			/* determine how last process exited */
			if (lastPsExited){
				printf("exit value %i\n", exitStatus); fflush(stdout);
			} 
			else {
				printf("terminated by signal %i\n", termSignal); fflush(stdout);
			}
		}
		/* change working directory of shell */
		else if (strcmp(argArray[0], "cd") == 0){
			int result = changeDir(argArray, numArgs);
			
			/* error checking */
			if (result != 0){
				perror("Directory change failed");
			}
		}
		/* otherwise execute non-built-in command */
		else {
			pid_t spawnPID = -5;

			/* check if it should be run in the background 
			(last arg is & and we are not in foreground only mode) */
			if (strcmp (argArray[numArgs - 1], "&") == 0 && !foreground_only){
				backgroundFlag = 1;
			}

			/* create child process */
			spawnPID = fork();


			if (spawnPID == -1){
				perror("Fork failed");
				exit(1);
			}
			/* spawnPID of 0 means we are in the child process */
			else if (spawnPID == 0){

				/* change sigint handler to default action for foreground child */
				if (!backgroundFlag){
					sigint_action.sa_handler = SIG_DFL;
					sigaction(SIGINT, &sigint_action, NULL);
				}

				/* pointers for IO redirection */
				char* fileIn = NULL;
				char* fileOut = NULL;

				/* bool used to skip redirection files in array copy */
				int skipNextArg = 0;

				/* new arg array without file operators or & */
				char* newArgs[numArgs];
				int newNumArgs = 0;

				/* step through array and look for < > & */
				for (int i = 0; i < numArgs; i++){
					/* update IO redirection pointers and 
					don't copy over operators */
					if (strcmp (argArray[i], "<") == 0){
						fileIn = argArray[i+1];
						skipNextArg = 1;
					}
					else if (strcmp (argArray[i], ">") == 0){
						fileOut = argArray[i+1];
						skipNextArg = 1;
					}
					else if (strcmp (argArray[i], "&") == 0){
					}
					/* otherwise copy over arg into new array */
					else {
						if (!skipNextArg){
							newArgs[newNumArgs] = argArray[i];
							/* next index is sentinal for end of args */
							newArgs[newNumArgs + 1] = NULL;
							newNumArgs++;
						}
						else {
							/* reset flag after skip */
							skipNextArg = 0;
						}
					}
				} 

				/* file descriptors for redirection */
				int sourceFD, targetFD, result = 0;

				/* redirect IO with dup2; if can't open, 
				print the error and set exit status to 1 */
				if (fileIn){
					/* input file opened for reading only */
					sourceFD = open(fileIn, O_RDONLY);
					if (sourceFD == -1){
						perror("Source file failed to open");
						exit(1);
					}
					result = dup2(sourceFD, 0);
					if (result == -1){
						perror("Source dup2() error");
						exit(2);
					}
				/* if no stdin redirection is specified for a background
				process, redirect stdin to come from  /dev/null */
				} else if (backgroundFlag){
					sourceFD = open("/dev/null", O_RDONLY);
					if (sourceFD == -1){
						perror("/dev/null failed to open");
						exit(1);
					}
					result = dup2(sourceFD, 0);
					if (result == -1){
						perror("/dev/null dup2() error");
						exit(2);
					}
				}

				/* open output file for writing only; 
				truncate if already exists, or create if not */
				if (fileOut){
					targetFD = open(fileOut, O_WRONLY | O_CREAT | O_TRUNC, 0644);
					if (targetFD == -1){
						perror("Target file failed to open");
						exit(1);
					}
					result = dup2(targetFD, 1);
					if (result == -1){
						perror("Redirection error");
						exit(2);
					}
				/* if no stdout redirection is specified for a background
				process, redirect stdout to go to /dev/null */
				} else if (backgroundFlag){
					targetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
					if (targetFD == -1){
						perror("/dev/null failed to open");
						exit(1);
					}
					result = dup2(targetFD, 1);
					if (result == -1){
						perror("Redirection error");
						exit(2);
					}
				}

				/* if exec fails, print error and have child terminate 
				self with error code */
				if (execvp(newArgs[0], newArgs) < 0){
					perror("Exec call failed");
					exit(1);
				} 
			}


			
			/* run in background and move on to next user prompt */
			if (backgroundFlag){
				bgChildPIDs[activeBgPIDs] = spawnPID;
				activeBgPIDs++;
				printf("background pid is %i\n", spawnPID); fflush(stdout);
			}
			/* wait until foreground child process terminates */
			else {
				waitpid(spawnPID, &childExitStatus, 0);

				/* check exit status */
				if (WIFEXITED (childExitStatus) != 0){
					/* update exit tracker */
					exitStatus = WEXITSTATUS(childExitStatus);
					lastPsExited = 1;
				}
				if (WIFSIGNALED(childExitStatus) != 0){
					/* update signal tracker */
					termSignal = WTERMSIG(childExitStatus);
					printf("terminated by signal %i\n", termSignal); fflush(stdout);
					lastPsExited = 0;
				}
			}
		}


		/* check for and print out completed backgrd child processes
		before next prompt */
		for (int i = 0; i < activeBgPIDs; i++){
			int childPID = waitpid(bgChildPIDs[i], &childExitStatus, WNOHANG);

			/* if childPID is nonzero, the process finished */
			if (childPID){
				printf("background pid %i is done: ", bgChildPIDs[i]); fflush(stdout);

				/* switch index of completed pid with the last index 
				to maintain contiguous array */
				bgChildPIDs[i] = bgChildPIDs[activeBgPIDs - 1];
				activeBgPIDs--;

				/* if exited normally */
				if (WIFEXITED (childExitStatus) != 0){
					exitStatus = WEXITSTATUS(childExitStatus);
					printf("exit value %i\n", exitStatus); fflush(stdout);
				}
				/* if was terminated by signal */
				if (WIFSIGNALED(childExitStatus) != 0){
					termSignal = WTERMSIG(childExitStatus);
					printf("terminated by signal %i\n", termSignal); fflush(stdout);
				}
			}
		}


		/* clear arg array */
		for (int i = 0; i < numArgs; i++){
			argArray[i] = NULL;
		}
		numArgs = 0;

		/* let Ctrl-Z through now that foreground child finished */
		sigprocmask(SIG_UNBLOCK, &sigtstp_set, NULL);

		/* get next command */
		numArgs = promptUser(command, argArray, &sigtstp_set);
	}


	/* kill child processes at the end before shell exits */
	for (int i = 0; i < activeBgPIDs; i++){
		kill(bgChildPIDs[i], SIGTERM);
		waitpid(bgChildPIDs[i], NULL, 0);	
	}

	return 0;
}


