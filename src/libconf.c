#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include "libconf.h"

#define LINE_SIZE 128

/*static void warning(const char* fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}*/

static size_t file_exists(const char* path) {
	struct stat buffer;

	if(stat(path, &buffer) == 0) return 1;
	
	return 0;
}

static FILE* open_file(const char* filename, const char* mode) {
	assert(filename != NULL);
	assert(mode != NULL);

	FILE* fp = fopen(filename, mode);
	if(fp == NULL) {
		//warning("cannot open file\n");
		return NULL;
	}

	return fp;
}

static char* dup_string(const char* string) {
	assert(string != NULL);

	size_t length = strlen(string) + 1;
	char* duplicate = malloc(length * sizeof(char));
	if(duplicate == NULL) {
		//warning("cannot malloc string\n");
		return NULL;
	}

	strncpy(duplicate, string, length);
	return duplicate;
}

static size_t write_string_to_file(FILE* fp, const char* string) {
	assert(fp != NULL);
	assert(string != NULL);

	size_t index = 0, length = strlen(string);
	for(; index < length; index++) {
		fputc(string[index], fp);
	}
	fputc('\n', fp);

	return index;
}

static char* read_line_from_file(FILE* fp) {
	assert(fp != NULL);

	int c;
	size_t position = 0, line_length = LINE_SIZE;
	char* line_buffer = calloc(line_length, sizeof(char));
	if(line_buffer == NULL) {
		//warning("cannot calloc buffer\n");
		return NULL;
	}

	while(1) {
		c = fgetc(fp);

		if(c == EOF || c == '\n') {
			if(position == 0 && (c == EOF || c == '\n')) {
				free(line_buffer);
				return NULL;
			}
			line_buffer[position] = '\0';
			return line_buffer;
		} else {
			line_buffer[position] = c;
		}

		position++;

		if(position >= line_length) {
			line_length += LINE_SIZE;
			line_buffer = realloc(line_buffer, line_length);
			if(line_buffer == NULL) {
				//warning("cannot realloc buffer\n");
				return NULL;
			}
		}
	}

	return NULL;
}

static ssize_t find_line_number(const char* file, const char* variable) {
	assert(file != NULL);
	assert(variable != NULL);

	FILE* fp = open_file(file, "r");
	if(!fp) return -1;

	size_t line_count = 1;
	char* line_buffer = NULL;
	while((line_buffer = read_line_from_file(fp)) != NULL) {
		char* value = strtok(line_buffer, "=");

		if(strcmp(value, variable) == 0) {
			free(line_buffer);
			fclose(fp);
			return line_count;
		}
		free(line_buffer);
		line_buffer = NULL;
		line_count++;
	}

	fclose(fp);
	return -1;
}

static char* make_temp_file_path(const char* filepath) {
	assert(filepath != NULL);

	char* temp_file = malloc(sizeof(char) * (strlen(filepath) + 5));
	if(temp_file == NULL) {
		//warning("cannot malloc temp file path\n");
		return NULL;
	}
	
	strcpy(temp_file, filepath);
	strcat(temp_file, ".tmp");

	return temp_file;
}

static ssize_t write_file_without_line(const char* file, size_t line_number) {
	assert(file != NULL);

	FILE* fp = open_file(file, "r");
	if(!fp) return -1;

	char* temp_file = make_temp_file_path(file);
	if(!temp_file) return -1;

	FILE* temp_fp = open_file(temp_file, "a");
	if(!temp_fp) return -1;

	size_t line_count = 1;
	char* line_buffer = NULL;
	while((line_buffer = read_line_from_file(fp)) != NULL) {
		if(line_count != line_number) {
			write_string_to_file(temp_fp, line_buffer);
		}
		free(line_buffer);
		line_buffer = NULL;
		line_count++;
	}

	fclose(fp);
	fclose(temp_fp);

	remove(file);
	rename(temp_file, file);

	free(temp_file);
	return 0;
}

static ssize_t rewrite_file_with_new_line(const char* file, size_t line_number, const char* new_line) {
	assert(file != NULL);
	assert(new_line != NULL);

	FILE* fp = open_file(file, "r");
	if(!fp) return -1;

	char* temp_file = make_temp_file_path(file);
	if(!temp_file) return -1;

	FILE* temp_fp = open_file(temp_file, "a");
	if(!temp_fp) return -1;

	size_t line_count = 1;
	char* line_buffer = NULL;
	while((line_buffer = read_line_from_file(fp)) != NULL) {
		if(line_count != line_number) {
			write_string_to_file(temp_fp, line_buffer);
		} else {
			write_string_to_file(temp_fp, new_line);
		}

		free(line_buffer);
		line_buffer = NULL;
		line_count++;
	}

	fclose(fp);
	fclose(temp_fp);

	remove(file);
	rename(temp_file, file);

	free(temp_file);
	return 0;
}

static ssize_t find_amount_of_tokens_in_value(const char* value, const char* delim) {
	char* duplicate_value = dup_string(value);
	if(duplicate_value == NULL) return -1;

	size_t amount_of_tokens = 0;
	char* token = strtok(duplicate_value, delim);
	while(token != NULL) {
		amount_of_tokens++;
		token = strtok(NULL, delim);
	}

	free(duplicate_value);
	return amount_of_tokens;
}

static char* make_path_to_file(const char* path, const char* file) {
	assert(path != NULL);
	assert(file != NULL);

	char* file_path = malloc(sizeof(char) * (strlen(path) + strlen(file) + 2));
	if(file_path == NULL) {
		//warning("cannot malloc path to file\n");
		return NULL;
	}

	strcpy(file_path, path);
	strcat(file_path, "/");
	strcat(file_path, file);
	return file_path;
}

static size_t check_on_path(const char* file) {
	if(strchr(file, '/') != NULL) return 0;

	return 1;
}

static char* make_variable(const char* name, const char* value) {
	assert(name != NULL);
	assert(value != NULL);

	size_t length = strlen(name) + strlen(value) + 3;	// +3 because =, \n, \0
	char* variable = malloc(length * sizeof(char));
	if(variable == NULL) {
		//warning("cannot malloc variable\n");
		return NULL;
	}

	strcpy(variable, name);
	strcat(variable, "=");
	strcat(variable, value);

	return variable;
}

static lc_split_t* init_split(const char* name) {
	lc_split_t* tokens = malloc(sizeof(char) * sizeof(lc_split_t));
	if(!tokens) {
		//warning("cannot allocate memory for tokens\n");
		return NULL;
	}

	tokens->head = NULL;
	tokens->name = dup_string(name);
	tokens->size = 0;

	return tokens;
}

static lc_token_t* make_node_token(const char* string, size_t index) {
	assert(string != NULL);

	lc_token_t* token = malloc(sizeof(char) * sizeof(lc_token_t));
	if(!token) {
		//warning("cannot allocate memory for token\n");
		return NULL;
	}

	token->string = dup_string(string);
	token->len = strlen(string);
	token->index = index;
	token->next = NULL;

	return token;
}

static lc_split_t* add_node_split(lc_split_t* tokens, lc_token_t* node) {
	assert(tokens != NULL);
	assert(node != NULL);

	lc_token_t* head = tokens->head;
	if(head == NULL) {
		tokens->head = node;
		return tokens;
	}

	while(head->next != NULL) {
		head = head->next;
	}
	head->next = node;

	return tokens;
}

static lc_token_t* get_token_by_id(lc_split_t* tokens, size_t index) {
	assert(tokens != NULL);

	if(tokens->size <= index) {
		//warning("invalid index\n");
		return NULL;
	}

	if(tokens->head == NULL) {
		//warning("list is empty\n");
		return NULL;
	}

	lc_token_t* node = tokens->head;
	while(node->next != NULL) {
		if(node->index == index) {
			return node;
		}

		node = node->next;
	}

	return NULL;
}

// main library api

char* lc_create_config(const char* file) {
	assert(file != NULL);

	char* filepath = NULL;
	if(check_on_path(file) != 0) {
		char* cwd = getcwd(NULL, 0);
		filepath = make_path_to_file(cwd, file);
		free(cwd);
	} else {
		filepath = dup_string(file);
	}

	if(filepath == NULL) return NULL;

	if(file_exists(filepath)) {
		//warning("file is exists: %s\n", filepath);
		return filepath;
	}

	int fd = open(filepath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if(fd == -1) {
		//warning("cannot create file: %s\n", filepath);
		return NULL;
	}

	close(fd);
	return filepath;
}

void lc_delete_config(const char* path) {
	assert(path != NULL);

	remove(path);
}

int lc_var_exists(const char* file, const char* variable) {
	assert(file != NULL);
	assert(variable != NULL);

	FILE* fp = open_file(file, "r");
	if(!fp) return -1;

	char* line_buffer = NULL;
	while((line_buffer = read_line_from_file(fp)) != NULL) {
		char* value = strtok(line_buffer, "=");

		if(strcmp(value, variable) == 0) {
			free(line_buffer);
			fclose(fp);
			return 0;
		}
		free(line_buffer);
		line_buffer = NULL;
	}

	fclose(fp);
	return -1;
}

int lc_insert_var(const char* file, const char* name, const char* value) {
	assert(file != NULL);
	assert(name != NULL);
	assert(value != NULL);

	if(!lc_var_exists(file, name)) {
		//warning("variable %s is exists in file: %s\n", name, file);
		return -1;
	}

	FILE* fp = open_file(file, "a");
	if(!fp) return -1;

	char* variable = make_variable(name, value);
	if(variable == NULL) return -1;

	write_string_to_file(fp, variable);

	free(variable);
	fclose(fp);
	return 0;
}

int lc_delete_var(const char* file, const char* variable) {
	assert(file != NULL);
	assert(variable != NULL);

	if(lc_var_exists(file, variable) != 0) return 0;

	ssize_t line_number;
	if((line_number = find_line_number(file, variable)) == -1) {
		//warning("cannot find line that contain %s variable\n", variable);
		return -1;
	}

	if(write_file_without_line(file, line_number) != 0) {
		//warning("error on write file\n");
		return -1;
	}

	return 0;
}

int lc_rewrite_var(const char* file, const char* variable, const char* new_value) {
	assert(file != NULL);
	assert(variable != NULL);
	assert(new_value != NULL);

	if(lc_var_exists(file, variable) != 0) {
		//warning("this variable does not exists\n");
		return -1;
	}

	ssize_t line_number;
	if((line_number = find_line_number(file, variable)) == -1) {
		//warning("cannot find line that contain %s variable\n", variable);
		return -1;
	}

	char* new_variable = make_variable(variable, new_value);
	if(new_variable == NULL) return -1;

	if(rewrite_file_with_new_line(file, line_number, new_variable) != 0) {
		//warning("error on rewrite file\n");
		free(new_variable);
		return -1;
	}

	free(new_variable);
	return 0;
}

char* lc_get_var(const char* file, const char* variable) {
	assert(file != NULL);
	assert(variable != NULL);

	if(lc_var_exists(file, variable)) {
		//warning("variable %s doesn't exists in file: %s\n", variable, file);
		return NULL;
	}

	FILE* fp = open_file(file, "r");
	if(!fp) return NULL;

	char* line_buffer = NULL;
	char* result_value = NULL;
	while((line_buffer = read_line_from_file(fp)) != NULL) {
		char* variable_name = strtok(line_buffer, "=");
		char* variable_value = strtok(NULL, "=");

		if(strcmp(variable_name, variable) == 0) {
			result_value = dup_string(variable_value);
			free(line_buffer);
			break;
		}
		free(line_buffer);
		line_buffer = NULL;
	}

	fclose(fp);
	return result_value;
}

void lc_display_config(const char* file) {
	assert(file != NULL);

	FILE* fp = open_file(file, "r");
	if(!fp) return;

	char* line_buffer = NULL;
	while((line_buffer = read_line_from_file(fp)) != NULL) {
		printf("%s\n", line_buffer);

		free(line_buffer);
		line_buffer = NULL;
	}

	fclose(fp);
}

lc_split_t* lc_split_var(const char* file, const char* name, const char* delim) {
	assert(file != NULL);
	assert(name != NULL);
	assert(delim != NULL);

	char* value = lc_get_var(file, name);
	if(!value) return NULL;

	lc_split_t* tokens = init_split(name);
	if(!tokens) return NULL;

	ssize_t tokens_size;
	if((tokens_size = find_amount_of_tokens_in_value(value, delim)) != -1) {
		tokens->size = tokens_size;
	} else {
		free(value);
		free(tokens);
		return NULL;
	}

	size_t index = 0;
	lc_token_t* node = NULL;
	char* token = strtok(value, delim);
	while(token != NULL) {
		node = make_node_token(token, index);
		if(node == NULL) {
			free(value);
			free(tokens);
			return NULL;
		}

		tokens = add_node_split(tokens, node);

		index++;
		token = strtok(NULL, delim);
	}

	free(value);
	return tokens;
}

char* lc_get_token(lc_split_t* tokens, size_t index) {
	assert(tokens != NULL);

	lc_token_t* token = NULL;
	if((token = get_token_by_id(tokens, index)) != NULL) {
		return dup_string(token->string);
	}

	return NULL;
}

void lc_free_split(lc_split_t* tokens) {
	assert(tokens != NULL);

	lc_token_t* head = tokens->head;
	lc_token_t* temp = NULL;

	while(head != NULL) {
		temp = head;
		head = head->next;

		free(temp->string);
		free(temp);
	}

	free(tokens->name);
	free(tokens);
}

void lc_print_tokens(lc_split_t* tokens) {
	assert(tokens != NULL);

	lc_token_t* head = tokens->head;
	while(head != NULL) {
		printf("%s, ", head->string);
		head = head->next;
	}
	printf("\n");
}

