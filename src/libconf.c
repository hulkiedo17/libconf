#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define LINE_SIZE 128

static void error(const char* fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(EXIT_FAILURE);
}

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
		error("cannot malloc string\n");
	}

	strncpy(dup, string, len);
	return dup;
}

static char* get_name_from_string(char* string) {
	char* temp = dup_string(string);
	char* name = strtok(temp, "=");
	char* res = dup_string(name);
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
		error("cannot calloc buffer\n");
	}

	while(1) {
		c = fgetc(fp);

		if(c == EOF || c == '\n') {
			if(pos == 0 && c == EOF) {
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
				error("cannot realloc buffer\n");
			}
		}
	}

	return NULL;
}

/*static int find_line_number(char* file, char* variable) {
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
		error("cannot malloc temp file path\n");
	}
	
	strcpy(temp, file);
	strcat(temp, ".tmp");

	return temp;
}

static char* get_path_file(char* file) {
	//TODO: make strchr to find '/' symbol

	char* path = getcwd(NULL, 0);
	if(path == NULL) {
		warning("getcwd failed\n");
		return NULL;
	}

	char* filepath = malloc(sizeof(char) * (strlen(path) + strlen(file) + 2));
	if(file == NULL) {
		error("cannot malloc memory for path\n");
	}

	strcpy(filepath, path);
	strcat(filepath, "/");
	strcat(filepath, file);
	free(path);

	return filepath;
}

static void move_file(char* file1, char* file2) {
	char* file1_path = get_path_file(file1);
	char* file2_path = get_path_file(file2);

	printf("file1_path = %s\n", file1_path);
	printf("file2_path = %s\n", file2_path);

	if(file_exists(file1_path)) {
		remove(file1_path);
	}
	rename(file2_path, file1_path);
	remove(file2_path);

	free(file1_path);
	free(file2_path);
}

static int write_file_without_line(char* file, int line_number) {
	assert(file != NULL);

	FILE* fp = open_file(file, "r");
	if(!fp) return -1;

	char* temp_file = make_temp_file_path(file);
	if(!temp_file) return -1;

	FILE* tmp_fp = open_file(file, "a");
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

	//move_file(file, temp_file);
	free(temp_file);
	return 0;
}*/

// main library api

char* create_file(char* path) {
	assert(path != NULL);

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

char* make_variable(char* name, char* value) {
	assert(name != NULL);
	assert(value != NULL);

	int len = strlen(name) + strlen(value) + 3;	// +3 because =, \n, \0
	char* variable = malloc(len * sizeof(char));
	if(variable == NULL) {
		error("cannot malloc variable\n");
	}

	strcpy(variable, name);
	strcat(variable, "=");
	strcat(variable, value);

	return variable;
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

int insert_variable(char* file, char* string) {
	assert(file != NULL);
	assert(string != NULL);

	char* name = get_name_from_string(string);
	if(!is_variable_exists(file, name)) {
		warning("variable %s is exists in file: %s\n", name, file);
		free(name);
		return -1;
	}
	free(name);

	FILE* fp = open_file(file, "a");
	if(!fp) return -1;

	write_string_to_file(fp, string);

	fclose(fp);
	return 0;
}

/*int delete_variable(char* file, char* variable) {
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

	printf("line = %d\n", line_number);
	if(write_file_without_line(file, line_number) != 0) {
		warning("error on write file\n");
		return -1;
	}

	return 0;
}*/

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
