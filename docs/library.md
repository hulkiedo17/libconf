# library functions:

```c
char* create_file(char* path);
```
this function creates a file and returns the path to it. If a path to a file is passed to the function, then the file is created along the path passed to the function. Otherwise, the file will be created in the home directory. The return path from the function is the allocated string in memory.

```c
void delete_file(char* path);
```
this function deletes the file, if it exists, at the path passed to the function. used after calling the create_file() function.

```c
int is_variable_exists(char* file, char* variable);
```
this function determines if a variable exists in a file by its name. Return value: -1 if it is not in the file (or search error), 0 if it is in the file.

```c
int insert_variable(char* file, char* name, char* value);
```
inserts a variable (with the following structure: name=value) into the file, at the specified path. If this variable is already in the file, then the insertion will not be performed.

```c
int delete_variable(char* file, char* variable);
```
deletes a variable in a file at the specified path. If there is no variable with the specified name in the file, then nothing will be done.

```c
int rewrite_variable(char* file, char* variable, char* new_value);
```
this function overwrites the value of the variable in the file at the specified path. If the variable does not exist in the file, then nothing will be done.

```c
char* get_variable(char* file, char* variable);
```
this function returns the value of the variable from the file at the specified path as an allocated string. If the variable does not exist in the file, then NULL is returned.

```c
int show_content(char* file);
```
this function outputs all variables from the file at the specified path to the output.

<!---
## split category functions:
this category of functions is designed to handle multiple values in one variable, separated by one specific character, for example: var=a,b,c,d,e

```c
char** split_values(char* file, char* variable_name, int* size, char* delim);
```
this function splits the value of the variable into separate values (tokens) and places them in an allocated two-dimensional array of strings (be careful when working with it). the size argument is passed the address of the variable in which the size of the array (the number of values / tokens) will be placed.

```c
char* get_split_from_values(char** tokens, int size, int index);
```
this function takes a single value (token/string) from the array by index, if it is in the range. The value taken from the array by index is copied into the allocated string in memory and returned from the function.

```c
void show_split_values(char** tokens, int size);
```
this function shows all values (tokens) in a two-dimensional array.

```c
void free_split_values(char** tokens, int size);
```
this function frees an allocated two-dimensional array from memory.
-->
