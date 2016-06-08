# DESCRIPTION

'myshell' is a simple shell designed to parse input into commands, arguments, and optional input redirection, output redirection, and background processing. Once input is parsed, the input is either processed internally if its a builtin command, otherwise it is located and executed. This shell does not support many standard shell features, the only features it supports are the features listed in this document. 

# BATCHFILE

To use a batchfile, create a file and have each line desired to be processed by 'myshell' entered on separate lines.

To run the batchfile, run the program and make the first argument the batchfile name.
Batch processing will exit program once EOF is reached in the batchfile.

batchfile formatting: './myshell batchfile'

# INTERNAL COMMANDS SUPPORTED: 

- cd 	- changes directory
- clr	- clears the screen
- dir     - lists contents of directory, supports I/O '>' and '>>' redirection   
- environ - lists environment, supports I/O '>' and '>>' redirection
- echo	- echos the string entered to standard output
		- supports I/O '>' and '>>' redirection
- help	- displays contents of readme file using the more command
		- supports I/O '>' and '>>' redirection
- pause	- pauses program until 'Enter' is pressed
- - quit	- quits 'myshell'

input syntax formatting examples: 
- 'command arg1 arg2 < inputfile > outputfile &' 
- 'command arg1 arg2 >> outputfile < inputfile &'
- 'command arg1 arg2 < inputfile | command arg1 arg2 > outputfile'

examples that demonstrate functionality:
- 'ls -l | sort'
- 'clr'
- 'man gcc > man_gcc'
- 'grep'
- 'sleep 10 &' 
- 'cd ..'
- 'dir /home > outputfile'
- 'dir / >> outputfile'
	
	
# IMPORTANT WARNINGS:      
             
* background symbol always appears at the end of the line  
* all sections to be parsed are separated by whitespace
* all redirection is optional
* only use of 1 pipe is supported
* only input redirection is supported to the left of the pipe 
* only output redirection is supported to the right of the pipe
* shell does not support bringing background process to foreground
* shell does not support reporting when background process finishes

I/O REDIRECTION OPERATORS
	
- '>'  - redirects standard output to file, creating and truncating if necessary
- '>>' - very similar to '>', except file is appended if necessary
- '<'  - redirects standard input to come from file
- '|'  - pipes are a set of processes chained by their standard streams so that the output of the process to the left of the pipe (stdout) feeds directly as input (stdin) to process to the right of the pipe
- '&'  - backgrounds a process, rendering the process unable to receive keyboard signals and unable to send output to the parent terminal. Essentially, the process set to the background acts as any other process, except it can't be seen.

PROGRAM ENVIRONMENT

The program environment represents many pieces of information that determine behavior and access to resources. Some of these settings are contained within configuration files and others are determined by user input. One way the shell keepds track of all these settings and details is through an area it maintains called the environment. The environment is an area that the shell builds each time it starts a user session that contains an array of environment strings that define system properties. Environment variables are variables that are defined for the current shell and are inherited by any child shells or processes. 

BACKGROUND PROCESS EXECUTION

A background process is one that runs "behind the scenes". This means that a backgrounded process runs without user intervention. In regards to environment variables, the group ID will differe from its terminal ID. Typically this is used for processes that use minimal system resources and perform tasts that require little to no input from the user. Often these programs wait for certain events to occur before executing. Once complete, they resume watching for certain conditions.  	

Use the 'man' or 'info' commands for more information on commands not covered in this document.
