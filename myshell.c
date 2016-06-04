//Alex Burns
//September 29th, 2015
//CIS 3207 Lab Project 2: creating a shell
 
#include <sys/types.h> //for dir
#include <dirent.h> //for dir
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h> //for open(), close(), 
#include <sys/stat.h> //for userid
#include <pwd.h> //for username
 
#define MYSHELL_TOKEN_DELIM " \t\r\n\a"
#define MYSHELL_TOKEN_SIZE 64
#define BATCH_TOK_DELIM "\n"
 
//forward function declarations for builtin shell commands
int cd(char **args);
int help(char **args);
int myshell_quit(char **args);
int clr(char **args); 
int myshell_pause(char **args);
int echo(char **args);
int dir(char **args);
int myshell_environ(char **args);
 
//Function prototypes 
DIR *opendir(const char *);
struct dirent *readdir(DIR *);
int closedir(DIR *);
void runsource(int pfd[], char **args);
void rundest(int pfd[], char **args2);
int argsLength(char **args);
 
//gloval variable for environ
extern char **environ;
 
//List of I/O redirection commands
char *redirection[] = {
  "<", //input redir
  ">", //output redir
  ">>", //output redir, appends not truncates
  "|", //pipe
  "&" //background
}; 
 
//List of builtin commands
char *builtin_str[] = {
  "cd",
  "help",
  "quit",
  "clr",
  "pause",
  "echo",
  "dir",
  "environ"
};
 
//List of builtin functions
//note: certain functions need myshell, as name is used in other functions
int (*builtin_func[]) (char **) = {
  &cd,
  &help,
  &myshell_quit, 
  &clr,
  &myshell_pause,
  &echo,
  &dir,
  &myshell_environ
};
 
//number of builtin functions
int number_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

//size of char **foo, includes null ptr
int argsLength(char **args) {
  int i = 0;
  int len = 0;
  while(args[i] != NULL) {
    len++;
    i++;
  }
  return len;
}
 
//**********************************************************************************************************************************************************************************
//START: Builtin function implementations
//Each function handles any possible io_redirection (within assignment parameters)
 
//clears screen
int clr(char **args)
{
    if(system("clear") < 0) {
    fprintf(stderr, "clear err\n");
    return 0;
  }
  return 1;
}
 
//lists all the environment strings
int myshell_environ(char **args)
{
  int len = 0;
  int saved_stdout;
  int i = 0;
  int io_output = 0;
  int io_output2 = 0;
  int out;
 
  len = argsLength(args);
 
  for(i=0;i<len;i++) {
    if(strcmp(args[i], redirection[1]) == 0) { io_output=i; }
    else if(strcmp(args[i], redirection[2]) == 0) { io_output2=i; }
  }
  if(io_output > 0) {
    //Save current stdout for use later
    saved_stdout = dup(STDOUT_FILENO);
    //open file for output redirection
    out = open(args[io_output +1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    //replace standard output with output file
    dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
    close(out);
  } else if (io_output2 > 0) {
    //open file for output redirection
    out = open(args[io_output2 +1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    //replace standard output with output file
    dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
    close(out);
  }
 
  i = 0;
  while(environ[i]) {
    printf("%s\n", environ[i]);
    i++;
  }
  //the below code works if comments removed, and can substitute for the above code
  //char **env = environ;
  //for (environ; *env; ++env) {
  //  printf("%s\n", *env);
  //}
 
  //if there was io redirection, redirect back to stdout
  if(io_output > 0 || io_output2 > 0) {
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
  }
  return 1;
}
 
//lists contents of directory <directory>
//set so if no <directory> arg passed, returns "no directory entered"
//could be modified to print current directory if no arg passed
int dir(char **args)
{
  int len = 0;
  int saved_stdout;
  int i = 0;
  int io_output = 0;
  int io_output2 = 0;
  int out;
   
  len = argsLength(args);
  
  for(i=0;i<len;i++) {
    if(strcmp(args[i], redirection[1]) == 0) { io_output=i; }
    else if(strcmp(args[i], redirection[2]) == 0) { io_output2=i; }
  }
  if(io_output > 0) {
    //Save current stdout for use later
    saved_stdout = dup(STDOUT_FILENO);
    //open file for output redirection
    out = open(args[io_output +1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    //replace standard output with output file
    dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
    close(out);
  } else if (io_output2 > 0) {
    //open file for output redirection
    out = open(args[io_output2 +1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    //replace standard output with output file
    dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
    close(out);
  }
 
  if (args[1] == NULL) {
    //char buff[PATH_MAX + 1];
    printf("No directory entered\n");
  } else {
    DIR *dp;
    struct dirent *ep;
    dp = opendir(args[1]);
 
    if (dp != NULL) {
      while (ep = readdir(dp)) {
        puts(ep->d_name);
      }
      (void) closedir (dp);
    } else {
      perror("Couldn't open directory");
    }
  }
  //redirect back to stdout
  if(io_output > 0 || io_output2 > 0) {
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
  }
  return 1;
}
 
//display <comment> on display followed by new line
int echo(char **args)
{
  int len = 0;
  int saved_stdout;
  int i = 0;
  int io_output = 0;
  int io_output2 = 0;
  int out;
   
  len = argsLength(args);
 
  for(i=0;i<len;i++) {
    if(strcmp(args[i], redirection[1]) == 0) { io_output=i; }
    else if(strcmp(args[i], redirection[2]) == 0) { io_output2=i; }
  }
  if(io_output > 0) {
    args[io_output] = NULL;
    //Save current stdout for use later
    saved_stdout = dup(STDOUT_FILENO);
    //open file for output redirection
    out = open(args[io_output +1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    //replace standard output with output file
    dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
    close(out);
  } else if (io_output2 > 0) {
    args[io_output2] = NULL;
    //open file for output redirection
    out = open(args[io_output2 +1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    //replace standard output with output file
    dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
    close(out);
  }
 
 
  i = 2;
  if (args[1] != NULL) {
    printf("%s", args[1]);
  } else {
    printf("\n");
    return 1;
  }
  while(args[i] != NULL) {
    printf(" %s", args[i]);
    i++;
  }
  printf("\n");
  //if there was io redirection, redirect back to stdout
  if(io_output > 0 || io_output2 > 0) {
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
  }
  return 1;
}
 
//simple way to pause shell
int myshell_pause(char **args)
{
    printf("Press [Enter] to unpause shell");
    getchar();
    return 1;
}
 
//change current default directory to <directory>, change PWD environ var
//if <directory> isnt present in arg1, report current directory
//if directory doesnt exist, report error
int cd(char **args)
{
  if (args[1] == NULL) {
    //no argument, report current directory
    char buff[PATH_MAX + 1];
    if(getcwd(buff, PATH_MAX + 1) == NULL) {
      fprintf(stderr, "cwd error in cd\n");
      exit(EXIT_FAILURE);
    }
    printf("no <directory> argument found.\nCurrent directory is: %s\n", buff);
    return 1;
  } else {
    if (chdir(args[1]) != 0) {
      perror("chdir");
    } else {
      setenv("PWD",args[1],1); //set environment variable
    }
  }
  return 1;
}
 
//display user manual. 
int help(char **args)
{
  int len = 0;
  int saved_stdout;
  int io_output = 0;
  int io_output2 = 0;
  int out;
  int i = 0; 
  len = argsLength(args);
 
  for(i=0;i<len;i++) {
    if(strcmp(args[i], redirection[1]) == 0) { io_output=i; }
    else if(strcmp(args[i], redirection[2]) == 0) { io_output2=i; }
  }
  if(io_output > 0) {
    //Save current stdout for use later
    saved_stdout = dup(STDOUT_FILENO);
    //open file for output redirection
    out = open(args[io_output +1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    //replace standard output with output file
    dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
    close(out);
  } else if (io_output2 > 0) {
    //open file for output redirection
    out = open(args[io_output2 +1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    //replace standard output with output file
    dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
    close(out);
  }
 
  system("more readme");

  //back to stdout
  if(io_output > 0 || io_output2 > 0) {
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);
  }
 
  return 1;
}
 
//exits program by returning 0 
int myshell_quit(char **args)
{
  return 0;
}
//END BUILTIN COMMANDS
//**********************************************************************************************************************************************************************************
 
 
// input is null terminated list of args
// launches non builtin cmd, handles io_redirection commands
// assumes program and program args come before (to the left) of any io redirection
// sets any index containing io redirection to null, so io redirection isnt passed to exec()
// always returns 1 to continue execution loop  
int myshell_launch(char **args)
{
  pid_t pid, wpid;
  int status; 
  int len = 0;
  int i = 0; //counter
  int io_counter = 0;
  int io_output = 0;
  int io_input = 0;
  int io_amper = 0;
  int io_pipe = 0;
  int io_output2 = 0;
  int in, out;
 
  len = argsLength(args);
 
  //iterate through args array, determine which io case(s) are needed
  //if io case found, variable set to equal the position of the redir symbol in args[]
  for(i=0;i<len;i++) {
    if(strcmp(args[i], redirection[0]) == 0) { io_input=i; } 
    else if(strcmp(args[i], redirection[1]) == 0) { io_output=i; } 
    else if(strcmp(args[i], redirection[2]) == 0) { io_output2=i; } 
    else if(strcmp(args[i], redirection[3]) == 0) { io_pipe=i; } 
    else if(strcmp(args[i], redirection[4]) == 0) { io_amper=i; }
    //printf("args[%d] = %s\n", i, args[i]);
  }
  //printf("len = %d\n", len);
  //printf("io_output = %d\n", io_output);
  //printf("io_input = %d\n", io_input);
  //printf("io_output2 = %d\n", io_output2);
  //printf("io_pipe = %d\n", io_pipe);
  //printf("io_amper = %d\n", io_amper);
 
  //begins large list of if else 
  //each one represents actions to take for a particular set of io commands
  if (io_output == 0 && io_input == 0 && io_output2 == 0 && io_pipe == 0) //if no redirection
  {
    if( (pid = fork()) < 0) { //error
      perror("fork");
    } else if (pid == 0) { //child
      if(io_amper > 0) {
        args[io_amper] = NULL; 
      }
      if(execvp(args[0], args) == -1) {
        perror("execvp");
        exit(EXIT_FAILURE);
      }
    } else { //parent
      if(io_amper > 0) {
        ;
      } else {
        do {
          wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
      }
    }
  } 
  else if (io_output > 0 && io_input == 0 && io_output2 == 0 && io_pipe == 0) //if '>'
  {
    if( (pid = fork()) < 0) { //error
      perror("fork");
    } else if (pid == 0) { //child, handle redirection in here
        //open file for output redirection
        args[io_output] = NULL; //assumes command and cmd args come before redirection
        out = open(args[io_output +1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
         //replace standard output with output file
        dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
        close(out); 
        if(execvp(args[0], args) == -1) {
          perror("execvp");
        }
    } else { //parent
      if(io_amper > 0) {
        ;
      } else {
        do {
          wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
      }
    }
  }
  else if (io_output == 0 && io_input > 0 && io_output2 == 0 && io_pipe == 0) //if '<'
  {
    if( (pid = fork()) < 0) { //error
      perror("fork");
    } else if (pid == 0) { //child, handle redirection in here
        //open file for output redirection
        args[io_input] = NULL; //assumes command and cmd args come before redirection
        in = open(args[io_input+1], O_RDONLY);
         //replace standard input with input file
        dup2(in, STDIN_FILENO); //STDIN_FILENO is 0
        close(in); //might need to also close in? not sure...
        if(execvp(args[0], args) == -1) {
          perror("execvp");
        }
    } else { //parent
      if(io_amper > 0) { //background
        ;
      } else {
        do {
          wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
      }
    }
  }
  else if (io_output == 0 && io_input == 0 && io_output2 > 0 && io_pipe == 0) //if '>>'
  {
    if( (pid = fork()) < 0) { //error
      perror("fork");
    } else if (pid == 0) { //child, handle redirection in here
        //sets arg index to null so exec doesn't read past cmd and args to cmd
        args[io_output2] = NULL; //assumes command and cmd args come before redirection        
        //opens for writing, appending, creating if necessary
        out = open(args[io_output2+1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
         //replace standard input with input file
        dup2(out, STDOUT_FILENO); //STOUT_FILENO is 1
        close(out); 
        if(execvp(args[0], args) == -1) {
          perror("execvp");
        }
    } else { //parent
      if(io_amper > 0) {
        ; //background, exit without waiting
      } else {
        do {
          wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
      }    
    }
  }
  else if (io_output > 0 && io_input > 0 && io_output2 == 0 && io_pipe == 0) //if '<' and '>'
  {
    if( (pid = fork()) < 0) { //error
      perror("fork");
    } else if (pid == 0) { //child, handle redirection in here
        //sets arg index to null so exec doesn't read past cmd and args to cmd
        args[io_output] = NULL; //assumes command and cmd args come before redirection
        args[io_input] = NULL;
        in = open(args[io_input +1], O_RDONLY);
        out = open(args[io_output +1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
         //replace standard output with output file
        dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
        //replaces standard input with input file
        dup2(in, STDIN_FILENO);
        //closing unused file descriptors
        close(in);
        close(out); 
        if(execvp(args[0], args) == -1) {
          perror("execvp");
        }
    } else { //parent
      if(io_amper > 0) {
        ; //background, exit without waiting
      } else {
        do {
          wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
      }
    }
  }
  else if (io_output == 0 && io_input > 0 && io_output2 > 0 && io_pipe == 0) //if '<' and '>>'
  {
    if( (pid = fork()) < 0) { //error
      perror("fork");
    } else if (pid == 0) { //child, handle redirection in here
        //sets arg index to null so exec doesn't read past cmd and args to cmd
        args[io_output2] = NULL; //assumes command and cmd args come before redirection
        args[io_input] = NULL;
        in = open(args[io_input +1], O_RDONLY);
        out = open(args[io_output2 +1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
         //replace standard output with output file
        dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
        //replaces standard input with input file
        dup2(in, STDIN_FILENO);
        //closing unused file descriptors
        close(in);
        close(out); 
        if(execvp(args[0], args) == -1) {
          perror("execvp");
        }
    } else { //parent
      if(io_amper > 0) {
        ; //background, dont wait for child process to finish
      } else {
        do {
          wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
      }
    }
  }
  else if (io_pipe > 0) //if single '|'
  {
    int j = 0; //counter
    int x = 0; //counter
    int z = 1; //counter
    int status;
    int pipefd[2]; 
     
    char **args2 = malloc((len - io_pipe) * sizeof(char *)); //total length of args (including null ptr) - where the pipe is (so, )
    if (!args2) { //if malloc error
      fprintf(stderr, "allocation error\n");
      exit(EXIT_FAILURE);
    }
    do { //puts to the right of pipe into args2
      args2[j] = args[io_pipe+z];
      j++;
      z++;
    } while (j<len-1-io_pipe); //args len - args null ptr io-pipe position
    //placing null ptrs
    args2[j] = NULL;
    args[io_pipe] = NULL;
     
    pipe(pipefd);
     
    runsource(pipefd, args);
    rundest(pipefd, args2);
    close(pipefd[0]);
    close(pipefd[1]); //closed both file descriptors on pipe
 
    while ((pid = wait(&status)) != -1) { //collects any child processes that have exit()ed
      fprintf(stderr, "process %d exits with %d\n", pid, WEXITSTATUS(status));
    }
  }
  //going back to loop, no quit
  return 1;
}
//left of pipe
void runsource(int pfd[], char **args) 
{ 
  int pid, wpid;
  int status;
  int len = 0;
  int i = 0;
  int io_input = 0;
  int in;
 
  len = argsLength(args);
 
  for(i=0;i<len;i++) { //if any args are io_input, set index
    //printf("args[%d] = %s\n", i, args[i]);
    if(strcmp(args[i], redirection[0]) == 0) { 
      io_input=i;  
    }
  }
  //printf("args[%d] = %s\n", i, args[i]);
 
  if( (pid = fork()) < 0) { //error 
    perror("runsource fork");
  } else if (pid == 0) { //child, handle redirection in here
  
    if(io_input > 0) {
      in = open(args[io_input+1], O_RDONLY);
      dup2(in, STDIN_FILENO); //STDIN_FILENO is 0
      close(in);
      args[io_input] = NULL;
    }
  
    dup2(pfd[1],STDOUT_FILENO); //end of this pipe becomes stdout
    if(execvp(args[0],args) == -1) {
      perror("execvp");
    }
  } else { //parent
    close(pfd[1]); //close input side of pipe
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
  }
}
//right of pipe
void rundest(int pfd[], char **args2)
{
  int pid, wpid;
  int len = 0;
  int i = 0;
  int io_output = 0;
  int io_output2 = 0;
  int io_amper = 0;
  int out;
  int status;
 
  len = argsLength(args2);
 
  for(i=0;i<len;i++) { //iterates through args, if any are redirect, set var to index 
    //printf("args2[%d] = %s\n", i, args2[i]);
    if(strcmp(args2[i],redirection[1]) == 0) { io_output=i; } 
    else if(strcmp(args2[i], redirection[2]) == 0) { io_output2=i; } 
    else if(strcmp(args2[i], redirection[4]) == 0) { io_amper=i; }
  }
  //printf("args2[%d] = %s\n", i, args2[i]);
  if( (pid = fork()) < 0) { //error 
    perror("runsource fork");
  } else if (pid == 0) { //child, handle redirection in here
    if(io_output > 0) {
      out = open(args2[io_output +1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
      //replace standard output with output file
      dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
      close(out); 
      args2[io_output] = NULL;
    } else if(io_output2 > 0) {
      out = open(args2[io_output2 +1], O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
      //replace standard output with output file
      dup2(out, STDOUT_FILENO); //STDOUT_FILENO is 1
      close(out); 
      args2[io_output2] = NULL;
    }
    dup2(pfd[0], STDIN_FILENO); //end of this pipe becomes stdin
    if(execvp(args2[0],args2) == -1) {
      perror("execvp");
    }
  } else { //parent
      do {
        pid = waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //wait until specified process is exited or killed
  }
}
 
//executes array of tokens, directs to builtin or executes external
//returns 1 if shell should continue
int execute(char **args)
{
  int i;
 
  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }
    // check for built in commands
  for (i = 0; i < number_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  return myshell_launch(args);
}
 
//reads a line from stdin, returns char pointer to line
//dynamically reallocates, getline is magical 
char *myshell_readline(void)
{
  char *line = NULL;
  ssize_t buffsize = 0; // have getline allocate a buffer for us
  if(getline(&line, &buffsize, stdin) < 0) {//error
    perror("getline");
    exit(EXIT_FAILURE);
  }
  return line;
}
 
 
// input line, returns null terminated array of tokens
// dynamically allocates memory, if necessary
// only works as intended if commands are separated by one or more spaces
char **chop_line(char *line)
{
  int buffsize = MYSHELL_TOKEN_SIZE;
  int index = 0;
  char **tokens = malloc(buffsize * sizeof(char*));
  char *token;
 
  if (!tokens) {
    fprintf(stderr, "allocation error\n"); //must be fprintf
    exit(EXIT_FAILURE);
  }
  token = strtok(line, MYSHELL_TOKEN_DELIM); //tokenize string 
  while (token != NULL) {
    tokens[index] = token;
    index++;
    if (index >= buffsize) {
      buffsize += MYSHELL_TOKEN_SIZE;
      tokens = realloc(tokens, buffsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, MYSHELL_TOKEN_DELIM);
  }
  tokens[index] = NULL;
  return tokens;
}
 
//Loop getting input and executing.
void loop(void)
{
  char *line;
  char **args;
  int status;
  char curDir[PATH_MAX + 1]; //for pathname: http://www.qnx.com/developers/docs/660/index.jsp?topic=%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fg%2Fgetcwd.html
  printf("\n(•_•)\n( •_•)>⌐■-■\n(⌐■_■)\nmyshell initialized\n\n");
  //if no batchfile, shell loop begins
  do {
    if(getcwd(curDir, PATH_MAX + 1) == NULL) {
        fprintf(stderr, "cwd error\n");
        exit(EXIT_FAILURE);
    }
    struct passwd *pws; //for getting id
    pws = getpwuid(geteuid());
    printf("%s@%s$ ", pws->pw_name, curDir); //prompt
    line = myshell_readline(); //get line from user
    args = chop_line(line);  //split line into tokens
    status = execute(args); //run tokenized line, lots of work in this function
    free(line);
    free(args);
  } while (status); //continue while execute returns positive status
}
 
//main: handles batchfile
//passes batchfile lines to execute(), exits when complete
//if no batchfile, enters cmd loop accepting user inpu
int main(int argc, char **argv)
{
  //if there is a batchfile:
  //batchfile is split into lines 
  //lines are processed through the exit function
  //function then exits
  if (argc > 1) {
    int i; //counter
    int status;
    FILE *fp;
    char **batch_lines;
    char *token;
 
    char *filename = argv[1];
    if((fp = fopen(filename, "r")) == NULL) { //open batchfile for reading
      printf("fileopen error\n");
      return -1;
    }
 
    fseek(fp, 0, SEEK_END); //go to end of batchfile
    int inputsize = ftell(fp); //this is size of batchfile
    char *batchfile = malloc(inputsize * sizeof(char)); //batchfile char array is now correct size
    if (!batchfile) { //err check
      fprintf(stderr, "allocation error\n");
      exit(EXIT_FAILURE);
    }
 
    fseek(fp,0,SEEK_SET); //go back to beginning of file
 
    for (i = 0; i < inputsize; i++) { //fill *batchfile
      fscanf(fp, "%c", &batchfile[i]);
    }
    batchfile[i] = EOF;
    fclose(fp); //done with file
    int buffsize = MYSHELL_TOKEN_SIZE;
    int position = 0;
    char **buffertokens = malloc(buffsize * sizeof(char*));
 
    if (!buffertokens) { //err check
      fprintf(stderr, "allocation error\n");
      exit(EXIT_FAILURE);
    }
 
    token = strtok(batchfile, BATCH_TOK_DELIM);
     
    while (token != NULL) {
      buffertokens[position] = token;
      position++;
      if (position >= buffsize) { //if reallocation is needed
        buffsize += MYSHELL_TOKEN_SIZE;
        buffertokens = realloc(buffertokens, buffsize * sizeof(char*));
        if(!buffertokens) {
          fprintf(stderr, "allocation error\n");
          exit(EXIT_FAILURE);
        }
      }
      token = strtok(NULL, BATCH_TOK_DELIM);
    }
    buffertokens[position] = NULL;
     
    for(i = 0; i < (position-1); i++) {
      batch_lines = chop_line(buffertokens[i]);
      status = execute(batch_lines);
      if (status < 1) {
        //freeing memory before shutdown
        free(batch_lines);
        free(buffertokens);
        return EXIT_SUCCESS;
      } 
    }
  } else {
    // No batchfile, run command loop accepting user input from stdin
    loop();
  }
  //shutdown
  return EXIT_SUCCESS;
}