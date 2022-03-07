#ifndef LIBCONF_H
#define LIBCONF_H

int write_to_file(char* filename, char* name, char* value);
int insert_to_file(char* filename, char* name, char* value);
char* read_variable(char* filename, char* name);
int is_var_exists(char* file, char* name);
int print_file(char* filename);
int delete_from_file(char* filename, char* name);

#endif
