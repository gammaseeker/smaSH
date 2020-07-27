#ifndef SMASH_H
#define SMASH_H

// ReadLine max buff size
#define RL_BUFFSIZE 1024
// Redirect flags that can be set and OR
#define IN_REDIR 0x1
#define OUT_REDIR 0x2
#define ERR_REDIR 0x4

#define BACKGROUND 0
#define FOREGROUND 1

void smash_loop(int argc, int non_int, char **argv);

// Set value of $? in symbol table
void set_ret_var(int ret);
int smash_redirect(int redir, char *infile, char *outfile, char **args);
int smash_assign_var();
int smash_echo();
int smash_kill();
void smash_cmd(int ground, char **args);

// fg command
void smash_foreground();

// bg command
void smash_background();
int smash_pwd();
int smash_cd(char *path);
void smash_exit();

// Inherit environment vars
void load_envp(char **envp);

// Parse input delimited by space
char **get_tokens(char *line);

// Computer number of tokens
int get_tokens_len();

// Get absolute path and append to arguments in argv
char *get_abs_path(char **argv);
#endif /*SMASH_H*/
