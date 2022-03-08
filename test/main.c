#include <stdio.h>
#include <stdlib.h>
#include <libconf.h>

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("please enter filename\n");
		exit(EXIT_FAILURE);
	}

	write_to_file(argv[1], "name1", "value1");
	insert_to_file(argv[1], "name2", "value2");

	print_file(argv[1]);

	return 0;
}
