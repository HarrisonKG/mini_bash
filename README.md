Kristen Harrison  
344 -- Operating Systems  
Project 3  
  
I wrote my own simple shell in C, similar to bash, which supports three built-in commands (exit, cd, and status), and handles all others by forking the process and exec'ing the command.   

It supports foreground and background processes, and signals are modified so that SIGINT (Ctrl-C) terminates only the foreground command if one is running and leaves the shell intact. SIGTSTP (Ctrl-Z) toggles back and forth between background-enabled and foreground-only modes.   
  
You can run it with the commands  make smallsh  and  ./smallsh    
