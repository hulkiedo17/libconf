#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "libconf.h"

// misc functions

static int _file_exists(const char *path)
{
	assert(path != NULL);

	struct stat buffer;

	return stat(path, &buffer);
}

static FILE* _file_open(const char *filename, const char *mode)
{
	assert(filename != NULL);
	assert(mode != NULL);

	FILE *fp = fopen(filename, mode);
	
	if(fp == NULL)
		return NULL;

	return fp;
}

static char* _duplicate_string(const char *string)
{
	assert(string != NULL);

	size_t length = strlen(string) + 1;

	char *duplicate = calloc(length, sizeof(char));
	if(duplicate == NULL)
		return NULL;

	memcpy(duplicate, string, length);

	return duplicate;
}

static char* _read_line_from_file(FILE *fp)
{
	assert(fp != NULL);

	size_t line_length = LINE_SIZE;

	char *line_buffer = calloc(line_length, sizeof(char));
	if(line_buffer == NULL)
	{
		perror("calloc");
		return NULL;
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
			line_length += LINE_SIZE;
			line_buffer = realloc(line_buffer, line_length);
			if(line_buffer == NULL)
			{
				perror("realloc");
				return NULL;
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

// config list api

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
	if(new_variable == NULL)
		return NULL;

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

static struct _lc_config_list* _create_list_element(lc_config_variable_t *variable)
{
	assert(variable != NULL);

	struct _lc_config_list *element = NULL;

	element = malloc(sizeof(struct _lc_config_list));
	if(element == NULL)
		return NULL;

	element->variable = variable;
	element->next = NULL;

	return element;
}

static lc_config_variable_t* _convert_line_to_variable(const char *line)
{
	assert(line != NULL);

	char *dup_line = _duplicate_string(line);
	if(dup_line == NULL)
		return NULL;

	char *name = strtok(dup_line, "=");
	char *value = strtok(NULL, "=");

	lc_config_variable_t *variable = _make_config_variable(name, value);

	free(dup_line);
	return variable;
}

static char* _convert_variable_to_line(lc_config_variable_t *variable)
{
	assert(variable != NULL);

	// +3 because we also add: '=', '\n', '\0'
	size_t length = strlen(variable->name) + strlen(variable->value) + 3;

	char *line = calloc(length, sizeof(char));
	if(line == NULL)
		return NULL;

	if(snprintf(line, length, "%s=%s\n", variable->name, variable->value) != ((int)length - 1))
	{
		free(line);
		return NULL;
	}

	return line;
}

static void _print_list(struct _lc_config_list *list)
{
	assert(list != NULL);

	struct _lc_config_list *head = list;

	//printf("%s:\n", config_name);
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
		config->list_count++;
		return LC_SUCCESS;
	}

	struct _lc_config_list *temp = config->list;
	while(temp->next != NULL)
		temp = temp->next;

	temp->next = element;

	config->error_type = LC_ERR_NONE;
	config->list_count++;
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
		config->list_count--;
		return LC_SUCCESS;
	}

	temp = element;
	prev_element->next = element->next;

	_free_list_element(temp);
	
	config->error_type = LC_ERR_NONE;
	config->list_count--;
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

// main io functions

static int _read_file_to_config(lc_config_t *config, FILE *fp)
{
	assert(config != NULL);
	assert(fp != NULL);

	char *line = NULL;
	lc_config_variable_t * variable = NULL;

	while((line = _read_line_from_file(fp)) != NULL)
	{
		if((variable = _convert_line_to_variable(line)) == NULL)
		{
			free(line);
			config->error_type = LC_ERR_MEMORY_NO;
			return LC_ERROR;
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
		if((line = _convert_variable_to_line(head->variable)) == NULL)
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

// main library api

void lc_init_config(lc_config_t *config)
{
	assert(config != NULL);

	config->list = NULL;
	config->list_count = 0;
	config->error_type = LC_ERR_NONE;
	//config->file = file; // if it's null, set null.
	// maybe status (-_-)
}

int lc_load_config(lc_config_t *config, const char *filename)
{
	assert(config != NULL);
	assert(filename != NULL);

	if(_file_exists(filename) != 0)
	{
		config->error_type = LC_ERR_FILE_NO;
		return LC_ERROR;
	}

	FILE *fp = _file_open(filename, "r");
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

int lc_dump_config(lc_config_t *config, const char *filename)
{
	assert(config != NULL);
	assert(filename != NULL);

	FILE *fp = _file_open(filename, "w");
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

int lc_add_variable(lc_config_t *config, const char *name, const char *value)
{
	assert(config != NULL);
	assert(name != NULL);
	assert(value != NULL);

	lc_config_variable_t * variable = _make_config_variable(name, value);
	if(variable == NULL)
	{
		config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	if(_add_list_element(config, variable) == LC_ERROR)
	{
		free(variable);
		return LC_ERROR;
	}

	return LC_SUCCESS;
}

int lc_delete_variable(lc_config_t *config, const char *name)
{
	assert(config != NULL);
	assert(name != NULL);

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
	assert(config != NULL);
	assert(name != NULL);

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
	assert(config != NULL);
	assert(name != NULL);
	assert(new_value != NULL);

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
	assert(config != NULL);
	assert(name != NULL);

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

void lc_print_config(const lc_config_t *config)
{
	_print_list(config->list);
}

void lc_clear_config(lc_config_t *config)
{
	if(config == NULL)
		return;

	_delete_list(config->list);

	config->list_count = 0;
	config->error_type = LC_ERR_NONE;
}

