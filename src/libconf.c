#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "libconf.h"

static const char * const error_msg[6] = {
	"LC_ERR_NONE",
	"LC_ERR_EMPTY",
	"LC_ERR_FILE_NO",
	"LC_ERR_MEMORY_NO",
	"LC_ERR_WRITE_NO",
	"LC_ERR_NOT_EXISTS"
};

#if defined DEBUG

static void warning(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

static void p_error(const char *fmt, ...)
{
	va_list ap;
	
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(EXIT_FAILURE);
}

#else

#define warning(...)
#define p_error(...)

#endif

static FILE* file_open(const char *file, const char *mode)
{
	FILE *fp = NULL;

	if(file == NULL || mode == NULL)
		return NULL;

	fp = fopen(file, mode);
	if(fp == NULL)
		p_error("[ERROR] %s: allocation failed\n", __func__);

	return fp;
}

// remove
static char* _read_line_from_file(FILE *fp)
{
	int c;
	size_t pos = 0;
	size_t len = (size_t)0;
	char *buf = NULL;

	if(fp == NULL)
		return NULL;

	len = LINE_SIZE;

	buf = calloc(len, sizeof(char));
	if(buf == NULL)
		p_error("[ERROR] %s: allocation failed\n", __func__);

	while(1)
	{
		c = fgetc(fp);

		if(c == EOF || c == '\n')
		{
			if(pos == 0 && (c == EOF || c == '\n'))
			{
				free(buf);
				return NULL;
			}

			buf[pos] = '\0';
			return buf;
		}

		buf[pos++] = c;

		if(pos >= len)
		{
			// TODO: make temporary pointer
			len += LINE_SIZE;
			buf = realloc(buf, len);
			if(buf == NULL)
				p_error("[ERROR] %s: allocation failed\n", __func__);
		}
	}

	return NULL;
}

static int _write_line_to_file(FILE *fp, const char *line)
{
	if(fp == NULL || line == NULL)
		return LC_ERROR;

	if(fwrite(line, 1, strlen(line), fp) != strlen(line))
		return LC_ERROR;

	return LC_SUCCESS;
}

// functions  for config list 

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
	lc_config_variable_t *variable = NULL;

	if(name == NULL || value == NULL)
		return NULL;

	variable = malloc(sizeof(lc_config_variable_t));
	if(variable == NULL)
		p_error("[ERROR] %s: allocation failed\n", __func__);

	variable->name = strdup(name);
	if(variable->name == NULL)
	{
		free(variable);
		return NULL;
	}

	variable->value = strdup(value);
	if(variable->value == NULL)
	{
		free(variable->name);
		free(variable);
		return NULL;
	}

	return variable;
}

static lc_config_variable_t* _create_variable_copy(lc_config_variable_t *variable)
{
	if(variable == NULL)
		return NULL;

	if(variable->name == NULL)
		return NULL;

	if(variable->value == NULL)
		return NULL;

	return _make_config_variable(variable->name, variable->value);
}

static struct _lc_config_list* _create_list_element(lc_config_variable_t *variable)
{
	struct _lc_config_list *element = NULL;

	if(variable == NULL)
		return NULL;

	element = malloc(sizeof(struct _lc_config_list));
	if(element == NULL)
		p_error("[ERROR] %s: allocation failed\n", __func__);

	element->variable = variable;
	element->next = NULL;

	return element;
}

static lc_config_variable_t* _convert_line_to_variable(const char *line, const char *delim)
{
	char *d_line = NULL;
	char *name = NULL;
	char *value = NULL;
	lc_config_variable_t *variable = NULL;

	if(line == NULL || delim == NULL)
		return NULL;

	if(strstr(line, delim) == NULL)
	{
		free(d_line);
		warning("[WARNING] %s: cannot find \"%s\" delimiter in line \"%s\"\n", __func__, delim, line);
		return NULL;
	}

	d_line = strdup(line);
	if(d_line == NULL)
		return NULL;

	name = strtok(d_line, delim);
	value = strtok(NULL, delim);

	variable = _make_config_variable(name, value);

	free(d_line);
	return variable;
}

static char* _convert_variable_to_line(lc_config_variable_t *variable, const char *delim)
{
	size_t len = (size_t)0;
	char *line = NULL;

	if(variable == NULL || delim == NULL)
		return NULL;

	// +2 because we also add: '\n', '\0'
	len = strlen(variable->name) + strlen(variable->value) + strlen(delim) + 2;

	line = calloc(len, sizeof(char));
	if(line == NULL)
		p_error("[ERROR] %s: allocation failed\n", __func__);

	if(snprintf(line, len, "%s%s%s\n", variable->name, delim, variable->value) != ((int)len - 1))
	{
		free(line);
		warning("[WARNING] %s: snprintf() failed\n", __func__);
		return NULL;
	}

	return line;
}

static void _print_list(struct _lc_config_list *list)
{
	struct _lc_config_list *head = list;

	if(list == NULL)
		return;

	while(head != NULL)
	{
		printf("%s=%s\n", head->variable->name, head->variable->value);
		head = head->next;
	}
	printf("\n");
}

static int _add_list_element(lc_config_t *config, lc_config_variable_t *variable)
{
	struct _lc_config_list *element = NULL;
	struct _lc_config_list *temp = NULL;

	if(config == NULL || variable == NULL)
		return LC_ERROR;

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

	temp = config->list;
	while(temp->next != NULL)
		temp = temp->next;

	temp->next = element;

	config->error_type = LC_ERR_NONE;
	config->list_size++;
	return LC_SUCCESS;
}

static struct _lc_config_list* _find_list_element(lc_config_t *config, const char *name)
{
	struct _lc_config_list *head = config->list;

	if(config == NULL || name == NULL)
		return NULL;

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
	struct _lc_config_list *prev = NULL;
	struct _lc_config_list *head = config->list;

	if(config == NULL || name == NULL)
		return NULL;

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
	struct _lc_config_list *temp = NULL;
	struct _lc_config_list *element = NULL;
	struct _lc_config_list *prev = NULL;

	if(config == NULL || name == NULL)
		return LC_ERROR;

	if(config->list == NULL)
	{
		config->error_type = LC_ERR_EMPTY;
		return LC_ERROR;
	}

	if((element = _find_list_element(config, name)) == NULL)
	{
		config->error_type = LC_ERR_NOT_EXISTS;
		return LC_ERROR;
	}

	if((prev = _find_prev_list_element(config, name)) == NULL)
	{
		config->list = config->list->next;
		_free_list_element(element);

		config->error_type = LC_ERR_NONE;
		config->list_size--;
		return LC_SUCCESS;
	}

	temp = element;
	prev->next = element->next;

	_free_list_element(temp);

	config->error_type = LC_ERR_NONE;
	config->list_size--;
	return LC_SUCCESS;
}

static void _delete_list(struct _lc_config_list *list)
{
	struct _lc_config_list *head = list;
	struct _lc_config_list *temp = NULL;

	if(list == NULL)
		return;

	while(head != NULL)
	{
		temp = head;
		head = head->next;

		_free_list_element(temp);
	}
}

static int _rewrite_list_element_value(lc_config_t *config, const char *name, const char *new_value)
{
	struct _lc_config_list *element = NULL;

	if(config == NULL || name == NULL || new_value == NULL)
		return LC_ERROR;

	if((element = _find_list_element(config, name)) == NULL)
		return LC_ERROR;

	free(element->variable->value);
	element->variable->value = strdup(new_value);

	config->error_type = LC_ERR_NONE;
	return LC_SUCCESS;
}

static int _replace_variable_in_list(struct _lc_config_list *list, lc_config_variable_t *variable)
{
	if(list == NULL || variable == NULL)
		return LC_ERROR;

	_free_config_variable(list->variable);
	list->variable = variable;

	return LC_SUCCESS;
}

// io functions

static int _read_file_to_config(lc_config_t *config, FILE *fp)
{
	char *line = NULL;
	lc_config_variable_t * variable = NULL;

	if(config == NULL || fp == NULL)
		return LC_ERROR;

	// TODO: delete read_line
	while((line = _read_line_from_file(fp)) != NULL)
	{
		if((variable = _convert_line_to_variable(line, config->delim)) == NULL)
		{
			free(line);
			config->error_type = LC_ERR_MEMORY_NO;
			// if it can't convert line to variable, it's just skip this line
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
	char *line = NULL;
	struct _lc_config_list *head = config->list;

	if(config == NULL || fp == NULL)
		return LC_ERROR;

	if(config->list == NULL)
	{
		config->error_type = LC_ERR_EMPTY;
		return LC_ERROR;
	}

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

int lc_init_config(lc_config_t *config, const char *path, const char *delim)
{
	if(config == NULL || delim == NULL)
	{
		warning("[WARNING] %s: arguments is null\n", __func__);
		return LC_ERROR;
	}

	config->list = NULL;
	config->list_size = 0;
	config->delim = NULL;

	if(path != NULL)
	{
		if((config->path = strdup(path)) == NULL)
		{
			config->error_type = LC_ERR_MEMORY_NO;
			return LC_ERROR;
		}
	}
	else
	{
		config->path = NULL;
	}

	if((config->delim = strdup(delim)) == NULL)
	{
		free(config->path);
		config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	config->error_type = LC_ERR_NONE;
	return LC_SUCCESS;
}

int lc_load_config(lc_config_t *config, const char *path)
{
	FILE *fp = NULL;

	if(config == NULL)
	{
		warning("[WARNING] %s: argument is null\n", __func__);
		return LC_ERROR;
	}

	if(path == NULL) 
	{
		if(config->path == NULL)
		{
			config->error_type = LC_ERR_FILE_NO;
			return LC_ERROR;
		}

		fp = file_open(config->path, "r");
	}
	else 
	{
		fp = file_open(path, "r");
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
		warning("[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	return _read_file_to_config(config, fp);
}

int lc_dump_config(lc_config_t *config, const char *path)
{
	FILE *fp = NULL;

	if(config == NULL)
	{
		warning("[WARNING] %s: arguments is null\n", __func__);
		return LC_ERROR;
	}

	if(path == NULL) 
	{
		if(config->path == NULL)
		{
			config->error_type = LC_ERR_FILE_NO;
			return LC_ERROR;
		}

		fp = file_open(config->path, "w");
	}
	else 
	{
		fp = file_open(path, "w");
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
		warning("[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	return _dump_config_to_file(config, fp);
}

int lc_add_variable(lc_config_t *config, lc_config_variable_t *variable)
{
	lc_config_variable_t *variable_copy = NULL;

	if(config == NULL || variable == NULL)
	{
		warning("[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	variable_copy = _create_variable_copy(variable);
	if(variable_copy == NULL)
	{
		warning("[WARNING] %s: _create_variable_copy failed\n", __func__);
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
		warning("[WARNING] %s: arguments is null\n", __func__);

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
	struct _lc_config_list *head = NULL;

	if(config == NULL || name == NULL)
	{
		warning("[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_EF_ERROR;
	}

	if(config->list == NULL)
	{
		config->error_type = LC_ERR_EMPTY;
		return LC_EF_ERROR;
	}

	if((head = _find_list_element(config, name)) == NULL)
		return LC_EF_NOT_EXISTS;

	return LC_EF_EXISTS;
}

int lc_set_variable(lc_config_t *config, const char *name, const char *new_value)
{
	if(config == NULL || name == NULL || new_value == NULL)
	{
		warning("[WARNING] %s: arguments is null\n", __func__);

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
	struct _lc_config_list *head = NULL;

	if(config == NULL || name == NULL)
	{
		warning("[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return NULL;
	}

	if(config->list == NULL)
	{
		config->error_type = LC_ERR_EMPTY;
		return NULL;
	}

	if((head = _find_list_element(config, name)) == NULL)
		return NULL;

	return _create_variable_copy(head->variable);
}

int lc_replace_variable(lc_config_t *config, const char *name, lc_config_variable_t *variable)
{
	struct _lc_config_list *head = NULL;
	lc_config_variable_t *variable_copy = NULL;

	if(config == NULL || name == NULL || variable == NULL)
	{
		warning("[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	if(config->list == NULL)
	{
		config->error_type = LC_ERR_EMPTY;
		return LC_ERROR;
	}

	if((head = _find_list_element(config, name)) == NULL)
		return LC_ERROR;

	variable_copy = _create_variable_copy(variable);
	if(variable_copy == NULL)
	{
		warning("[WARNING] %s: _create_variable_copy failed\n", __func__);
		config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	_replace_variable_in_list(head, variable_copy);

	return LC_SUCCESS;
}

void lc_print_config(const lc_config_t *config)
{
	if(config == NULL)
	{
		warning("[WARNING] %s: argument is null\n", __func__);
		return;
	}

	if(config->list == NULL)
		return;

	_print_list(config->list);
}

char* lc_get_error(const lc_config_t *config)
{
	if(config == NULL)
	{
		warning("[WARNING] %s: argument is null\n", __func__);
		return NULL;
	}

	if(config->error_type < 0 || config->error_type > 5)
	{
		warning("[ERROR] %s: invalid error index\n", __func__);
		return NULL;
	}

	return strdup(error_msg[config->error_type]);
}

void lc_clear_config(lc_config_t *config)
{
	if(config == NULL) {
		warning("[WARNING] %s: argument is null\n", __func__);
		return;
	}

	_delete_list(config->list);

	free(config->path);
	free(config->delim);

	config->list_size = 0;
	config->error_type = LC_ERR_NONE;
	config->path = NULL;
}

char* lc_get_delim(lc_config_t *config)
{
	if(config == NULL) {
		warning("[WARNING] %s: argument is null\n", __func__);
		return NULL;
	}

	return strdup(config->delim);
}

int lc_set_delim(lc_config_t *config, const char *delim)
{
	if(config == NULL || delim == NULL) {
		warning("[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	free(config->delim);

	if((config->delim = strdup(delim)) == NULL)
	{
		config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	return LC_SUCCESS;
}

size_t lc_get_size(const lc_config_t *config)
{
	if(config == NULL) {
		warning("[WARNING] %s: argument is null\n", __func__);
		return (size_t)0;
	}

	return config->list_size;
}

char* lc_get_path(const lc_config_t *config)
{
	if(config == NULL)
	{
		warning("[WARNING] %s: argument is null\n", __func__);
		return NULL;
	}

	return strdup(config->path);
}

int lc_set_path(lc_config_t *config, const char *path)
{
	if(config == NULL || path == NULL)
	{
		warning("[WARNING] %s: arguments is null\n", __func__);

		if(config != NULL)
			config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	free(config->path);

	if((config->path = strdup(path)) == NULL)
	{
		config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	return LC_SUCCESS;
}

void lc_clear_path(lc_config_t *config)
{
	if(config == NULL) {
		warning("[WARNING] %s: argument is null\n", __func__);
		return;
	}

	free(config->path);
	config->path = NULL;
}

lc_config_variable_t* lc_create_variable(const char *name, const char *value)
{
	if(name == NULL || value == NULL) {
		warning("[WARNING] %s: arguments is null\n", __func__);
		return NULL;
	}

	return _make_config_variable(name, value);
}

lc_config_variable_t* lc_create_variable_copy(lc_config_variable_t *variable)
{
	if(variable == NULL) {
		warning("[WARNING] %s: argument is null\n", __func__);
		return NULL;
	}

	return _create_variable_copy(variable);
}

void lc_destroy_variable(lc_config_variable_t *variable)
{
	if(variable == NULL) {
		warning("[WARNING] %s: argument is null\n", __func__);
		return;
	}

	_free_config_variable(variable);
}

char* lc_get_variable_name(lc_config_variable_t *variable)
{
	if(variable == NULL) {
		warning("[WARNING] %s: argument is null\n", __func__);
		return NULL;
	}

	return strdup(variable->name);
}

char* lc_get_variable_value(lc_config_variable_t *variable)
{
	if(variable == NULL) {
		warning("[WARNING] %s: argument is null\n", __func__);
		return NULL;
	}

	return strdup(variable->value);
}

int lc_set_variable_name(lc_config_variable_t *variable, const char *name)
{
	if(variable == NULL || name == NULL) {
		warning("[WARNING] %s: arguments is null\n", __func__);
		return LC_ERROR;
	}

	free(variable->name);
	variable->name = strdup(name);

	return LC_SUCCESS;
}

int lc_set_variable_value(lc_config_variable_t *variable, const char *value)
{
	if(variable == NULL || value == NULL) {
		warning("[WARNING] %s: arguments is null\n", __func__);
		return LC_ERROR;
	}

	free(variable->value);
	variable->value = strdup(value);

	return LC_SUCCESS;
}
