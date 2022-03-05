#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

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

FILE* open_file(char* filename, char* mode) {
	FILE* fp = fopen(filename, mode);
	if(fp == NULL) {
		warning("cannot open file\n");
		return NULL;
	}
	return fp;
}

static char* read_line(FILE* fp) {
	if(fp == NULL) {
		warning("cannot access file\n");
		return NULL;
	}

	int c, pos = 0, len_line = 128;
	char* line = calloc(len_line, sizeof(char));
	if(line == NULL) {
		error("cannot calloc buffer\n");
	}

	while(1) {
		c = fgetc(fp);

		if(c == EOF || c == '\n'){
			line[pos] = '\0';
			return line;
		} else {
			line[pos] = c;
		}
		
		pos++;

		if(pos >= len_line) {
			len_line += 128;
			line = realloc(line, len_line);
			if(line == NULL) {
				error("cannot realloc buffer\n");
			}
		}
	}

	return NULL;
}

static size_t buflen(char* buffer) {
	size_t i = 0;
	if(buffer == NULL) {
		return i;
	}

	while(buffer[i] != '\0') {
		i++;
	}

	return i;
}

static char* dup(char* string) {
	size_t length;
	char* dup = NULL;

	length = buflen(string) + 1;
	dup = malloc(length * sizeof(char));
	if(dup == NULL) {
		error("cannot allocate mem for dup string");
	}

	strncpy(dup, string, length);
	return dup;
}

void write_var(FILE* fp, char* var, char* value) {
	fprintf(fp, "%s=%s\n", var, value);
}

char* read_var(FILE* fp, char* var) {
	rewind(fp);
	char * line = NULL;
	char * result = NULL;
	while((line = read_line(fp)) != NULL) {
		if(line[0] == '\0') {
			free(line);
			break;
		}

		char* name = strtok(line, "=");
		char* value = strtok(NULL, "=");

		if(strcmp(var, name) == 0) {
			result = dup(value);
			free(line);
			break;
		}
		free(line);
		line = NULL;
	}

	return result;
}

/*char* delete_var(FILE* fp, char* var) {

}*/

void print_all(FILE* fp) {
	rewind(fp);

	char* line = NULL;
	while((line = read_line(fp)) != NULL) {
		if(line[0] == '\0') {
			free(line);
			break;
		}

		printf("%s\n", line);
		free(line);
		line = NULL;
	}
}

/*int main(void) {
	FILE* fp = open_file("test.txt", "a+");
	if(fp == NULL) {
		error("fp is null\n");
	}

	//write_var(fp, "OS", "Linux");
	//write_var(fp, "Arch", "x86");

	//char* result = read_var(fp, "Arch");
	//printf("result = %s\n", result);

	print_all(fp);

	fclose(fp);
}*/
