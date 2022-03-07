#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "libconf.h"

#define PUT_NEW_LINE 1
#define NOPUT_NEW_LINE 0
#define LINE_SIZE 128
#define BUFFER_SIZE 512
#define CUR_LINE_POS 0
#define NEXT_LINE_POS 1

static void error(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

static void warning(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
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

static int put_str_to_file(FILE* fp, char* string) {
	assert(fp != NULL);
	assert(string != NULL);

	int pos = 0, len = strlen(string);
	while(pos < len) {
		fputc(string[pos], fp);
		pos++;
	}

	return pos;
}

static char* put_str_to_buffer(char* buffer, char* string) {
	assert(buffer != NULL);
	assert(string != NULL);

	int len_buf = strlen(buffer);
	int len_str = strlen(string);
	int len_newbuf = len_buf + len_str;

	char* new_buffer = malloc(len_newbuf * sizeof(char));
	if(new_buffer == NULL) {
		error("cannot malloc buffer\n");
	}

	strncpy(new_buffer, buffer, len_buf);
	strncpy(new_buffer + len_buf, string, len_str + 1);

	return new_buffer;
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

		if(c == EOF || c == '\n'){
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

static char* copy_buffer(char* buffer) {
	assert(buffer != NULL);

	int len = strlen(buffer) + 1;
	char* new_buffer = malloc(len * sizeof(char));
	if(new_buffer == NULL) {
		error("cannot calloc buffer\n");
	}

	strncpy(new_buffer, buffer, len);
	return new_buffer;
}

static int find_variable_position(char* buffer, char* name, int next_line_flag) {
	assert(buffer != NULL);
	assert(name != NULL);

	int position = 0;
	char* new_buffer = copy_buffer(buffer);

	char* line = strtok(new_buffer, "\n");
	while(line != NULL) {
		if(strstr(line, name) != NULL) {
			if(next_line_flag == NEXT_LINE_POS) {
				position += strlen(line) + 1;
			}

			free(new_buffer);
			return position;
		}
		position += strlen(line) + 1;
		line = strtok(NULL, "\n");
	}

	free(new_buffer);
	return -1;
}

static char* create_variable_string(char* name, char* value) {
	assert(name != NULL);
	assert(value != NULL);

	int len_str = strlen(name) + strlen(value) + 3; // +3 because =, \n, \0
	char* string = malloc(len_str * sizeof(char));
	if(string == NULL) {
		error("cannot malloc string\n");
	}

	sprintf(string, "%s=%s\n", name, value);

	return string;
}

int write_to_file(char* filename, char* name, char* value) {
	if(name == NULL) {
		warning("unknown name\n");
		return -1;
	}
	if(filename == NULL) {
		warning("unknown filename\n");
		return -1;
	}
	if(value == NULL) {
		warning("unknown value\n");
		return -1;
	}

	FILE* fp = open_file(filename, "w+");
	if(!fp) return -1;

	char* string = create_variable_string(name, value);
	put_str_to_file(fp, string);
	free(string);

	fclose(fp);
	return 0;
}

int append_to_file(char* filename, char* name, char* value) {
	if(name == NULL) {
		warning("unknown name\n");
		return -1;
	}
	if(filename == NULL) {
		warning("unknown filename\n");
		return -1;
	}
	if(value == NULL) {
		warning("unknown value\n");
		return -1;
	}

	FILE* fp = open_file(filename, "a+");
	if(!fp) return -1;

	char* string = create_variable_string(name, value);
	put_str_to_file(fp, string);
	free(string);

	fclose(fp);
	return 0;
}

char* read_variable(char* filename, char* name) {
	if(name == NULL) {
		warning("unknown name\n");
		return NULL;
	}
	if(filename == NULL) {
		warning("unknown filename\n");
		return NULL;
	}

	FILE* fp = open_file(filename, "r");
	if(!fp) return NULL;
	
	char * line = NULL;
	char * result = NULL;
	while((line = read_line_from_file(fp)) != NULL) {
		char* var = strtok(line, "=");
		char* value = strtok(NULL, "=");

		if(strcmp(name, var) == 0) {
			result = strdup(value);
			free(line);
			break;
		}
		free(line);
		line = NULL;
	}

	return result;
}

char* read_file_to_buffer(char* filename) {
	if(filename == NULL) {
		warning("unknown filename\n");
		return NULL;
	}

	int c, position = 0, bufsize = BUFFER_SIZE;

	FILE* fp = open_file(filename, "r");
	if(!fp) return NULL;
	
	char* buf = calloc(bufsize, sizeof(char));
	if(buf == NULL) {
		error("cannot calloc buffer\n");
	}

	while(1) {
		c = fgetc(fp);

		if(c == EOF){
			if(position == 0 && c == EOF) {
				free(buf);
				fclose(fp);
				return NULL;
			}

			buf[position] = '\0';
			fclose(fp);
			return buf;
		} else {
			buf[position] = c;
		}
		
		position++;

		if(position >= bufsize) {
			bufsize += BUFFER_SIZE;
			buf = realloc(buf, bufsize);
			if(buf == NULL) {
				error("cannot realloc buffer\n");
			}
		}
	}

	fclose(fp);
	return NULL;
}

int write_buffer_to_file(char* filename, char* buffer) {
	if(filename == NULL) {
		warning("unknown filename\n");
		return -1;
	}
	if(buffer == NULL) {
		warning("buffer is empty");
		return -1;
	}

	FILE* fp = open_file(filename, "w");
	if(!fp) return -1;

	put_str_to_file(fp, buffer);

	fclose(fp);
	return 0;
}

int append_buffer_to_file(char* filename, char* buffer) {
	if(filename == NULL) {
		warning("unknown filename\n");
		return -1;
	}
	if(buffer == NULL) {
		warning("buffer is empty");
		return -1;
	}

	FILE* fp = open_file(filename, "a");
	if(!fp) return -1;

	put_str_to_file(fp, buffer);

	fclose(fp);
	return 0;
}

char* delete_variable_from_buffer(char* buffer, char* name) {
	if(name == NULL) {
		warning("unknown name\n");
		return NULL;
	}
	if(buffer == NULL) {
		warning("buffer is empty\n");
		return NULL;
	}

	int position1 = find_variable_position(buffer, name, CUR_LINE_POS);
	int position2 = find_variable_position(buffer, name, NEXT_LINE_POS);
	int new_buffer_size = strlen(buffer) - (position2 - position1) + 1;

	printf("buffer = %d\n", new_buffer_size);
	if(new_buffer_size <= 0 || new_buffer_size == 1) {
		free(buffer);
		return NULL;
	}

	char* new_buffer = malloc(new_buffer_size * sizeof(char));
	if(new_buffer == NULL) {
		error("cannot malloc buffer\n");
	}

	strncpy(new_buffer, buffer, position1);
	strncpy(new_buffer + position1, buffer + position2, strlen(buffer) - position2 + 1);

	free(buffer);
	return new_buffer;
}

char* insert_variable_to_buffer(char* buffer, char* name, char* value) {
	if(name == NULL) {
		warning("unknown name\n");
		return NULL;
	}
	if(value == NULL) {
		warning("unknown value\n");
		return NULL;
	}
	if(buffer == NULL) {	
		return create_variable_string(name, value);
	}

	char* string = create_variable_string(name, value);
	char* new_buffer = put_str_to_buffer(buffer, string);

	free(buffer);
	free(string);

	return new_buffer;
}

int print_file(char* filename) {
	if(filename == NULL) {
		warning("unknown filename\n");
		return -1;
	}

	FILE* fp = open_file(filename, "r");
	if(!fp) return -1;

	int c;
	while((c = fgetc(fp)) != EOF) {
		putc(c, stdout);
	}

	fclose(fp);
	return 0;
}
