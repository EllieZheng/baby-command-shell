Author: Ellie Zheng

Completed term: 2016 Fall


*****************************************************************************
### Steps finished
*****************************************************************************
Mini-Project: Command Shell

All contents in Step 1, 2, 3:

**Step 1:** Read a command name, run it. After exiting, return the exit status.

**Step 2:** Search the PATH environment variable for commands, which can take arguments.

**Step 3:** Navigate between directories: cd, pushd, popd, dirstack. Prompts are changed accordingly.

**Step 4:** Redirection (<, >, 2>). Pipes is not implemented.

(I attempted pipes in the foler "4_step_attempt", but the program crashes from 
time to time, so I am rather presenting the well-polished first 3.5 steps)


*****************************************************************************
### Implementation details
*****************************************************************************
#### Data structures:
1. class UserInput:  stores and parses user's input
2. class CmdHandler: takes in UserInput and executes commands

These two classes are independent and have interfaces that can be used by 
other callers as well.


#### class **UserInput**: 
1. store arguments that seperated by white spaces (not inluding "\ ") in 
   vector<string> first
2. search from the vector for "<" ">" "2>", and store the redirected file-
   names into a string array, and delete them from the arguments vectors
3. convert vector<string> into char**
4. public methods are provided for obtaining the command(char *), the 
   argument list(char **), the redirection info (string[3])
5. execeptions might be thrown, including empty input or calling exit

#### class **CmdHandler**:
1. store the working directory, the caller's directory, directory stack, and
   environment parameter
2. method executeCmd(UserInput) is used to execute the command:
    1. if path is not specified (i.e., do not contain /):
          * if "cd" "pushd" "dirstack" are called, then make changes to the directory
          * search in PATHs the program. If found, update the command with the full path to it
 
    2. if path is specified, or command is in PATHs:
          * fork the process 
          * child: set up redirection according to the UserInput by dup2(), 
          then call execve(). 
          Exception might be thrown if execve() fails, and error message will
          be printed by perror.
          * parent: wait and check the status of child. If the program actually
          runs, print out the exit status
3. method printPrompt() is used to reflect current working directory

#### main:
1. initialize UserInput and CmdHandler
2. while "exit" is not called, update UserInput, and run CmdHandler.

   Catch the exceptions mentioned above

#### pipes

(not implemented in the current version presented here):

In the foler "4_step_attempt" (again, not the code in current directory), 
pipes is added by building another class with a field of vector<UserInput>.
   
In the CmdHandler class, executeCmd() now takes in a vector of UserInput, and
uses a loop to update the redirection info in pairs: input[i] = pipefd[1], 
output[i+1] = pipefd[2]. Then the rest of the execution remains the same.

Errors occur in terms of strings, possibly related to parsing.


*****************************************************************************
### Test cases
*****************************************************************************
1. Commands
    1. Program with path given, i.e., has a / in it
        *   Existing file with different file types (regular files, links,
             directory, etc.), and different permission 
        *  Files in either current folder or other folders, absolute path
             or relative path
        * File not exists
    2. Program with no path given, i.e., does no have / in it
        *   Common linux commands: ls, echo, cat, clear, cp, mkdir
        *  Programs in user-added PATHs: ~/ece551/lz91/bin/
        * Programs in the current folder
        *  Command not exist: randomly typed words
    3. Program executed successfully or with error, or ^C'd by user  
       In this case, program exit status should printed accordingly
    *. Do not know how to test if fork() fails or other system calls fail

2. Input parsing
    -- use a program called "myEcho" to print out the arguments passed to it
    1. long arguments
    2. large number of arguments
    3. arguments that are seperated by arbitrary number of white spaces,
       or at the beginning or at the end of the command
    4. arguments that have "\ " in them, which appears at the beginning, in
       the middle, at the end, stand-alone, consecutively("\ \ ")

3. Directory
    1. cd, pushd
       *   cd into child directory
       *  cd ./ ../
       * cd into other directories with absolute/relative path
            -- that can be accessed
            -- that we do not have permission to access
            -- that do not exist
       *  take randomly typed words as arguments
       *   take multiple arguments
       *  take no argument (return to caller's directory by default)
    2. popd, dirstack
       *   have directory in stack
       *  empty stack

4. Redirection
    -- use echo, cat and sortLines
    1. syntax
       *   have one of <, >, 2> in it
       *  have all three of them in it
       * have multiple symbols that are the same (save the last occurred
            filename by default)
       *  symbols appear at the beginning, in the middle, or at the end of
            the commands
       *   nothing appears after a symbol
    2. file
       * without path (search in current folder)
       * have absolute/relative path
       * different types and permission (as described in I.1.i)
       * input/output files exist or do not exist
       * large input/output size

No test cases are given for pipes, as it is not implemented in this version.



