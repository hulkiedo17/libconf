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
EXTENSIONS=.s:.c:.a:.o:.cpp:.S
```

and this code:
```c
#include <stdio.h>
#include <stdlib.h>
#include <libconf.h>

int main(void) {
	char* file = "./data.txt";

	int size = 0;	// here through size variable we get the array size
	char** extensions = split_values(file, "EXTENSIONS", &size, ":");
	if(extensions == NULL) { printf("this var doesn't exists"); exit(EXIT_FAILURE); }

	show_split_values(extensions, size);

	char* cpp_extension = get_split_from_values(extensions, size, 4);	// 4 because .cpp have this index
	printf("cpp = %s\n", cpp_extension);

	free_split_values(extensions);
	free(cpp_extension);
	return 0;
}
```

here we take the EXTENSIONS variable from the file and split it's value into tokens separated by the ":" character, and put the size of the returned array into the size variable by passing it's address to the function. then display the contents of the array with tokens. we then take a single token from the array at a certain index (in this case, .cpp has index 4), and return it from the function, which we then output. At the end we release the array with tokens and the token from memory.

