# basic usage of library

```c
#include <stdio.h>
#include <stdlib.h>
#include <libconf.h>

// usage: ./test [file1] [file2]
int main(int argc, char **argv)
{
	if(argc != 3)
		return EXIT_FAILURE;

	lc_config_t config;

	// initialize a config for first file
	lc_init_config(&config, argv[1], ": ");

	// creating a new variable
	lc_config_variable_t *variable = lc_create_variable("name1", "value1");

	// adding variable to config structure
	lc_add_variable(&config, variable);

	// destroy variable (cleanup from memory)
	lc_destroy_variable(variable);

	// dump config to file
	lc_dump_config(&config, NULL);

	// print config to output
	lc_print_config(&config);

	// cleanup config
	lc_clear_config(&config);



	// initialize a config for second file
	lc_init_config(&config, NULL, " = ");

	// loading data from file
	lc_load_config(&config, argv[2]);

	// creating a new variable
	variable = lc_create_variable("variable", "test-value");

	// adding variable to config structure
	lc_add_variable(&config, variable);

	// rename the variable name
	lc_set_variable_name(variable, "variable2");

	// adding the changed variable to config structure
	lc_add_variable(&config, variable);

	// destroy variable (cleanup from memory)
	lc_destroy_variable(variable);

	// dump config to file
	lc_dump_config(&config, argv[2]);

	//print config to output
	lc_print_config(&config);

	// cleanup config
	lc_clear_config(&config);

	return EXIT_SUCCESS;
}
```
