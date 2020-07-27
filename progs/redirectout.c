#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv){
    // argv[1] == src
    // argv[2] == 8dest
    printf("arg 1:%s\n", argv[1]);
    printf("arg 2:%s\n", argv[2]);

    int len = strlen(argv[2]);
    char dest[len];
    int c = 0;
    while(c < len-1){
        dest[c] = argv[2][c+1];
        c++;
    }
    dest[len-1] = '\0';

    int destfd;
    if((destfd = open(dest, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0){
        perror(argv[2]);
        exit(1);
    }
    dup2(destfd, 1); // set destination fd to stdout
    
    pid_t child = fork();
    if(child < 0){
        perror("fork() error");
        exit(1);
    }
    if(child == 0){
        char *args[] = {argv[1], NULL};
        if(execvp(argv[1], args) == -1){
            perror("execvp() failed");
        }
    }
    wait(NULL);
    return 0;
}
