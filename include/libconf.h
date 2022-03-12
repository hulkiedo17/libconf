#ifndef LIBCONF_H
#define LIBCONF_H

char* create_file(char* path);
char* make_variable(char* name, char* value);
int is_variable_exists(char* file, char* variable);
int insert_variable(char* file, char* string);
//int delete_variable(char* file, char* variable);
char* get_variable(char* file, char* variable);
int show_content(char* file);

#endif
