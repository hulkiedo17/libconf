#include <stdio.h>
#include <stdlib.h>
#include "libconf.h"

int main(void) {
	FILE* fp = open_file("test.txt", "a+");

	//write_var(fp, "EDITOR", "vim");

	char* value = read_var(fp, "EDITOR");
	printf("value = %s\n", value);
	free(value);

	print_all(fp);

	fclose(fp);
}
