#ifndef LIBCONF_H
#define LIBCONF_H

typedef struct lc_token {
	char* string;
	size_t len;
	size_t index;
	struct lc_token* next;
} lc_token_t;

typedef struct lc_split {
	lc_token_t* head;
	char* name;
	size_t size;
} lc_split_t;

char* lc_create_config(char* file);
void lc_delete_config(char* path);
int lc_var_exists(char* file, char* variable);
int lc_insert_var(char* file, char* name, char* value);
int lc_delete_var(char* file, char* variable);
int lc_rewrite_var(char* file, char* variable, char* new_value);
char* lc_get_var(char* file, char* variable);
void lc_display_config(char* file);

lc_split_t* lc_split_var(char* file, char* name, char* delim);
char* lc_get_token(lc_split_t* tokens, size_t index);
void lc_free_split(lc_split_t* tokens);
void lc_print_tokens(lc_split_t* tokens);

#endif
