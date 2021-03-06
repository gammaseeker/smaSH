# Smash
This is my implementation of "smash" (SMAll SHell)

    * Can perform subset of commands from Bash
    * Implements cd, pwd, and echo from scratch
    * Implements job control support
    * Implements redirection of input, output, and error
    * Supports creation, reading, and writing to variables
    * Supports Non-interactive use
    * Supports debug mode
    * Regression Tests for various use cases

There is documentation within all header files. Some
functions are deemed to be self-explanatory based on the function name
(ex smash_pwd()).

## How to compile and run
Run `make` to generate the `smash` binary. You may now run the shell in
interactive mode by running `./smash` or `./smash -d`. To run in
non-interactive mode either use the shebang of wherever smash is installed in your sh file. Or you
can run `./smash <your_sh_file>`

## Test Suites and Regression Tests
Inside `reg_tests/` directory there are 3 test scripts that run
smash in non-interactive mode.

    * `test01.sh` invokes test drivers in `reg_tests/simple_cmd`
    * `test02.sh` invokes test drivers in `reg_tests/var_cmd`
    * `test03.sh` invokes test drivers in `reg_tests/redir_cmd`

Upon running the test scripts it will inform you of what scenario the 
test script is running. Since it can be tricky to determine if some commands
, such as `ls -la` suceeded (dir contents change all the time). 
The scripts check whether or not
smash has returned successfully or returned an error code. It is up
to a human to verify if the command produced expected output. 

### test01.sh
This test script will run simple commands such as `pwd` and `ls`. The drivers
for the test programs can be found in `reg_tests/simple_cmd`. Sleep was called
in test01.sh so that it is easier to see the output per test case.

### test02.sh
This test script will test variable operations such as echo environment
variables to verify they have been inherited correctly, set variables, set $?.
Drivers can be found in `reg_tests/var_cmd`.

### test03.sh
This test script will test redirection operations. The test covers all
types of combinations of input, output, and error redirection. The test drivers
in `reg_tests/redir_cmd`
have specific output/error files that they produce to check the result.
\N/A under a column refers to no file being produced, not a file called N/A.

    test_driver             STDOUT file                 STDERR file
    * redir_out.sh          redir_out_outfile           N/A
    * redir_out_err.sh      redir_out_err_outfile       edir_out_err_outfile
    * redir_in_out.sh       redir_in_out_outfile        N/A
    * redir_err.sh          N/A                         redir_err_errfile
    * redir_in_out_err.sh   redir_in_out_err_outfile    edir_in_out_err_outfile

## Debug Mode
If you run `./smash -d` it will activate the shell's debug mode where it will
write debugging info to stderr. Every command executed will say "RUNNING: <cmd>"
and every command ending will show it's exit status and say
"ENDED: <cmd> (ret=%d)".

## Data Structures
### symbol table
I went with an extremely simple approach to track variables. There are two
char* arrays in `symbol_table.h` var_names[] tracks variable names and
stores the name at an index. The variable names are stored in a linear order
so O(n) insertion. Coupled with var_names[] insertion, another linear
time insertion is made into values[] which stores the value the variable is 
storing. Operations on these data structures have been wrapped up in 
functions in `symbol_table.c`

### job
In `job_control.h` I tracked jobs by making a linked list of job nodes.
This made the most sense since a job spec is determined in ascending 
numeric order. The operations on this `jobs` linked list are all standard
linked list operations.

## `smash.h`
Contains important macros that are used. There is documentation on
these macros and certain functions.

## `smash.c`
Buckle up reader because we're getting into the meat and bones of this project.
There are two program flows I will discuss: non-interactive, and interactive
mode. Some steps are shared between both flows so I will
state that first.

Shared steps:
    1. Install signal handlers
    2. Make smash it's own process group leader, and bring smash into foreground
    3. inherit environment vars from bash
    4. check for debug mode
    5. check interactivity mode

After those shared steps the program flow for non-interactive mode is as follows:
    1. Parse argv
    2. If smash is executing some script (`./smash some_script`) it will 
    execvp and execute it
    3. If smash is executed from a shebang line, it will call `smash_loop()`
    and read in that script file line-by-line and executing each line

After those shared steps the program flow for interactive mode is as follows:
    1. Call `smash_loop()` and prompt user
    2. Upon receiving a command, identify what command it is and execute
    the appropriate handler
    3. Upon successful execution $? will be set and the user will be 
    prompted agian
    4. The program may exit via CTRL-C or entering `exit` into the shell, and
    the shell will clean up all data structures

### `smash_cd()`
Uses `chdir(2)`

### `smash_pwd()`
Uses `getcwd(3)`

### `smash_echo()`
Uses `printf()`. The function will check to see if the argument
to echo contains $ If it does then a look up in the symbol table is performed to
retrieve and print the variable's value. Else we print the plain argument.

### `smash_assign_var()`
Parses tokens from the user and performs an insertion to the symbol table.

## Redirection
### `smash_redirect()`
Upon parsing input from the user, the program will set flags for what
type of redirection that must be performed. The flags are defined in
`smash.h` and are IN_REDIR 0X1, OUT_REDIR 0X2, ERR_REDIR 0X4 and can be
logically OR. Redirection is implemented in the traditional sense where
we save the file descriptor we are about to redirect, then replace that file
descriptor with the appropriate source/destination, run our command, then
restore our original file descriptors.

## Job Control
This was really wild to implement.
### `job_print()`
Defined in `job_control.c` will print out the list of jobs in just about
the same format as bash:

[job_spec] pid job_state job_name

The job_spec is defined in ascending numeric order. This function is called when
the user types in the command `jobs` or if the user sends a job
to the background and suspends it via CTRL-Z.

### `smash_kill()`
Parses tokens from the user and sends signal to appropriate process

### &
To start up a job in the background a user can type in `CMD1 &`. 
The shell will return to the foreground as the background process
continues execution. A command you may use to test this is running
`./background.sh` in the shell in interactive mode.

### CTRL C
This will terminate the job in the foreground.

### CTRL Z
This will send the foreground job into the background and suspends it. This is
handled in `sigchld_handler()` in `smash.c`. I used WIFSTOPPED() to check
when a process is stopped and update my jobs data structure accordingly.

### `smash_foreground()`
This handles the `fg` command. Implemented by calling tcsetpgrp and giving control
of stdin to the job that is specified to move to the foreground. The function
sends a SIGCONT to that job and then waits for the job to finish before
returning control to the shell.

### `smash_background()`
This handles the `bg` command and sends a process to run in the background.
