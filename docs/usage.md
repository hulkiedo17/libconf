# basic usage of library

```c
#include <stdio.h>
#include <stdlib.h>
#include <libconf.h>

int main(void) {
	char* path = create_file("test.txt");

	insert_variable(path, "OS", "Linux");
	insert_variable(path, "Arch", "x86");
	insert_variable(path, "SHELL", "bash");

	show_content(path);

	delete_variable(path, "SHELL");

	show_content(path);

	rewrite_variable(path, "OS", "Ubuntu");

	show_content(path);

	char* OS = get_variable(path, "OS");
	printf("OS = %s\n", OS);

	// if you want, you can delete file
	// delete_file(path);

	free(path);
	free(OS);
	return 0;
}
```

here you create a file called test.txt then add 3 variables to it: OS, Arch, SHELL. then the contents of that file are displayed. next, you delete the SHELL variable, and overwrite the value of the OS variable. it then takes the value of the OS variable and displays it. at the end, we release the memory from the path to the file and the value of the OS variable.

# usage of split category functions
here we have a file with this data(in the same directory as the code file):
```shell
TEST1=a|b|c|d|e
TEST2=a.b.c.d.e
```

and this code:
```c
#include <stdio.h>
#include <stdlib.h>
#include <libconf.h>

int main(void) {
	char* file = "test.txt";

	// getting all tokens from TEST1 variable, with "|" delim
	split_t* test1 = split_variable(file, "TEST1", "|");
	if(test1 == NULL) {
		return EXIT_FAILURE;
	}

	// getting all tokens from TEST2 variable, with "." delim
	split_t* test2 = split_variable(file, "TEST2", ".");
	if(test2 == NULL) {
		return EXIT_FAILURE;
	}

	print_split(test1); // output: a, b, c, d, e
	print_split(test2); // output: a, b, c, d, e

	char* value1 = get_token_split(test1, 1);	// get "b" from test1 list
	if(value1 == NULL) {
		return EXIT_FAILURE;
	}

	char* value2 = get_token_split(test2, 3);	// get "d" from test2 list
	if(value2 == NULL) {
		return EXIT_FAILURE;
	}

	printf("value(1) = %s\n", value1);
	printf("value(3) = %s\n", value2);

	free_split(test1);
	free_split(test2);

	free(value1);
	free(value2);

	return 0;
}
```

here we take the TEST1 and TEST2 variables from the file and split it's values into tokens that contains in linked list structure, named split_t, and display the content of lists. Also we getting 2 times tokens by his index, get token with 1 index from test1 and token with 3 index from test2, and then displaying them. Then we release the lists from memory.
