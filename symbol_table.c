#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

void st_init(){
    int i;
    for(i = 0; i < SYMBOL_TABLE_SIZE; i++){
        var_names[i] = NULL;
        values[i] = NULL;
    }
}

int st_insert(char *var_name, void *value){
    int i;
    for(i = 0; i < SYMBOL_TABLE_SIZE; i++){
        if(var_names[i] == NULL && values[i] == NULL){
            var_names[i] = var_name;
            values[i] = value;
            return 0;
        }
    }
    return 1;
}

int st_update(char *var_name, void *value){
    int i;
    for(i = 0; i < SYMBOL_TABLE_SIZE; i++){
        if(strcmp(var_names[i], var_name) == 0){
            values[i] = value;
            return 0;
        }
    }
    return 1;
}

void *st_get(char *var_name){
    int i;
    for(i = 0; i < SYMBOL_TABLE_SIZE; i++){ 
        if(var_names[i] == NULL){
            return NULL;
        }
        if(strcmp(var_names[i], var_name) == 0){
            return values[i];
        }
    }
    return NULL;
}

void st_print(){
    int i = 0;
    while(var_names[i]){
        printf("var_names[%d] = %s\n", i, (char *)var_names[i]);
        printf("values[%d] = %s\n", i, (char *)values[i]);
        i++;
    }
}
