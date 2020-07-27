#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

int main(int argc, char **argv, char **envp){
    /*
    for(char **env = envp; *env != 0; env++){
        char *thisEnv = *env;
        printf("%s\n", thisEnv);
    }
    char *home = getenv("HOME");
    char *path = getenv("PATH");
    printf ("Your home directory is %s PATH is %s.\n", home, path);
    */

    st_init();
    // Handle ls_color
    char *ls_color = strtok(*envp, "=");
    //printf("%s\n", ls_color);
    char *ls_color_value = strtok(NULL, "");
    //printf("%s\n", ls_color);

    st_insert(ls_color, ls_color_value);
    int i = 1;
    while(envp[i]){
        char *var_name = strtok(envp[i], "=");
        //printf("%s\n", var_name);
        char *value = strtok(NULL, "=");
        //printf("%s\n", value);
        st_insert(var_name, value);
        i++;
    }
    st_print();
    st_update("PATH", "deez nutz");

    st_print();
    printf("%s\n", (char *)st_get("PATH"));
    return 0;
}
