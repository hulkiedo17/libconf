#ifndef LIBCONF_H
#define LIBCONF_H

// direct access to file
int write_to_file(char* filename, char* name, char* value);
int append_to_file(char* filename, char* name, char* value);
char* read_variable(char* filename, char* name);
int print_file(char* filename);

// access to file through buffer
char* read_file_to_buffer(char* filename);
int write_buffer_to_file(char* filename, char* buffer);
int append_buffer_to_file(char* filename, char* buffer);
char* delete_variable_from_buffer(char* buffer, char* name);
char* insert_variable_to_buffer(char* buffer, char* name, char* value);

#endif
