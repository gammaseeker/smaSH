#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#define SYMBOL_TABLE_SIZE 1024

// var_names[i] maps to values[i]
void *values[SYMBOL_TABLE_SIZE];
char *var_names[SYMBOL_TABLE_SIZE];

void st_init();
int st_insert(char *var_name, void *value);
int st_update(char *var_name, void *value);
void *st_get(char *var_name);
void st_print();

#endif /*SYMBOL_TABLE_H */
