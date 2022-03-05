#ifndef LIBCONF_H
#define LIBCONF_H

FILE* open_file(char* filename, char* mode);
void write_var(FILE* fp, char* var, char* value);
char* read_var(FILE* fp, char* var);
//char* delete_var(FILE* fp, char* var);
void print_all(FILE* fp);

#endif
