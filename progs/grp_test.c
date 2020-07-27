#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stddef.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/termios.h>
/* NOTE: This example illustrates tcsetgrp() and setpgrp(), but doesn’t
function correctly because SIGTTIN and SIGTTOU aren’t handled.*/
int main()
{
    //int status;
    int cpid;
    int ppid;
    char buf[256];
    //sigset_t blocked;
    ppid = getpid();
    if (!(cpid=fork()))
    {
        setpgid(0,0);
        tcsetpgrp (0, getpid());
        signal (SIGINT, SIG_DFL);
        signal (SIGQUIT, SIG_DFL);
        signal (SIGTSTP, SIG_DFL);
        signal (SIGTTIN, SIG_DFL);
        signal (SIGTTOU, SIG_DFL);
        signal (SIGCHLD, SIG_DFL);
        execl ("/bin/vi", "vi", NULL);
        exit (-1);
    }
    if (cpid < 0)
        exit(-1);
    setpgid(cpid, cpid);
    tcsetpgrp (0, cpid);
    waitpid (cpid, NULL, 0);
    tcsetpgrp (0, ppid);
    while (1)
    {
        memset (buf, 0, 256);
        fgets (buf, 256, stdin);
        puts ("ECHO: ");
        puts (buf);
        puts ("\n");
    }
}
