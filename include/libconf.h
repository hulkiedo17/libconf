#ifndef LIBCONF_H
#define LIBCONF_H

typedef struct token {
	char* string;
	int len;
	int index;
	struct token* next;
} token_t;

typedef struct split {
	token_t* head;
	int size;
} split_t;

char* create_file(char* file);
void delete_file(char* path);
int is_variable_exists(char* file, char* variable);
int insert_variable(char* file, char* name, char* value);
int delete_variable(char* file, char* variable);
int rewrite_variable(char* file, char* variable, char* new_value);
char* get_variable(char* file, char* variable);
//char** split_values(char* file, char* variable_name, int* size, char* delim);
//char* get_split_from_values(char** tokens, int size, int index);
//void free_split_values(char** tokens, int size);
//void show_split_values(char** tokens, int size);
int show_content(char* file);

#endif
