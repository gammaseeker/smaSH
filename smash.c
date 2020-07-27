#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include "job_control.h"
#include "symbol_table.h"
#include "smash.h"

// Must be global so they can be freed if sigint_handler invoked
char *line;
char **tokens;
int debug, status, child, shell_pid, target_proc, ctrlz, job_foreground; // 1== job goes in foreground
FILE *fp;
size_t len;
void sigint_handler(int sig_num){
    free(line);
    free(tokens);
    job_free_all(&jobs);
    exit(0);
}

void sigttou_handler(int sig_num){
    if(job_foreground && !ctrlz){
        if(tcsetpgrp(STDIN_FILENO, getpgid(child)) < 0){
            // Child terminated, go back to terminal
            if(tcsetpgrp(STDIN_FILENO, getpgrp()) < 0){
                perror("tcsetpgrp 2");
                exit(1);
            }
        }
    }else{
        if(tcsetpgrp(STDIN_FILENO, getpgrp()) < 0){
            perror("tcsetpgrp 2");
            exit(1);
        }
    }
}

void sigttin_handler(int sig_num){
    if(job_foreground && !ctrlz){
        if(tcsetpgrp(STDIN_FILENO, getpgid(child)) < 0){
            // Child terminated, go back to terminal
            if(tcsetpgrp(STDIN_FILENO, getpgrp()) < 0){
                perror("tcsetpgrp 2");
                exit(1);
            }
        }
    }else{
        if(tcsetpgrp(STDIN_FILENO, getpgrp()) < 0){
            perror("tcsetpgrp 2");
            exit(1);
        }
    }
}

void sigchld_handler(int sig_num){
    if(waitpid(-1, &status, WNOHANG | WUNTRACED) == 0){
        wait(&status);

        char buff[4];
        sprintf(buff, "%d", status);
        st_insert("$?", buff);
        if(debug){
            struct job *job = job_get_by_pid(&jobs, child);
            if(job != NULL){
                fprintf(stderr, "ENDED: %s (ret=%d)\n", job->job_name, status);
            }
        }
        job_delete(&jobs, child);
    }
    else{
        if(WIFSTOPPED(status)){
            kill(target_proc, SIGSTOP);
            job_update(&jobs, target_proc, 0); // 0 suspend
            job_print(&jobs);
            ctrlz = 1;
        }else{
            char buff[4];
            sprintf(buff, "%d", status);
            st_insert("$?", buff);
            if(debug){
                struct job *job = job_get_by_pid(&jobs, child);
                if(job != NULL){
                    fprintf(stderr, "ENDED: %s (ret=%d)\n", job->job_name, status);
                }
            }
            job_delete(&jobs, child);
        }
    }
}

int main(int argc, char** argv, char** envp){
    printf("running smash\n");
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, sigint_handler);
    signal(SIGTTOU, sigttou_handler);
    signal(SIGTTIN, sigttin_handler);
    signal(SIGCHLD, sigchld_handler);
    // Make Smash process group leader
    if(setpgid(getpid(), getpid()) < 0){
        perror("setpgid");
        exit(1);
    }
    // Get stdin control from bash
    if(tcsetpgrp(STDIN_FILENO, getpgrp()) < 0){
        perror("tcsetpgrp 1");
        exit(1);
    }

    load_envp(envp);
    job_init(&jobs);
    // Detect -d
    if(argc > 1){
        if(strcmp(argv[1], "-d") == 0){
            debug = 1;
            perror("Debug mode activated\n");
        }
    }else{
        debug = 0;
    }

    char *cwd = get_abs_path(argv);
    if(strstr(cwd, argv[0]) != NULL){
        free(cwd);
        smash_loop(argc, 1, argv); // non_int set to true
    }
    else{
        free(cwd);
        if(argc > 1){
            if(debug){
                int size = 1;
                if(argc > 3){
                    size = argc - 2;
                }
                char **exec_args = (char **)malloc(size);
                if(size > 1){
                    int i = 0;
                    while(i < size-1){
                        exec_args[i] = argv[2+i];
                        i++;
                    }
                    exec_args[i] = NULL;
                    execvp(argv[2], exec_args);
                }
            }else{
                int size = 1;
                if(argc > 2){
                    size = argc - 1;
                }
                char **exec_args = (char **)malloc(size);
                if(size > 1){
                    int i = 0;
                    while(i < size-1){
                        exec_args[i] = argv[1+i];
                        i++;
                    }
                    exec_args[i] = NULL;
                }else{
                    exec_args[0] = NULL;
                }
                execvp(argv[1], exec_args);
            }
        }
    }
    smash_loop(argc, 0, argv);
    return 0;
}

void smash_loop(int argc, int non_int, char **argv){
    setpgid(0, 0); // create shell proc group
    job_foreground = 1; // Assume job takes foreground unless & passed in
    ctrlz = 0;
    shell_pid = getpid();
    int open_file = 0;
    while(1){
        if(!non_int){
            printf("smash> ");
        }
        ctrlz = 0;
        int cmd_exec = 0;
        if(!non_int){
            line = (char *)malloc(RL_BUFFSIZE);
            if(fgets(line, RL_BUFFSIZE, stdin) == NULL){
                if(errno) exit(1);
            }
            int len = strlen(line);
            if(line[len -1] == '\n'){
                line[len-1] = 0;
            }
        }else{
            if(!open_file){
                fp = fopen(argv[1], "r");
                line = NULL;
                open_file = 1;
                len = 0;
            }
            if(getline(&line, &len, fp) == -1){
                fclose(fp);
                free(line);
                free(line);
                free(tokens);
                job_free_all(&jobs);
                exit(0);
            }
        }

        // tokenize line
        tokens = get_tokens(line);
        int num_tokens = get_tokens_len();

        // no input then ENTER or comment
        if(!tokens[0] || tokens[0][0] == '#'){
            free(line);
            free(tokens);
            continue;
        }
        if(num_tokens > 1){
            char *args[num_tokens];
            int redir = 0x0; // 0x1 redir in, 0x2 redirout, 0x4 redirerr
            int end_args = 0;
            char *infile = NULL;
            char *outfile = NULL;
            for(int i = 0; i < num_tokens; i++){
                char *token;
                // Handle out redirect
                if((token = strchr(tokens[i], '>')) != NULL 
                    && (strstr(tokens[i], "2>") == NULL)){
                    redir |= 0x2; 

                    outfile = (char *)malloc(strlen(tokens[i]));
                    strcpy(outfile, tokens[i]);
                    
                    if(!end_args){
                        end_args = 1;
                        args[i] = NULL;
                    }
                }
                // Handle err redirect
                if((token = strstr(tokens[i], "2>")) != NULL){
                    redir |= 0x4;

                    outfile = (char *)malloc(strlen(tokens[i]));
                    strcpy(outfile, tokens[i]);

                    if(!end_args){
                        end_args = 1;
                        args[i] = NULL;
                    }
                }
                // Handle in redirect
                if(strchr(tokens[i], '<') != NULL){
                    redir |= 0x1;

                    infile = (char *)malloc(strlen(tokens[i]));
                    strcpy(infile, tokens[i]);

                    if(!end_args){
                        end_args = 1;
                        args[i] = NULL;
                    }
                }
                if(!end_args) args[i] = tokens[i];
            }
            if(redir){
                if(debug){
                    perror("RUNNING: redirect\n");
                }
                int ret = smash_redirect(redir, infile, outfile, args);
                char buff[2];
                sprintf(buff, "%d", ret);
                st_insert("$?", buff);
                if(infile) free(infile);
                if(outfile) free(outfile);
                if(debug){
                    fprintf(stderr, "ENDED: redirect (ret=%d)\n", ret);
                }
                cmd_exec = 1;
            }
        }
        // if line contains = or $ variable handling
        if(strchr(line, '=') != NULL){
            cmd_exec = 1;
            if(debug){
                perror("RUNNING: variable assignment\n");
            }
            int ret = smash_assign_var();
            set_ret_var(ret);
            if(debug){
                fprintf(stderr, "ENDED: variable assignment (ret=%d)\n", ret);
            }
        }
        if(strcmp(line, "exit") == 0){
            smash_exit();
        }
        if(strcmp(tokens[0], "pwd") == 0){
            cmd_exec = 1;
            if(debug){
                perror("RUNNING: pwd\n");
            }
            int ret = smash_pwd();
            set_ret_var(ret);
            if(debug){
                fprintf(stderr, "ENDED: pwd (ret=%d)\n", ret);
            }
        }
        if(strcmp(tokens[0], "cd") == 0){
            cmd_exec = 1;
            if(debug){
                perror("RUNNING: cd\n");
            }
            int ret = smash_cd(tokens[1]);
            set_ret_var(ret);
            if(debug){
                fprintf(stderr, "ENDED: cd (ret=%d)\n", ret);
            }
        }
        if(strcmp(tokens[0], "echo") == 0){
            cmd_exec = 1;
            if(debug){
                perror("RUNNING: echo\n");
            }
            int ret = smash_echo();
            set_ret_var(ret);
            if(debug){
                fprintf(stderr, "ENDED: echo (ret=%d)\n", ret);
            }
        }
        if(strcmp(tokens[0], "jobs") == 0){
            cmd_exec = 1;
            if(debug){
                perror("RUNNING: jobs\n");
            }
            int ret = job_print(&jobs);
            set_ret_var(ret);
            if(debug){
                fprintf(stderr, "ENDED: jobs (ret=%d)\n", ret);
            }
        }
        if(strcmp(tokens[0], "fg") == 0){
            cmd_exec = 1;
            if(debug){
                perror("RUNNING: fg\n");
            }
            smash_foreground();
        }
        if(strcmp(tokens[0], "bg") == 0){
            cmd_exec = 1;
            smash_background();
        }
        if(strcmp(tokens[0], "kill") == 0){
            cmd_exec = 1;
            if(debug){
                perror("RUNNING: kill\n");
            }
            int ret = smash_kill();
            set_ret_var(ret);
            if(debug){
                fprintf(stderr, "ENDED: kill (ret=%d)\n", ret);
            }
        }
        if(!cmd_exec){
            // send job to bg

            if(strcmp(tokens[num_tokens-1], "&") == 0){
                char *args[num_tokens];
                for(int i = 0; i < (num_tokens-1); i++){
                    args[i] = tokens[i];
                }
                args[num_tokens-1] = NULL;

                if(debug){
                    fprintf(stderr, "RUNNING: %s\n", args[0]);
                }
                smash_cmd(BACKGROUND, args);
            }else{
                // send job to fg
                char *args[num_tokens + 1];
                for(int i = 0; i < num_tokens; i++){
                    args[i] = tokens[i];
                }
                args[num_tokens] = NULL;

                if(debug){
                    fprintf(stderr, "RUNNING: %s\n", args[0]);
                }
                smash_cmd(FOREGROUND, args);
            }
        }
        free(line);
        free(tokens);
    }
}

void set_ret_var(int ret){
    char buff[2];
    sprintf(buff, "%d", ret);
    st_insert("$?", buff);
}

int smash_redirect(int redir, char *infile, char *outfile, char **args){
    signal(SIGCHLD, SIG_DFL);
    int infile_len = 0;
    int outfile_len = 0;

    if(infile) infile_len = strlen(infile);
    if(outfile) outfile_len = strlen(outfile);

    int saved_stdin = 0;
    int saved_stdout = 0;
    int saved_stderr = 0;

    if(redir & IN_REDIR){
        char src[infile_len];
        int srcfd;
        int c = 0;
        while(c < (infile_len - 1)){
            src[c] = infile[c+1];
            c++;
        }
        src[infile_len - 1] = '\0';
        saved_stdin = dup(STDIN_FILENO);
        if((srcfd = open(src, O_RDONLY)) < 0){
            perror("open failed");
            return 1;
        }
        dup2(srcfd, STDIN_FILENO);
        close(srcfd);
    }

    if(outfile){
        fflush(stdout);
        char dest[outfile_len];
        int destfd;
        if(redir & ERR_REDIR){
            int c = 0;
            while(c < (outfile_len - 2)){
                dest[c] = outfile[c+2];
                c++;
            }
            dest[outfile_len - 2] = '\0';
            saved_stderr = dup(STDERR_FILENO);
            if((destfd = open(dest, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0){
                perror("open failed");
                return 1;
            }
            dup2(destfd, STDERR_FILENO);
            close(destfd);
        }
        if(redir & OUT_REDIR){
            int c = 0;
            while(c < (outfile_len - 1)){
                dest[c] = outfile[c+1];
                c++;
            }
            dest[outfile_len - 1] = '\0';
            saved_stdout = dup(STDOUT_FILENO);
            if((destfd = open(dest, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0){
                perror("open failed");
                return 1;
            }
            dup2(destfd, STDOUT_FILENO);
            close(destfd);
        }
    }

    // Handle special cmds
    int cmd_exec = 0;
    if(strcmp(tokens[0], "pwd") == 0){
        cmd_exec = 1;
        int ret = smash_pwd();
        char buff[2];
        sprintf(buff, "%d", ret);
        st_insert("$?", buff);
    }
    if(strcmp(tokens[0], "echo") == 0){
        cmd_exec = 1;
        int ret = smash_echo();
        char buff[2];
        sprintf(buff, "%d", ret);
        st_insert("$?", buff);
    }
    if(strcmp(tokens[0], "jobs") == 0){
        cmd_exec = 1;
        job_print(&jobs);
    }

    // Regular cmds
    if(!cmd_exec){
        child = fork();
        if(child < 0){
            perror("fork() error");
            return 1;
        }
        if(child == 0){
            if(execvp(tokens[0], args) == -1){
                perror("execvp() failed");
                return 1;
            }
        }
        wait(NULL);
    }

    if(redir & IN_REDIR){
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
    }
    if(redir & OUT_REDIR){
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
    }
    if(redir & ERR_REDIR){
        dup2(saved_stderr, STDERR_FILENO);
        close(saved_stderr);
    }
    signal(SIGCHLD, sigchld_handler);
    return 0;
}

int smash_assign_var(){
    char *temp = strtok(tokens[0], "=");
    char *var_name =(char *)malloc(strlen(temp));
    strcpy(var_name, temp);
    temp = strtok(NULL, "");
    char *value = (char *)malloc(strlen(temp));
    strcpy(value, temp);
    st_insert(var_name, value);
    return 0;
}

int smash_echo(){
    if(tokens[1][0] == '$'){
        // Parse var name
        int var_name_len = strlen(tokens[1]);
        char var_name[var_name_len];
        int c = 0;
        while(c < var_name_len){
            var_name[c] = tokens[1][c + 1]; // skip $
            c++;
        }
        // Get var from symbol table
        char *value = (char *)st_get(var_name);
        // Output
        if(value == NULL){
            printf("Error variable does not exist\n");
            return 1;
        }else{
            printf("%s\n", value);
        }

    } else{
        int i = 1;
        char *buff = (char *)malloc(RL_BUFFSIZE);
        while(tokens[i]){ // compare to space and EOF?
            strcat(buff, tokens[i]);
            strcat(buff, " ");
            i++;
        }
        strcat(buff, "\n");
        strcat(buff, "\0");
        printf("%s", buff);
    }
    return 0;
}

int smash_kill(){
    int signal;
    int pid;
    char pid_buff[strlen(tokens[2])];
    char sig_buff[strlen(tokens[1])-1];
    int c = 0;
    while(c < (strlen(tokens[1])-1)){
        sig_buff[c] = tokens[1][c+1];
        c++;
    }
    sscanf(sig_buff, "%d", &signal);

    c = 0;
    while(c < strlen(tokens[2])){
        pid_buff[c] = tokens[2][c];
        c++;
    }
    sscanf(pid_buff, "%d", &pid);
    if(kill(pid, signal) < 0){
        perror("kill failed");
        return 1;
    }
    return 0;
}

void smash_cmd(int ground, char **args){
    job_foreground = ground;
    child = fork();
    target_proc = child;

    if(child < 0){
        perror("fork() error");
        exit(1);
    }
    if(child == 0){
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);

        // Child is its own group leader
        if(setpgid(0, 0) < 0){
            perror("setpgid 2");
            exit(1);
        }
        // Transfer controlling terminal
        if(tcsetpgrp(STDIN_FILENO, getpgrp()) < 0){
            perror("child tcsetpgrp");
            exit(1);
        }
        if(execvp(tokens[0], args) == -1)
            printf("Failed to execute %s\n", tokens[0]);
    }else{
        // Synchronize
        if(setpgid(child, child) < 0){
            if(errno != EACCES){
                perror("setpgid 3");
                exit(1);
            }
        }
        // Synchronize
        if(tcsetpgrp(STDIN_FILENO, getpgid(child)) < 0){
            perror("tcsetpgrp 2");
            exit(1);
        }

        if(ground == BACKGROUND){
            job_insert(&jobs, child, 1, tokens[0]);
            job_print(&jobs);
        }else{
            job_insert(&jobs, child, 1, tokens[0]);
        }
    }
}

void smash_background(){
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    int job_spec;
    sscanf(tokens[1], "%d", &job_spec);
    struct job *target_job = job_get(&jobs, job_spec);
    job_update(&jobs, target_job->pid, 1); // change job state to running

    kill(target_job->pid, SIGSTOP);
    
    if(tcsetpgrp(STDIN_FILENO, (target_job->pid)) < 0){
        perror("tcsetpgrp 2");
        exit(1);
    }
    
    kill(target_job->pid, SIGCONT);

    signal(SIGTTOU, sigttou_handler);

    signal(SIGTTOU, SIG_IGN);
    if(tcsetpgrp(STDIN_FILENO, getpgrp()) < 0){
        perror("tcsetpgrp 2");
        exit(1);
    }
    signal(SIGTTOU, sigttou_handler);
    signal(SIGCHLD, sigchld_handler);
}

void smash_foreground(){
    signal(SIGCHLD, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    int job_spec;
    sscanf(tokens[1], "%d", &job_spec);
    struct job *target_job = job_get(&jobs, job_spec);
    job_update(&jobs, target_job->pid, 1); // change job state to running

    kill(target_job->pid, SIGSTOP);

    if(tcsetpgrp(STDIN_FILENO, (target_job->pid)) < 0){
        perror("tcsetpgrp 2");
        exit(1);
    }
    kill(target_job->pid, SIGCONT);


    signal(SIGTTOU, sigttou_handler);
    waitpid(target_job->pid, &status, WUNTRACED);

    signal(SIGTTOU, SIG_IGN);
    if(tcsetpgrp(STDIN_FILENO, getpgrp()) < 0){
        perror("tcsetpgrp 2");
        exit(1);
    }
    signal(SIGTTOU, sigttou_handler);
    signal(SIGCHLD, sigchld_handler);
}

int smash_pwd(){
    char *pwd = (char *)malloc(255);
    if(getcwd(pwd, 255) == NULL){
        free(pwd);
        free(line);
        free(tokens);
        perror("pwd failed\n");
        return 1;
    }
    printf("%s\n", pwd);
    free(pwd);
    return 0;
}

int smash_cd(char *path){
    if(chdir(path) != 0){
        perror("chdir failed\n");
        return 1;
    }
    return 0;
}

void smash_exit(){
    free(line);
    free(tokens);
    job_free_all(&jobs);
    exit(0);
}

void load_envp(char **envp){
    st_init();
    char *ls_color = strtok(*envp, "=");
    char *ls_color_val = strtok(NULL, "");
    st_insert(ls_color, ls_color_val);
    int i = 1;
    while(envp[i]){
        char *var_name = strtok(envp[i], "=");
        char *value = strtok(NULL, "=");
        st_insert(var_name, value);
        i++;
    }
    st_insert("?", "0");
}

char **get_tokens(char *line){
    int i = 0;
    char **words = (char **)malloc(RL_BUFFSIZE);
    char *pch = strtok(line, " ");
    while(pch != NULL){
        if(strchr(pch, '\n') != NULL){
            pch[strlen(pch) - 1] = '\0';
        }
        words[i] = pch;
        i++;
        pch = strtok(NULL, " ");
    }
    words[i] = NULL;
    return words;
}

int get_tokens_len(){
    int ctr = 0;
    while(tokens[ctr]){
        ctr++;
    }
    return ctr;
}

char *get_abs_path(char** argv){
    char *cwd = malloc(PATH_MAX);
    if(getcwd(cwd, PATH_MAX) == NULL){
        perror("getcwd() error");
        exit(1);
    }
    char *prog = strtok(argv[0], ".");
    strcat(cwd, prog);
    return cwd;
}
