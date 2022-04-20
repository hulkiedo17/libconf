#ifndef LIBCONF_H
#define LIBCONF_H

#include <stddef.h>

typedef struct lc_config {
	char* buffer;
	size_t len;
} lc_config_t;

/*
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
*/

lc_config_t* create_empty_config(void);
lc_config_t* create_config(const char* buffer);
lc_config_t* lc_load_config(const char* path);
int lc_dump_config(const lc_config_t* config, const char* path);
void lc_print_config(const lc_config_t* config);
void lc_free_config(lc_config_t* config);

/*
char* lc_create_config(const char* file);
void lc_delete_config(const char* path);
int lc_var_exists(const char* file, const char* variable);
int lc_insert_var(const char* file, const char* name, const char* value);
int lc_delete_var(const char* file, const char* variable);
int lc_rewrite_var(const char* file, const char* variable, const char* new_value);
char* lc_get_var(const char* file, const char* variable);
void lc_display_config(const char* file);

lc_split_t* lc_split_var(const char* file, const char* name, const char* delim);
char* lc_get_token(lc_split_t* tokens, size_t index);
void lc_free_split(lc_split_t* tokens);
void lc_print_tokens(lc_split_t* tokens);
*/

#endif
