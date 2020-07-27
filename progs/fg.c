void fgCommand(char *args[]){
    //We need to move all background processes to foreground.
    for(int i = 0; background_processes.background_process_count > 0 && i < MAX_BACKGROUND_PROCESS; i++){

        //First of all, until this point the user might have closed the process, so we need to check it.
        checkBackgroundProcesses();

        //If that process is closed, we can skip over.
        if(background_processes.process_pids[i] == 0) continue;

        //Printing the process name.
        printf("The process with the p_id = %d and the name \"%s\" is running in the foreground now...\n",
                    background_processes.process_pids[i], background_processes.process_names[i]);

        //Before moving the process to the foreground, we stop it.
        kill(background_processes.process_pids[i], SIGTSTP);

        //Setting the io_ports to the same as the main process.
        tcsetpgrp(main_process_pid, background_processes.process_pids[i]);

        //After moving the process to the foreground, we continue it.
        kill(background_processes.process_pids[i], SIGCONT);

        //And waiting the process to end. Also we need to set the variables because it is a foreground process now.
        fg_process_pid = background_processes.process_pids[i];
        strcpy(fg_process_name, background_processes.process_names[i]);
        waitpid(fg_process_pid, NULL, WUNTRACED);

        //If ctrl-z is pressed we need to exit fg. But we also need to delete it from the bg list.
        if(fg_process_pid == 0){

            background_processes.process_pids[i] = 0;
            background_processes.background_process_count--;

            break;
        }

        printf("\nProcess terminated!!!\n");

        //Setting the variables back.
        background_processes.process_pids[i] = 0;
        background_processes.background_process_count--;
        fg_process_pid = 0;
    }
    //And lastly, we need to set the shell back.
    tcsetpgrp(STDIN_FILENO, main_process_pid);
    tcsetpgrp(STDOUT_FILENO, main_process_pid);
}
