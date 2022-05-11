#ifndef LIBCONF_H
#define LIBCONF_H

#define LINE_SIZE 256
#define LC_SUCCESS 0
#define LC_ERROR 1

enum _lc_config_error
{
	LC_ERR_NONE = 0,
	LC_ERR_EMPTY = 1,
	LC_ERR_FILE_NO = 2,
	LC_ERR_MEMORY_NO = 3,
	LC_ERR_WRITE_NO = 4,
	LC_ERR_NOT_EXISTS = 5
};

typedef enum lc_existence
{
	LC_EF_ERROR = 0,
	LC_EF_EXISTS = 1,
	LC_EF_NOT_EXISTS = 2
} lc_existence_t;

typedef struct lc_config_variable
{
	char * name;
	char * value;
} lc_config_variable_t;

struct _lc_config_list
{
	lc_config_variable_t *variable;
	struct _lc_config_list *next;
};

typedef struct lc_config
{
	struct _lc_config_list *list;
	size_t list_size;
	enum _lc_config_error error_type;
	// char *file;
} lc_config_t;


// basic functions for init and clear config
void lc_init_config(lc_config_t *config);

void lc_clear_config(lc_config_t *config);

size_t lc_get_size(const lc_config_t *config);

int lc_is_empty(const lc_config_t *config);


// io functions for config
int lc_load_config(lc_config_t *config, const char *filename);

int lc_dump_config(lc_config_t *config, const char *filename);

void lc_print_config(const lc_config_t *config);

int lc_print_error(const lc_config_t *config);

// (add file to config struct, and 2 funcs to work with it)
// load_config(FILE)
// dump_config(FILE)


// functions for editing variables in config 
int lc_add_variable(lc_config_t *config, const char *name, const char *value);

int lc_delete_variable(lc_config_t *config, const char *name);

lc_existence_t lc_is_variable_in_config(lc_config_t *config, const char *name);

int lc_set_variable(lc_config_t *config, const char *name, const char *new_value);

lc_config_variable_t* lc_get_variable(lc_config_t *config, const char *name);

// int add_variable() // variable, not 2 char pointers
// int delete_variable() // variable, not char pointer
// ___ is_var_in_config() // variable, not char pointer
// int replace_variable(var, var)
// int replace_variable(char*, var)


// functions for editing variables
// char* get_variable_name()
// char* get_variable_value()
// int create_variable()
// void destroy_variable()
// int set_variable_name()
// int set_variable_value()
// void print_variable()
//
// set_*, get_* -> with int(u and s), float, double

#endif
