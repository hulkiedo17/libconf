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

