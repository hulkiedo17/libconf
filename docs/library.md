# library functions:

```c
char* lc_create_config(char* path);
```
this function creates a file and returns the path to it. If a path to a file is passed to the function, then the file is created along the path passed to the function. Otherwise, the file will be created in the home directory. The return path from the function is the allocated string in memory.

```c
void lc_delete_config(char* path);
```
this function deletes the file, if it exists, at the path passed to the function. used after calling the lc_create_config() function.

```c
int lc_var_exists(char* file, char* variable);
```
this function determines if a variable exists in a file by its name. Return value: -1 if it is not in the file (or search error), 0 if it is in the file.

```c
int lc_insert_var(char* file, char* name, char* value);
```
inserts a variable (with the following structure: name=value) into the file, at the specified path. If this variable is already in the file, then the insertion will not be performed.

```c
int lc_delete_var(char* file, char* variable);
```
deletes a variable in a file at the specified path. If there is no variable with the specified name in the file, then nothing will be done.

```c
int lc_rewrite_var(char* file, char* variable, char* new_value);
```
this function overwrites the value of the variable in the file at the specified path. If the variable does not exist in the file, then nothing will be done.

```c
char* lc_get_var(char* file, char* variable);
```
this function returns the value of the variable from the file at the specified path as an allocated string. If the variable does not exist in the file, then NULL is returned.

```c
int lc_display_config(char* file);
```
this function outputs all variables from the file at the specified path to the output.

## split category functions:
this category of functions is designed to handle multiple values in one variable, separated by one specific character, for example: var=a,b,c,d,e

```c
lc_split_t* lc_split_var(char* file, char* name, char* delim);
```
this function splits the value of the variable into separate values (tokens) and places them in an linked list(called lc_split_t).

```c
char* lc_get_token(lc_split_t* tokens, int index);
```
this function gets the token by specified index from list of tokens(lc_split_t).

```c
void lc_free_split(lc_split_t* tokens);
```
this function free an linked list (lc_split_t) of tokens from memory.

```c
void lc_print_split(lc_split_t* tokens);
```
this function shows all tokens from linked list (lc_split_t).
