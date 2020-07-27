#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

int status, child_pid;
int job_foreground;
int terminal_pg; 
int stop;
void sigttou_handler(int arg) 
{
    if(job_foreground && stop == 0){
        if(tcsetpgrp(STDIN_FILENO, getpgid(child_pid)) < 0){
            if(tcsetpgrp (STDIN_FILENO, getpgrp()) < 0){
                perror("tcsetpgrp 2");
                exit(1);
            }
        }
    }else{
        if(tcsetpgrp(STDIN_FILENO, getpgrp()) < 0) {
            perror("tcsetpgrp 2");
            exit(1);
        }
    }
}
void sigttin_handler(int arg) 
{
    printf("sigttin hit\n");
    if(job_foreground && stop == 0){
        if (tcsetpgrp (STDIN_FILENO, getpgid(child_pid)) < 0) {
            if( tcsetpgrp (STDIN_FILENO, getpgrp()) < 0) {
                perror("tcsetpgrp 2");
                exit(1);
            }
        }
    }else{
        if( tcsetpgrp (STDIN_FILENO, getpgrp()) < 0) {
            perror("tcsetpgrp 2");
            exit(1);
        }
    }
}
void sigchld_handler(int arg)
{
    printf("sigchld hit\n");
    switch(arg){
        case SIGCHLD:
            if(waitpid(-1, &status, WNOHANG | WUNTRACED) == 0){
                printf("in if condition\n");
                wait(&status);
            }
            if(WIFSTOPPED(status)){
                kill(child_pid, SIGTSTP);
                printf("wif\n");
                /*
                if( tcsetpgrp (STDIN_FILENO, terminal_pg) < 0) {
                    perror("tcsetpgrp 2");
                    exit(1);
                }
                */
                printf("hit\n");
                stop = 1;
            }
            break;
    }
}

int main(int argc, char *argv[])
{
    stop = 0;
    job_foreground = 1;
	printf("pgrp=%d pid=%d term=%d\n", getpgrp(), getpid(), tcgetpgrp(STDIN_FILENO));

   /* Ignore interactive and job-control signals.  */
   signal (SIGTTIN, sigttin_handler);
   signal (SIGTTOU, sigttou_handler);
   signal (SIGCHLD, sigchld_handler);
   signal (SIGTSTP, SIG_IGN);
	   

	/* Make your shell its process group leader */
	if (setpgid (getpid(), getpid()) < 0) {
            perror ("setpgid 1");
            exit (1);
	}
	/* Get terminal control from the  shell that started your shell */
    terminal_pg = getpgrp();
	if( tcsetpgrp (STDIN_FILENO, getpgrp()) < 0) {
		perror("tcsetpgrp 1");
		exit(1);
	}
	if( (child_pid = fork()) < 0) {
		perror("fork:");
		exit(1);
	}
	if( child_pid == 0) { /* child */
           signal (SIGQUIT, SIG_DFL);
           signal (SIGTSTP, SIG_DFL);
           signal (SIGTTIN, SIG_DFL);
           signal (SIGTTOU, SIG_DFL);
           signal (SIGCHLD, SIG_DFL);
           signal (SIGINT, SIG_DFL);

		/* Make yourself process group leader */
		if( setpgid(child_pid, child_pid) < 0) {
			perror("setpgid 2");
			exit(1);
		}

        if( tcsetpgrp (STDIN_FILENO, getpgrp()) < 0) {
            perror("child tcsetpgrp");
            exit(1);
        }
		/* transfer controlling terminal */
        
#if 0
        char *cmd = "./background.sh";
        char *argv[2];
        argv[0] = "./background.sh";
        argv[1] = NULL;
#endif
#if 1
        char *cmd = "ls";
        char *argv[3];
        argv[0] = "ls";
        argv[1] = "-la";
        argv[2] = NULL;
#endif
		if (execvp(cmd, argv) < 0) {
			exit(1);
		}
	} else /* parent */ {
		/* Make child its own process group leader  - to avoid race conditions */
		if( setpgid(child_pid, child_pid) < 0) {
			if( errno != EACCES ) {
				perror("setpgid 3");
				exit(1);
			}
		}
        if (tcsetpgrp (STDIN_FILENO, getpgid(child_pid)) < 0) {
            perror("tcsetpgrp 2");
            exit(1);
        }
        if (kill(child_pid, SIGCONT) < 0) {
            perror("kill");
        }

        /* Children completed: put the shell back in the foreground.  */
        printf("Parent\n");
        if(WIFSTOPPED(status))
            printf("Child process stopped\n");
        else {
            printf("Child process terminated. Now in parent. Press any key to terminate.\n");
            getchar();
        }
        printf("Type something: \n");
        printf("Parent got %c\n", getchar());
	}
    return 0;
}

