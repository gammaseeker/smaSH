#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv){
    // argv[1] == dest
    // argv[2] == 8src
    printf("arg 1:%s\n", argv[1]);
    printf("arg 2:%s\n", argv[2]);

    int len = strlen(argv[3]);
    char dest[len];
    int c = 0;
    while(c < len-1){
        dest[c] = argv[3][c+1];
        c++;
    }
    dest[len-1] = '\0';

    int srcfd;
    if((srcfd = open(dest, O_RDONLY)) < 0){
        perror(argv[3]);
        exit(1);
    }
    dup2(srcfd, 0); // set src fd to stdin
    
    pid_t child = fork();
    if(child < 0){
        perror("fork() error");
        exit(1);
    }
    if(child == 0){
        char *args[] = {argv[1], argv[2], NULL};
        if(execvp(argv[1], args) == -1){
            perror("execvp() failed");
        }
    }
    wait(NULL);
    return 0;
}
