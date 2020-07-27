#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char **argv){
    printf("argc: %d\n", argc);
    int i = 0;
    while(argv[i]){
        printf("argv[%d]: %s\n", i, argv[i]);
        i++;
    }

    char *args[2];
    args[0] = "./bar.sh";
    args[1] = NULL;

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return 1;
    }

    char *tok = strtok(argv[0], ".");
    printf("tok %s\n", tok);
    strcat(cwd, tok);
    printf("Current working dir: %s\n", cwd);
    if(strcmp(argv[0], cwd)){
        printf("non-int mode\n");
        FILE *fp = fopen(argv[1], "r");
        char *line = NULL;
        size_t len = 0;
        while(getline(&line, &len, fp) != -1){
            printf("line length: %zd\n", strlen(line));
            printf("%s\n", line);
        }
        printf("\n\nMax Line size: %zd\n", len);
        fclose(fp);
        free(line);
    }else{
        execvp("./bar.sh", args);
    }
    return 0;
}
