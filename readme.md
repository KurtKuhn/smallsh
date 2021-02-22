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

## A Sample Execution of the Program

```

$ smallsh

: ls
junk   smallsh    smallsh.c

: ls > junk
: status
exit value 0

: cat junk
junk
smallsh
smallsh.c

: wc < junk > junk2

: wc < junk
       3       3      23
       
: test -f badfile

: status
exit value 1

: wc < badfile
cannot open badfile for input

: status
exit value 1

: badfile
badfile: no such file or directory

: sleep 5
^Cterminated by signal 2

: status &
terminated by signal 2

: sleep 15 &
background pid is 4923

: ps
  PID TTY          TIME CMD
 4923 pts/0    00:00:00 sleep
 4564 pts/0    00:00:03 bash
 4867 pts/0    00:01:32 smallsh
 4927 pts/0    00:00:00 ps
:
: # that was a blank command line, this is a comment line
:
background pid 4923 is done: exit value 0

: # the background sleep finally finished
: sleep 30 &
background pid is 4941

: kill -15 4941
background pid 4941 is done: terminated by signal 15

: pwd
/nfs/stak/users/chaudhrn/CS344/prog3

: cd
: pwd
/nfs/stak/users/chaudhrn

: cd CS344
: pwd
/nfs/stak/users/chaudhrn/CS344

: echo 4867
4867

: echo $$
4867

: ^C^Z
Entering foreground-only mode (& is now ignored)
: date
 Mon Jan  2 11:24:33 PST 2017
 
: sleep 5 &
: date
 Mon Jan  2 11:24:38 PST 2017
 
: ^Z
Exiting foreground-only mode

: date
 Mon Jan  2 11:24:39 PST 2017
 
: sleep 5 &
background pid is 4963

: date
 Mon Jan 2 11:24:39 PST 2017
 
: exit
$
```
