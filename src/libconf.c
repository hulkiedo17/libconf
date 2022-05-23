#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "libconf.h"

#if defined DEBUG
static void warning(FILE *out, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(out, fmt, ap);
	va_end(ap);
}
#else
#define warning(...)
#endif

static FILE* _file_open(const char *filename, const char *mode)
{
	assert(filename != NULL);
	assert(mode != NULL);

	FILE *fp = fopen(filename, mode);
	if(fp == NULL) {
		warning(stderr, "[ERROR] %s: fopen() failed\n", __func__);
		exit(EXIT_FAILURE);
	}

	return fp;
}

static char* _duplicate_string(const char *string)
{
	assert(string != NULL);

	size_t length = strlen(string) + 1;

	char *duplicate = calloc(length, sizeof(char));
	if(duplicate == NULL) {
		warning(stderr, "[ERROR] %s: allocation failed\n", __func__);
		exit(EXIT_FAILURE);
	}

	memcpy(duplicate, string, length);

	return duplicate;
}

static char* _find_delimiter(const char * string, const char * delim)
{
	assert(string != NULL);
	assert(delim != NULL);

	return strstr(string, delim);
}

static char* _read_line_from_file(FILE *fp)
{
	assert(fp != NULL);

	size_t line_length = LINE_SIZE;

	char *line_buffer = calloc(line_length, sizeof(char));
	if(line_buffer == NULL)
	{
		warning(stderr, "[ERROR] %s: allocation failed\n", __func__);
		exit(EXIT_FAILURE);
	}

	int c;
	size_t position = 0;

	while(1)
	{
		c = fgetc(fp);

		if(c == EOF || c == '\n')
		{
			if(position == 0 && (c == EOF || c == '\n'))
			{
				free(line_buffer);
				return NULL;
			}

			line_buffer[position] = '\0';
			return line_buffer;
		}
		else
		{
			line_buffer[position] = c;
		}

		position++;

		if(position >= line_length)
		{
			// TODO: make temporary pointer
			line_length += LINE_SIZE;
			line_buffer = realloc(line_buffer, line_length);
			if(line_buffer == NULL)
			{
				warning(stderr, "[ERROR] %s: allocation failed\n", __func__);
				exit(EXIT_FAILURE);
			}
		}
	}

	return NULL;
}

static int _write_line_to_file(FILE *fp, const char *line)
{
	assert(fp != NULL);
	assert(line != NULL);

	if(fwrite(line, 1, strlen(line), fp) != strlen(line))
		return LC_ERROR;

	return LC_SUCCESS;
}

// config list functions

static void _free_config_variable(lc_config_variable_t *variable)
{
	if(variable == NULL)
		return;

	free(variable->name);
	free(variable->value);
	free(variable);
}

static void _free_list_element(struct _lc_config_list *element)
{
	if(element == NULL)
		return;

	_free_config_variable(element->variable);
	free(element);
}

static lc_config_variable_t* _make_config_variable(const char *name, const char *value)
{
	lc_config_variable_t *new_variable = NULL;

	new_variable = malloc(sizeof(lc_config_variable_t));
	if(new_variable == NULL) {
		warning(stderr, "[ERROR] %s: allocation failed\n", __func__);
		exit(EXIT_FAILURE);
	}

	new_variable->name = _duplicate_string(name);
	if(new_variable->name == NULL)
	{
		free(new_variable);
		return NULL;
	}

	new_variable->value = _duplicate_string(value);
	if(new_variable->value == NULL)
	{
		free(new_variable->name);
		free(new_variable);
		return NULL;
	}

	return new_variable;
}

static lc_config_variable_t* _create_variable_copy(lc_config_variable_t *variable)
{
	assert(variable != NULL);

	if(variable->name == NULL)
		return NULL;

	if(variable->value == NULL)
		return NULL;

	return _make_config_variable(variable->name, variable->value);
}

static struct _lc_config_list* _create_list_element(lc_config_variable_t *variable)
{
	assert(variable != NULL);

	struct _lc_config_list *element = NULL;

	element = malloc(sizeof(struct _lc_config_list));
	if(element == NULL) {
		warning(stderr, "[ERROR] %s: allocation failed\n", __func__);
		exit(EXIT_FAILURE);
	}

	element->variable = variable;
	element->next = NULL;

	return element;
}

static lc_config_variable_t* _convert_line_to_variable(const char *line, const char *delim)
{
	assert(line != NULL);
	assert(delim != NULL);

	char *dup_line = _duplicate_string(line);
	if(dup_line == NULL)
		return NULL;


	if(_find_delimiter(line, delim) == NULL)
	{
		free(dup_line);
		warning(stderr, "[WARNING] %s: cannot find \"%s\" delimiter in line \"%s\"\n", __func__, delim, line);
		return NULL;
	}

	char *name = strtok(dup_line, delim);
	char *value = strtok(NULL, delim);

	lc_config_variable_t *variable = _make_config_variable(name, value);

	free(dup_line);
	return variable;
}

static char* _convert_variable_to_line(lc_config_variable_t *variable, const char *delim)
{
	assert(variable != NULL);
	assert(delim != NULL);

	// +2 because we also add: '\n', '\0'
	size_t length = strlen(variable->name) + strlen(variable->value) + strlen(delim) + 2;

	char *line = calloc(length, sizeof(char));
	if(line == NULL) {
		warning(stderr, "[ERROR] %s: allocation failed\n", __func__);
		exit(EXIT_FAILURE);
	}

	if(snprintf(line, length, "%s%s%s\n", variable->name, delim, variable->value) != ((int)length - 1))
	{
		free(line);
		// TODO: error check
		return NULL;
	}

	return line;
}

static void _print_list(struct _lc_config_list *list)
{
	assert(list != NULL);

	struct _lc_config_list *head = list;

	while(head != NULL)
	{
		printf("%s=%s\n", head->variable->name, head->variable->value);
		head = head->next;
	}
	printf("\n");
}

static int _add_list_element(lc_config_t *config, lc_config_variable_t *variable)
{
	assert(config != NULL);
	assert(variable != NULL);

	struct _lc_config_list *element = NULL;

	element = _create_list_element(variable);
	if(element == NULL)
	{
		config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	if(config->list == NULL)
	{
		config->list = element;
		config->error_type = LC_ERR_NONE;
		config->list_size++;
		return LC_SUCCESS;
	}

	struct _lc_config_list *temp = config->list;
	while(temp->next != NULL)
		temp = temp->next;

	temp->next = element;

	config->error_type = LC_ERR_NONE;
	config->list_size++;
	return LC_SUCCESS;
}

static struct _lc_config_list* _find_list_element(lc_config_t *config, const char *name)
{
	assert(config != NULL);
	assert(name != NULL);

	struct _lc_config_list *head = config->list;

	while(head != NULL)
	{
		if(strcmp(head->variable->name, name) == 0)
		{
			config->error_type = LC_ERR_NONE;
			return head;
		}

		head = head->next;
	}

	config->error_type = LC_ERR_NOT_EXISTS;
	return NULL;
}

static struct _lc_config_list* _find_prev_list_element(lc_config_t *config, const char *name)
{
	assert(config != NULL);
	assert(name != NULL);

	struct _lc_config_list *prev = NULL;
	struct _lc_config_list *head = config->list;

	while(head != NULL)
	{
		if(strcmp(head->variable->name, name) == 0)
		{
			if(prev == NULL)
			{
				config->error_type = LC_ERR_NOT_EXISTS;
				return NULL;
			}

			config->error_type = LC_ERR_NONE;
			return prev;
		}

		prev = head;
		head = head->next;
	}

	config->error_type = LC_ERR_NOT_EXISTS;
	return NULL;
}

static int _delete_list_element(lc_config_t *config, const char *name)
{
	assert(config != NULL);
	assert(name != NULL);

	if(config->list == NULL)
	{
		config->error_type = LC_ERR_EMPTY;
		return LC_ERROR;
	}

	struct _lc_config_list *temp = NULL;
	struct _lc_config_list *element = NULL;
	struct _lc_config_list *prev_element = NULL;

	if((element = _find_list_element(config, name)) == NULL)
	{
		config->error_type = LC_ERR_NOT_EXISTS;
		return LC_ERROR;
	}

	if((prev_element = _find_prev_list_element(config, name)) == NULL)
	{
		config->list = config->list->next;
		_free_list_element(element);

		config->error_type = LC_ERR_NONE;
		config->list_size--;
		return LC_SUCCESS;
	}

	temp = element;
	prev_element->next = element->next;

	_free_list_element(temp);

	config->error_type = LC_ERR_NONE;
	config->list_size--;
	return LC_SUCCESS;
}

static void _delete_list(struct _lc_config_list *list)
{
	if(list == NULL)
		return;

	struct _lc_config_list *head = list;
	struct _lc_config_list *temp = NULL;

	while(head != NULL)
	{
		temp = head;
		head = head->next;

		_free_list_element(temp);
	}
}

static int _rewrite_list_element_value(lc_config_t *config, const char *name, const char *new_value)
{
	assert(config != NULL);
	assert(name != NULL);
	assert(new_value != NULL);

	struct _lc_config_list *element = NULL;

	if((element = _find_list_element(config, name)) == NULL)
		return LC_ERROR;

	free(element->variable->value);
	element->variable->value = _duplicate_string(new_value);

	config->error_type = LC_ERR_NONE;
	return LC_SUCCESS;
}

static int _replace_variable_in_list(struct _lc_config_list *list, lc_config_variable_t *variable)
{
	assert(list != NULL);
	assert(variable != NULL);

	_free_config_variable(list->variable);
	list->variable = variable;

	return LC_SUCCESS;
}

// io functions

static int _read_file_to_config(lc_config_t *config, FILE *fp)
{
	assert(config != NULL);
	assert(fp != NULL);

	char *line = NULL;
	lc_config_variable_t * variable = NULL;

	while((line = _read_line_from_file(fp)) != NULL)
	{
		if((variable = _convert_line_to_variable(line, config->delim)) == NULL)
		{
			free(line);
			config->error_type = LC_ERR_MEMORY_NO;
			// TODO: error check
			//return LC_ERROR;
			continue;
		}

		if(_add_list_element(config, variable) == LC_ERROR)
		{
			free(line);
			free(variable);
			return LC_ERROR;
		}

		free(line);
	}

	return LC_SUCCESS;
}

static int _dump_config_to_file(lc_config_t *config, FILE *fp)
{
	assert(config != NULL);
	assert(fp != NULL);

	if(config->list == NULL)
	{
		config->error_type = LC_ERR_EMPTY;
		return LC_ERROR;
	}

	char *line = NULL;
	struct _lc_config_list *head = config->list;

	while(head != NULL)
	{
		if((line = _convert_variable_to_line(head->variable, config->delim)) == NULL)
		{
			config->error_type = LC_ERR_MEMORY_NO;
			return LC_ERROR;
		}

		if(_write_line_to_file(fp, line) == LC_ERROR)
		{
			free(line);
			config->error_type = LC_ERR_WRITE_NO;
			return LC_ERROR;
		}

		free(line);
		head = head->next;
	}

	config->error_type = LC_ERR_NONE;
	return LC_SUCCESS;
}

// api functions

int lc_init_config(lc_config_t *config, const char *filepath, const char *delim)
{
	if(config == NULL || delim == NULL)
	{
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);
		return LC_ERROR;
	}

	config->list = NULL;
	config->list_size = 0;
	config->delim = NULL;

	if(filepath != NULL)
	{
		if((config->filepath = _duplicate_string(filepath)) == NULL)
		{
			config->error_type = LC_ERR_MEMORY_NO;
			return LC_ERROR;
		}
	}
	else
	{
		config->filepath = NULL;
	}

	if((config->delim = _duplicate_string(delim)) == NULL)
	{
		free(config->filepath);
		config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	config->error_type = LC_ERR_NONE;
	return LC_SUCCESS;
}

int lc_load_config(lc_config_t *config, const char *filepath)
{
	if(config == NULL)
	{
		warning(stderr, "[WARNING] %s: argument is null\n", __func__);
		return LC_ERROR;
	}

	/*if(_file_exists(filepath) != 0)
	{
		config->error_type = LC_ERR_FILE_NO;
		return LC_ERROR;
	}*/

	FILE *fp = NULL;

	if(filepath == NULL) 
	{
		if(config->filepath == NULL)
		{
			config->error_type = LC_ERR_FILE_NO;
			return LC_ERROR;
		}

		fp = _file_open(config->filepath, "r");
	}
	else 
	{
		fp = _file_open(filepath, "r");
	}

	if(fp == NULL)
	{
		config->error_type = LC_ERR_FILE_NO;
		return LC_ERROR;
	}

	if(_read_file_to_config(config, fp) == LC_ERROR)
	{
		fclose(fp);
		return LC_ERROR;
	}

	fclose(fp);

	return LC_SUCCESS;
}

int lc_load_config_stream(lc_config_t *config, FILE *fp)
{
	if(config == NULL || fp == NULL)
	{
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	return _read_file_to_config(config, fp);
}

int lc_dump_config(lc_config_t *config, const char *filepath)
{
	if(config == NULL)
	{
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);
		return LC_ERROR;
	}

	FILE *fp = NULL;

	if(filepath == NULL) 
	{
		if(config->filepath == NULL)
		{
			config->error_type = LC_ERR_FILE_NO;
			return LC_ERROR;
		}

		fp = _file_open(config->filepath, "w");
	}
	else 
	{
		fp = _file_open(filepath, "w");
	}

	if(fp == NULL)
	{
		config->error_type = LC_ERR_FILE_NO;
		return LC_ERROR;
	}

	if(_dump_config_to_file(config, fp) == LC_ERROR)
	{
		fclose(fp);
		return LC_ERROR;
	}

	fclose(fp);

	return LC_SUCCESS;
}

int lc_dump_config_stream(lc_config_t *config, FILE *fp)
{
	if(config == NULL || fp == NULL)
	{
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	return _dump_config_to_file(config, fp);
}

int lc_add_variable(lc_config_t *config, lc_config_variable_t *variable)
{
	if(config == NULL || variable == NULL)
	{
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	lc_config_variable_t *variable_copy = NULL;

	variable_copy = _create_variable_copy(variable);
	if(variable_copy == NULL)
	{
		warning(stderr, "[WARNING] %s: _create_variable_copy failed\n", __func__);
		config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	if(_add_list_element(config, variable_copy) == LC_ERROR)
	{
		free(variable);
		return LC_ERROR;
	}

	return LC_SUCCESS;
}

int lc_delete_variable(lc_config_t *config, const char *name)
{
	if(config == NULL || name == NULL)
	{
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}


	if(config->list == NULL)
	{
		config->error_type = LC_ERR_EMPTY;
		return LC_ERROR;
	}

	if((_delete_list_element(config, name)) == LC_ERROR)
		return LC_ERROR;

	return LC_SUCCESS;
}

lc_existence_t lc_is_variable_in_config(lc_config_t *config, const char *name)
{
	if(config == NULL || name == NULL)
	{
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_EF_ERROR;
	}

	if(config->list == NULL)
	{
		config->error_type = LC_ERR_EMPTY;
		return LC_EF_ERROR;
	}

	struct _lc_config_list *head = NULL;

	if((head = _find_list_element(config, name)) == NULL)
		return LC_EF_NOT_EXISTS;

	return LC_EF_EXISTS;
}

int lc_set_variable(lc_config_t *config, const char *name, const char *new_value)
{
	if(config == NULL || name == NULL || new_value == NULL)
	{
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	if(config->list == NULL)
	{
		config->error_type = LC_ERR_EMPTY;
		return LC_ERROR;
	}

	if(_rewrite_list_element_value(config, name, new_value) != LC_SUCCESS)
		return LC_ERROR;

	return LC_SUCCESS;
}

lc_config_variable_t* lc_get_variable(lc_config_t *config, const char *name)
{
	if(config == NULL || name == NULL)
	{
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return NULL;
	}

	if(config->list == NULL)
	{
		config->error_type = LC_ERR_EMPTY;
		return NULL;
	}

	struct _lc_config_list *head = NULL;

	if((head = _find_list_element(config, name)) == NULL)
		return NULL;

	return head->variable;
}

int lc_replace_variable(lc_config_t *config, const char *name, lc_config_variable_t *variable)
{
	if(config == NULL || name == NULL || variable == NULL)
	{
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	if(config->list == NULL)
	{
		config->error_type = LC_ERR_EMPTY;
		return LC_ERROR;
	}

	struct _lc_config_list *head = NULL;

	if((head = _find_list_element(config, name)) == NULL)
		return LC_ERROR;

	_replace_variable_in_list(head, variable);

	return LC_SUCCESS;
}

void lc_print_config(const lc_config_t *config)
{
	if(config == NULL)
	{
		warning(stderr, "[WARNING] %s: argument is null\n", __func__);
		return;
	}

	if(config->list == NULL)
		return;

	_print_list(config->list);
}

int lc_print_error(const lc_config_t *config)
{
	// TODO: make get_error_str()
	if(config == NULL)
	{
		warning(stderr, "[WARNING] %s: argument is null\n", __func__);
		return LC_ERROR;
	}

	const char * const error_msg[6] = {
		"LC_ERR_NONE",
		"LC_ERR_EMPTY",
		"LC_ERR_FILE_NO",
		"LC_ERR_MEMORY_NO",
		"LC_ERR_WRITE_NO",
		"LC_ERR_NOT_EXISTS"
	};

	fprintf(stderr, "[CONFIG] error_type = %s\n", error_msg[config->error_type]);
	return LC_SUCCESS;
}

void lc_clear_config(lc_config_t *config)
{
	if(config == NULL) {
		warning(stderr, "[WARNING] %s: argument is null\n", __func__);
		return;
	}

	_delete_list(config->list);

	free(config->filepath);
	free(config->delim);

	config->list_size = 0;
	config->error_type = LC_ERR_NONE;
	config->filepath = NULL;
}

char* lc_get_delim(lc_config_t *config)
{
	if(config == NULL) {
		warning(stderr, "[WARNING] %s: argument is null\n", __func__);
		return NULL;
	}

	return _duplicate_string(config->delim);
}

int lc_set_delim(lc_config_t *config, const char *delim)
{
	if(config == NULL || delim == NULL) {
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	free(config->delim);

	if((config->delim = _duplicate_string(delim)) == NULL)
	{
		config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	return LC_SUCCESS;
}

size_t lc_get_size(const lc_config_t *config)
{
	if(config == NULL) {
		warning(stderr, "[WARNING] %s: argument is null\n", __func__);
		return (size_t)0;
	}

	return config->list_size;
}

/*int lc_is_empty(const lc_config_t *config)
{
	if(config == NULL)
	{
		warning(stderr, "[WARNING] %s: argument is null\n", __func__);
		return LC_ERROR;
	}

	return config->list_size == 0;
}*/

char* lc_get_path(const lc_config_t *config)
{
	if(config == NULL)
	{
		warning(stderr, "[WARNING] %s: argument is null\n", __func__);
		return NULL;
	}

	return _duplicate_string(config->filepath);
}

int lc_set_path(lc_config_t *config, const char *filepath)
{
	if(config == NULL || filepath == NULL)
	{
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	free(config->filepath);

	if((config->filepath = _duplicate_string(filepath)) == NULL)
	{
		config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	return LC_SUCCESS;
}

void lc_clear_path(lc_config_t *config)
{
	if(config == NULL) {
		warning(stderr, "[WARNING] %s: argument is null\n", __func__);
		return;
	}

	free(config->filepath);
	config->filepath = NULL;
}

lc_config_variable_t* lc_create_variable(const char *name, const char *value)
{
	if(name == NULL || value == NULL) {
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);
		return NULL;
	}

	return _make_config_variable(name, value);
}

lc_config_variable_t* lc_create_variable_copy(lc_config_variable_t *variable)
{
	if(variable == NULL) {
		warning(stderr, "[WARNING] %s: argument is null\n", __func__);
		return NULL;
	}

	return _create_variable_copy(variable);
}

void lc_destroy_variable(lc_config_variable_t *variable)
{
	if(variable == NULL) {
		warning(stderr, "[WARNING] %s: argument is null\n", __func__);
		return;
	}

	_free_config_variable(variable);
}

char* lc_get_variable_name(lc_config_variable_t *variable)
{
	if(variable == NULL) {
		warning(stderr, "[WARNING] %s: argument is null\n", __func__);
		return NULL;
	}

	return _duplicate_string(variable->name);
}

char* lc_get_variable_value(lc_config_variable_t *variable)
{
	if(variable == NULL) {
		warning(stderr, "[WARNING] %s: argument is null\n", __func__);
		return NULL;
	}

	return _duplicate_string(variable->value);
}

int lc_set_variable_name(lc_config_variable_t *variable, const char *name)
{
	if(variable == NULL || name == NULL) {
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);
		return LC_ERROR;
	}

	free(variable->name);
	variable->name = _duplicate_string(name);

	return LC_SUCCESS;
}

int lc_set_variable_value(lc_config_variable_t *variable, const char *value)
{
	if(variable == NULL || value == NULL) {
		warning(stderr, "[WARNING] %s: arguments is null\n", __func__);
		return LC_ERROR;
	}

	free(variable->value);
	variable->value = _duplicate_string(value);

	return LC_SUCCESS;
}
