#ifndef LIBCONF_H
#define LIBCONF_H

char* create_file(char* file);
void delete_file(char* path);
char* make_variable(char* name, char* value);
int is_variable_exists(char* file, char* variable);
int insert_variable(char* file, char* string);
int delete_variable(char* file, char* variable);
int rewrite_variable(char* file, char* variable, char* new_value);
char* get_variable(char* file, char* variable);
char** split_values(char* file, char* variable_name, int* size, char* delim);
char* get_split_from_values(char** tokens, int size, int index);
void free_split_values(char** tokens, int size);
void show_split_values(char** tokens, int size);
int show_content(char* file);

#endif
