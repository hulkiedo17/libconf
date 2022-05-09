#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "libconf.h"

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

static char* _get_name_from_variable(const char *line)
{
	assert(line != NULL);

	char *dup_line = _duplicate_string(line);
	if(dup_line == NULL)
		return NULL;

	char *variable_name = strtok(dup_line, "=");

	char *name = _duplicate_string(variable_name);
	if(name == NULL)
	{
		free(dup_line);
		return NULL;
	}

	free(dup_line);
	return name;
}

static char* _get_value_from_variable(const char *line)
{
	assert(line != NULL);

	char *dup_line = _duplicate_string(line);
	if(dup_line == NULL)
		return NULL;

	strtok(dup_line, "=");

	char *variable_value = strtok(NULL, "=");

	char *value = _duplicate_string(variable_value);
	if(value == NULL)
	{
		free(dup_line);
		return NULL;
	}

	free(dup_line);
	return value;
}

/*static struct _lc_config_variable* _make_variable2(const char *name, const char *value)
{
	struct _lc_config_variable *new_variable = NULL;

	new_variable = malloc(sizeof(struct _lc_config_variable));
	if(new_variable == NULL)
	{
		perror("malloc");
		return NULL;
	}

	new_variable->name = _duplicate_string(name);
	new_variable->value = _duplicate_string(value);

	return new_variable;
}*/

static char* _make_variable(const char *name, const char *value)
{
	assert(name != NULL);
	assert(value != NULL);

	size_t length = strlen(name) + strlen(value) + 2; // '=' and '\0'

	char *variable = calloc(length, sizeof(char));
	if(variable == NULL)
		return NULL;

	memcpy(variable, name, strlen(name));
	memcpy(variable + strlen(name), "=", 1);
	memcpy(variable + strlen(name) + 1, value, strlen(value));

	return variable;
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

	if(fwrite("\n", 1, 1, fp) != 1)
		return LC_ERROR;

	return LC_SUCCESS;
}

// config list api

static struct _lc_config_list* _create_list_element(const char *line)
{
	assert(line != NULL);

	struct _lc_config_list *element = NULL;

	element = malloc(sizeof(struct _lc_config_list));
	if(element == NULL)
		return NULL;

	element->line = _duplicate_string(line);
	if(element->line == NULL)
	{
		free(element);
		return NULL;
	}

	element->next = NULL;

	return element;
}

static int _add_list_element(lc_config_t *config, const char *line)
{
	assert(config != NULL);
	assert(line != NULL);

	struct _lc_config_list *element = NULL;

	element = _create_list_element(line);
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

static int _delete_list_element(lc_config_t *config, const char *line)
{
	assert(config != NULL);
	assert(line != NULL);

	struct _lc_config_list *prev = config->list;
	struct _lc_config_list *head = config->list;
	struct _lc_config_list *temp = NULL;

	if(strcmp(head->line, line) == 0)
	{
		temp = head;
		config->list = config->list->next;

		free(temp->line);
		free(temp);

		config->error_type = LC_ERR_NONE;
		config->list_count--;
		return LC_SUCCESS;
	}

	if(head->next == NULL)
	{
		config->error_type = LC_ERR_NOT_EXISTS;
		return LC_ERROR;
	}

	while(head != NULL)
	{
		if(strcmp(head->line, line) == 0)
		{
			temp = head;
			prev->next = head->next;

			free(temp->line);
			free(temp);

			config->error_type = LC_ERR_NONE;
			config->list_count--;
			return LC_SUCCESS;
		}

		prev = head;
		head = head->next;
	}

	config->error_type = LC_ERR_NOT_EXISTS;
	return LC_ERROR;
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

		free(temp->line);
		free(temp);
	}
}

static struct _lc_config_list* _find_list_element(lc_config_t *config, const char *name)
{
	assert(config != NULL);
	assert(name != NULL);

	char *variable_name = NULL;
	struct _lc_config_list *head = config->list;

	while(head != NULL)
	{
		if((variable_name = _get_name_from_variable(head->line)) == NULL)
		{
			config->error_type = LC_ERR_MEMORY_NO;
			return NULL;
		}

		if(strcmp(variable_name, name) == 0)
		{
			free(variable_name);
			config->error_type = LC_ERR_NONE;
			return head;
		}

		free(variable_name);
		head = head->next;
	}

	config->error_type = LC_ERR_NOT_EXISTS;
	return NULL;
}

static int _rewrite_list_element(lc_config_t *config, const char *name, const char *new_line)
{
	assert(config != NULL);
	assert(name != NULL);
	assert(new_line != NULL);

	char *temp_line = NULL;
	struct _lc_config_list *element = NULL;

	if((element = _find_list_element(config, name)) == NULL)
		return LC_ERROR;

	if((temp_line = _duplicate_string(new_line)) == NULL)
	{
		config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	free(element->line);
	element->line = temp_line;

	config->error_type = LC_ERR_NONE;
	return LC_SUCCESS;
}

static void _print_list(struct _lc_config_list *list)
{
	assert(list != NULL);

	struct _lc_config_list *head = list;

	while(head != NULL)
	{
		printf("%s\n", head->line);
		head = head->next;
	}
	printf("\n");
}

// main io functions

static int _read_file_to_config(lc_config_t *config, FILE *fp)
{
	assert(config != NULL);
	assert(fp != NULL);

	char *line = NULL;

	while((line = _read_line_from_file(fp)) != NULL)
	{
		if(_add_list_element(config, line) == LC_ERROR)
		{
			free(line);
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

	struct _lc_config_list *head = config->list;

	while(head != NULL)
	{
		if(_write_line_to_file(fp, head->line) == LC_ERROR)
		{
			config->error_type = LC_ERR_WRITE_NO;
			return LC_ERROR;
		}

		head = head->next;
	}

	return LC_SUCCESS;
}

// main library api

void lc_init_config(lc_config_t *config)
{
	assert(config != NULL);

	config->list = NULL;
	config->list_count = 0;
	config->error_type = LC_ERR_NONE;
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

	config->error_type = LC_ERR_NONE;
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

	config->error_type = LC_ERR_NONE;
	return LC_SUCCESS;
}

int lc_add_variable(lc_config_t *config, const char *name, const char *value)
{
	assert(config != NULL);
	assert(name != NULL);
	assert(value != NULL);

	char *variable = _make_variable(name, value);

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

	free(variable);
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

	struct _lc_config_list *head = NULL;

	if((head = _find_list_element(config, name)) == NULL)
		return LC_ERROR;

	if((_delete_list_element(config, head->line)) == LC_ERROR)
		return LC_ERROR;

	config->error_type = LC_ERR_NONE;
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

	config->error_type = LC_ERR_NONE;
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

	char *new_variable = _make_variable(name, new_value);

	if(new_variable == NULL)
	{
		config->error_type = LC_ERR_MEMORY_NO;
		return LC_ERROR;
	}

	if(_rewrite_list_element(config, name, new_variable) != LC_SUCCESS)
	{
		free(new_variable);
		return LC_ERROR;
	}

	free(new_variable);
	return LC_SUCCESS;
}

char* lc_get_value(lc_config_t *config, const char *name)
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

	char *value = NULL;

	if((value = _get_value_from_variable(head->line)) == NULL)
	{
		config->error_type = LC_ERR_MEMORY_NO;
		return NULL;
	}

	return value;
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

