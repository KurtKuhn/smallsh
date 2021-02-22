# Smallsh - A Small Shell Program

## About the program

Smallsh is a shell written in C. It implements a small subset of featuers of other well-known shells, such as bash.

The program has the ability to do the following

1. Provide a prompt for running commands
2. Handle blank lines and comments, which are lines beginning with the # character
3. Provide expansion for the variable $$
4. Execute 3 commands exit, cd, and status via code built into the shell
5. Execute other commands by creating new processes using a function from the exec family of functions
6. Support input and output redirection
7. Support running commands in foreground and background processes
8. Implement custom handlers for 2 signals, SIGINT and SIGTSTP


### Before you'll need MinGW and add it to your PATH
[You can read more about MinGW here](https://sourceforge.net/projects/mingw/)

## How to compile the program:

```
gcc --std=gnu99 -o smallsh smallsh.c
```

## How to run the program after compiling:
  
```
./smallsh
```
