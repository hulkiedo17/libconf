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
	char* name;
	int size;
} split_t;

char* create_file(char* file);
void delete_file(char* path);
int is_variable_exists(char* file, char* variable);
int insert_variable(char* file, char* name, char* value);
int delete_variable(char* file, char* variable);
int rewrite_variable(char* file, char* variable, char* new_value);
char* get_variable(char* file, char* variable);
int show_content(char* file);

split_t* split_variable(char* file, char* name, char* delim);
char* get_token_split(split_t* tokens, int index);
void free_split(split_t* tokens);
void print_split(split_t* tokens);

#endif
