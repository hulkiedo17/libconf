# basic usage of library

```c
#include <stdio.h>
#include <stdlib.h>
#include <libconf.h>

int main(void) {
	char* path = lc_create_config("test.txt");

	lc_insert_var(path, "OS", "Linux");
	lc_insert_var(path, "Arch", "x86");
	lc_insert_var(path, "SHELL", "bash");

	lc_display_config(path);

	lc_delete_var(path, "SHELL");

	lc_display_config(path);

	lc_rewrite_var(path, "OS", "Ubuntu");

	lc_display_config(path);

	char* OS = lc_get_var(path, "OS");
	printf("OS = %s\n", OS);

	// if you want, you can delete file
	// lc_delete_config(path);

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
	lc_split_t* test1 = lc_split_var(file, "TEST1", "|");
	if(test1 == NULL) {
		return EXIT_FAILURE;
	}

	// getting all tokens from TEST2 variable, with "." delim
	lc_split_t* test2 = lc_split_var(file, "TEST2", ".");
	if(test2 == NULL) {
		return EXIT_FAILURE;
	}

	lc_print_tokens(test1); // output: a, b, c, d, e
	lc_print_tokens(test2); // output: a, b, c, d, e

	char* value1 = lc_get_token(test1, 1);	// get "b" from test1 list
	if(value1 == NULL) {
		return EXIT_FAILURE;
	}

	char* value2 = lc_get_token(test2, 3);	// get "d" from test2 list
	if(value2 == NULL) {
		return EXIT_FAILURE;
	}

	printf("value(1) = %s\n", value1);
	printf("value(3) = %s\n", value2);

	lc_free_split(test1);
	lc_free_split(test2);

	free(value1);
	free(value2);

	return 0;
}
```

here we take the TEST1 and TEST2 variables from the file and split it's values into tokens that contains in linked list structure, named lc_split_t, and display the content of lists. Also we getting 2 times tokens by his index, get token with 1 index from test1 and token with 3 index from test2, and then displaying them. Then we release the lists from memory.
