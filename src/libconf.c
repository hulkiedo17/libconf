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

static void warning(const char* fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}

static int file_exists(char* path) {
	struct stat buffer;

	if(stat(path, &buffer) == 0) {
		return 1;
	}
	return 0;
}

static FILE* open_file(char* filename, char* mode) {
	assert(filename != NULL);
	assert(mode != NULL);

	FILE* fp = fopen(filename, mode);
	if(fp == NULL) {
		warning("cannot open file\n");
		return NULL;
	}

	return fp;
}

static char* dup_string(char* string) {
	assert(string != NULL);

	int len = strlen(string) + 1;
	char* dup = malloc(len * sizeof(char));
	if(dup == NULL) {
		warning("cannot malloc string\n");
		return NULL;
	}

	strncpy(dup, string, len);
	return dup;
}

static char* get_name_from_string(char* string) {
	char* temp = dup_string(string);
	char* name = strtok(temp, "=");
	
	char* res = dup_string(name);
	/*if(res == NULL) {
		return NULL;
	}*/

	free(temp);
	return res;
}

static int write_string_to_file(FILE* fp, char* string) {
	assert(fp != NULL);
	assert(string != NULL);

	int i = 0, len = strlen(string);
	for(; i < len; i++) {
		fputc(string[i], fp);
	}
	fputc('\n', fp);

	return i;
}

static char* read_line_from_file(FILE* fp) {
	assert(fp != NULL);

	int c, pos = 0, len_line = LINE_SIZE;
	char* line = calloc(len_line, sizeof(char));
	if(line == NULL) {
		warning("cannot calloc buffer\n");
		return NULL;
	}

	while(1) {
		c = fgetc(fp);

		if(c == EOF || c == '\n') {
			if(pos == 0 && (c == EOF || c == '\n')) {
				free(line);
				return NULL;
			}
			line[pos] = '\0';
			return line;
		} else {
			line[pos] = c;
		}

		pos++;

		if(pos >= len_line) {
			len_line += LINE_SIZE;
			line = realloc(line, len_line);
			if(line == NULL) {
				warning("cannot realloc buffer\n");
				return NULL;
			}
		}
	}

	return NULL;
}

static int find_line_number(char* file, char* variable) {
	assert(file != NULL);
	assert(variable != NULL);

	FILE* fp = open_file(file, "r");
	if(!fp) return -1;

	int line_count = 1;
	char* line = NULL;
	while((line = read_line_from_file(fp)) != NULL) {
		char* name = strtok(line, "=");

		if(strcmp(name, variable) == 0) {
			free(line);
			fclose(fp);
			return line_count;
		}
		free(line);
		line = NULL;
		line_count++;
	}

	fclose(fp);
	return -1;
}

static char* make_temp_file_path(char* file) {
	assert(file != NULL);

	char* temp = malloc(sizeof(char) * (strlen(file) + 5));
	if(temp == NULL) {
		warning("cannot malloc temp file path\n");
		return NULL;
	}
	
	strcpy(temp, file);
	strcat(temp, ".tmp");

	return temp;
}

static int write_file_without_line(char* file, int line_number) {
	assert(file != NULL);

	FILE* fp = open_file(file, "r");
	if(!fp) return -1;

	char* temp_file = make_temp_file_path(file);
	if(!temp_file) return -1;

	FILE* tmp_fp = open_file(temp_file, "a");
	if(!tmp_fp) return -1;

	int line_count = 1;
	char* line = NULL;
	while((line = read_line_from_file(fp)) != NULL) {
		if(line_count != line_number) {
			write_string_to_file(tmp_fp, line);
		}
		free(line);
		line = NULL;
		line_count++;
	}

	fclose(fp);
	fclose(tmp_fp);

	remove(file);
	rename(temp_file, file);

	free(temp_file);
	return 0;
}

static int rewrite_file_with_new_line(char* file, int line_number, char* new_line) {
	assert(file != NULL);
	assert(new_line != NULL);

	FILE* fp = open_file(file, "r");
	if(!fp) return -1;

	char* temp_file = make_temp_file_path(file);
	if(!temp_file) return -1;

	FILE* tmp_fp = open_file(temp_file, "a");
	if(!tmp_fp) return -1;

	int line_count = 1;
	char* line = NULL;
	while((line = read_line_from_file(fp)) != NULL) {
		if(line_count != line_number) {
			write_string_to_file(tmp_fp, line);
		} else {
			write_string_to_file(tmp_fp, new_line);
		}

		free(line);
		line = NULL;
		line_count++;
	}

	fclose(fp);
	fclose(tmp_fp);

	remove(file);
	rename(temp_file, file);

	free(temp_file);
	return 0;
}

static int find_amount_of_tokens_in_value(char* value, char* delim) {
	char* val = dup_string(value);
	if(val == NULL) {
		return -1;
	}

	int amount_of_tokens = 0;
	char* token = strtok(val, delim);
	while(token != NULL) {
		amount_of_tokens++;
		token = strtok(NULL, delim);
	}

	free(val);
	return amount_of_tokens;
}

static const char* get_home_dir(void) {
	const char* home_dir = NULL;

	if((home_dir = getenv("HOME")) == NULL) {
		home_dir = getpwuid(getuid())->pw_dir;
	}

	return home_dir;
}

static char* make_path_to_file(const char* path, const char* file) {
	assert(path != NULL);
	assert(file != NULL);

	char* file_path = malloc(sizeof(char) * (strlen(path) + strlen(file) + 2));
	if(file_path == NULL) {
		warning("cannot malloc path to file\n");
		return NULL;
	}

	strcpy(file_path, path);
	strcat(file_path, "/");
	strcat(file_path, file);
	return file_path;
}

static int check_on_path(const char* file) {
	if(strchr(file, '/') != NULL) {
		return 0;
	}

	return 1;
}

static char* make_variable(char* name, char* value) {
	assert(name != NULL);
	assert(value != NULL);

	int len = strlen(name) + strlen(value) + 3;	// +3 because =, \n, \0
	char* var = malloc(len * sizeof(char));
	if(var == NULL) {
		warning("cannot malloc variable\n");
		return NULL;
	}

	strcpy(var, name);
	strcat(var, "=");
	strcat(var, value);

	return var;
}

static split_t* init_split(char* name) {
	split_t* tokens = malloc(sizeof(char) * sizeof(split_t));
	if(!tokens) {
		warning("cannot allocate memory for tokens\n");
		return NULL;
	}

	tokens->head = NULL;
	tokens->name = dup_string(name);
	tokens->size = 0;

	return tokens;
}

static token_t* make_node_token(char* string, int index) {
	assert(string != NULL);

	token_t* token = malloc(sizeof(char) * sizeof(token_t));
	if(!token) {
		warning("cannot allocate memory for token\n");
		return NULL;
	}

	token->string = dup_string(string);
	token->len = strlen(string);
	token->index = index;
	token->next = NULL;

	return token;
}

static split_t* add_node_split(split_t* tokens, token_t* node) {
	assert(tokens != NULL);
	assert(node != NULL);

	token_t* head = tokens->head;
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

// main library api

char* create_file(char* file) {
	assert(file != NULL);

	char* path = NULL;
	if(check_on_path(file) != 0) {
		path = make_path_to_file(get_home_dir(), file);
	} else {
		path = dup_string(file);
	}

	if(path == NULL) {
		return NULL;
	}

	if(file_exists(path)) {
		warning("file is exists: %s\n", path);
		return path;
	}

	int fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if(fd == -1) {
		warning("cannot create file: %s\n", path);
		return NULL;
	}

	close(fd);
	return path;
}

void delete_file(char* path) {
	assert(path != NULL);

	remove(path);
}

int is_variable_exists(char* file, char* variable) {
	assert(file != NULL);
	assert(variable != NULL);

	FILE* fp = open_file(file, "r");
	if(!fp) return -1;

	char* line = NULL;
	while((line = read_line_from_file(fp)) != NULL) {
		char* name = strtok(line, "=");

		if(strcmp(name, variable) == 0) {
			free(line);
			fclose(fp);
			return 0;
		}
		free(line);
		line = NULL;
	}

	fclose(fp);
	return -1;
}

int insert_variable(char* file, char* name, char* value) {
	assert(file != NULL);
	assert(name != NULL);
	assert(value != NULL);

	char* string = make_variable(name, value);
	if(string == NULL) {
		return -1;
	}

	char* var_name = get_name_from_string(string);
	if(var_name == NULL) {
		free(string);
		return -1;
	}

	if(!is_variable_exists(file, var_name)) {
		warning("variable %s is exists in file: %s\n", name, file);
		free(string);
		free(var_name);
		return -1;
	}
	free(var_name);

	FILE* fp = open_file(file, "a");
	if(!fp) return -1;

	write_string_to_file(fp, string);

	free(string);
	fclose(fp);
	return 0;
}

int delete_variable(char* file, char* variable) {
	assert(file != NULL);
	assert(variable != NULL);

	if(is_variable_exists(file, variable) != 0) {
		return 0;
	}

	int line_number;
	if((line_number = find_line_number(file, variable)) == -1) {
		warning("cannot find line that contain %s variable\n", variable);
		return -1;
	}

	if(write_file_without_line(file, line_number) != 0) {
		warning("error on write file\n");
		return -1;
	}

	return 0;
}

int rewrite_variable(char* file, char* variable, char* new_value) {
	assert(file != NULL);
	assert(variable != NULL);
	assert(new_value != NULL);

	if(is_variable_exists(file, variable) != 0) {
		warning("this variable does not exists\n");
		return -1;
	}

	int line_number;
	if((line_number = find_line_number(file, variable)) == -1) {
		warning("cannot find line that contain %s variable\n", variable);
		return -1;
	}

	char* new_variable = make_variable(variable, new_value);
	if(new_variable == NULL) {
		return -1;
	}

	if(rewrite_file_with_new_line(file, line_number, new_variable) != 0) {
		warning("error on rewrite file\n");
		free(new_variable);
		return -1;
	}

	free(new_variable);
	return 0;
}

char* get_variable(char* file, char* variable) {
	assert(file != NULL);
	assert(variable != NULL);

	if(is_variable_exists(file, variable)) {
		warning("variable %s doesn't exists in file: %s\n", variable, file);
		return NULL;
	}

	FILE* fp = open_file(file, "r");
	if(!fp) return NULL;

	char* line = NULL;
	char* result = NULL;
	while((line = read_line_from_file(fp)) != NULL) {
		char* var = strtok(line, "=");
		char* value = strtok(NULL, "=");

		if(strcmp(var, variable) == 0) {
			result = dup_string(value);
			free(line);
			break;
		}
		free(line);
		line = NULL;
	}

	fclose(fp);
	return result;
}

split_t* split_variable(char* file, char* name, char* delim) {
	assert(file != NULL);
	assert(name != NULL);
	assert(delim != NULL);

	char* value = get_variable(file, name);
	if(!value) {
		warning("this %s variable does not exists\n", name);
		return NULL;
	}

	split_t* tokens = init_split(name);
	if(!tokens) {
		return NULL;
	}

	int size;
	if((size = find_amount_of_tokens_in_value(value, delim)) != -1) {
		tokens->size = size;
	} else {
		free(value);
		free(tokens);
		return NULL;
	}

	int index = 0;
	token_t* node = NULL;
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

void free_split(split_t* tokens) {
	assert(tokens != NULL);

	token_t* head = tokens->head;
	token_t* temp = NULL;

	while(head != NULL) {
		temp = head;
		head = head->next;

		free(temp->string);
		free(temp);
	}

	free(tokens->name);
	free(tokens);
}

void print_split(split_t* tokens) {
	assert(tokens != NULL);

	printf("name: %s\n", tokens->name);
	token_t* head = tokens->head;
	while(head != NULL) {
		printf("%s, ", head->string);
		head = head->next;
	}
	printf("\n");
}

int show_content(char* file) {
	assert(file != NULL);

	FILE* fp = open_file(file, "r");
	if(!fp) return -1;

	char* line = NULL;
	while((line = read_line_from_file(fp)) != NULL) {
		printf("%s\n", line);

		free(line);
		line = NULL;
	}

	fclose(fp);
	return 0;
}
