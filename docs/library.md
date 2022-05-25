# Library API



## 1 - Structs in library

Here are the structures with which you interact when working with the library (not all structures):

```c
typedef struct lc_config
{
	struct _lc_config_list *list;
	size_t list_size;
	enum _lc_config_error error_type;
	char *filepath;
	char *delim;
} lc_config_t;
```
The main configuration structure that stores the linked list of variables, the size of the list, the error type code, the path to the file (optional), and the variable separator (separates the name and value in the variable) (required).

---

```c
typedef struct lc_config_variable
{
	char * name;
	char * value;
} lc_config_variable_t;
```

А structure that holds the name and value. Represents a variable in the configuration (essentially a string from the configuration file).

## 2 - Return values

Function return values:
- LC_SUCCESS - successful execution
- LC_ERROR - unsuccessful execution

There is one exception: the lc_is_variable_in_config() function returns an lc_existence_t enum type, which is shown below:
```c
typedef enum lc_existence
{
	LC_EF_ERROR = 0,
	LC_EF_EXISTS = 1,
	LC_EF_NOT_EXISTS = 2
} lc_existence_t;
```

---

Configuation errors enumeration:
This is an enumeration containing error codes that occur when working with lc_config_t (for example: the required element was not found, or the file was not opened) and you can view them for better error handling.

```c
enum _lc_config_error
{
	LC_ERR_NONE = 0,
	LC_ERR_EMPTY = 1,
	LC_ERR_FILE_NO = 2,
	LC_ERR_MEMORY_NO = 3,
	LC_ERR_WRITE_NO = 4,
	LC_ERR_NOT_EXISTS = 5
};
```

## 3 - API

```c
int lc_init_config(lc_config_t *config, const char *filepath, const char *delim);
```
Configuration structure initialization function.

Required arguments:
- config - address of a local lc_config_t variable.
- filepath - a string containing the path to the file (may be NULL).
- delim - a string containing the deliminator for variables (cannot be NULL).

(important to know, you need to set the separator right away, otherwise when reading lines from a file it will not be possible to split the line into a value and a name to create a configuration variable)

Return value:
- LC_ERROR on error.
- LC_SUCCESS on success.

---

```c
void lc_clear_config(lc_config_t *config);
```
Configuration structure cleanup function.

Required argument:
- config - address of a local lc_config_t variable.

---

```c
char* lc_get_delim(lc_config_t *config);
```
Function to get delimiter from configuration structure.

Required argument:
- config - address of a local lc_config_t variable.

Return value:
- NULL on error.
- pointer to string(delimiter) on success.

(important to know: after use, you need to free the pointer returned from the function from memory.)

---

```c
int lc_set_delim(lc_config_t *config, const char *delim);
```
Function to set new delimiter from configuration structure.

Required argument:
- config - address of a local lc_config_t variable.
- delim - a string containing the deliminator for variables.

Return value:
- LC_ERROR on error.
- LC_SUCCESS on success.

---

```c
size_t lc_get_size(const lc_config_t *config);
```
А function that returns the size of the list of variables in the configuration structure.

Required argument:
- config - address of a local lc_config_t variable.

Return value:
- size_t

---

```c
char* lc_get_path(const lc_config_t *config);
```

А function that returns the file path stored in the configuration structure.

Required argument:
- config - address of a local lc_config_t variable.

Return value:
- NULL on error.
- pointer to string(path to file) on success.

(important to know: after use, you need to free the pointer returned from the function from memory.)

---

```c
int lc_set_path(lc_config_t *config, const char *filepath);
```

А function that sets the new file path stored in the configuration structure.

Required argument:
- config - address of a local lc_config_t variable.
- filepath - a string containing the path to file.

Return value:
- LC_ERROR on error.
- LC_SUCCESS on success.

---

```c
void lc_clear_path(lc_config_t *config);
```

А function that releases from memory the pointer value that stores the path to the file in the configuration structure.

Required argument:
- config - address of a local lc_config_t variable.

---

```c
int lc_load_config(lc_config_t *config, const char *filepath);
```

This function loads data from a file into a config structure.

Required argument:
- config - address of a local lc_config_t variable.
- filepath - a string containing the path to file.

(the function can use either the filepath specified in the function arguments, or if it is NULL, then use the filepath in the configuration structure itself, which was given when calling lc_init_config())

Return value:
- LC_ERROR on error.
- LC_SUCCESS on success.

---

```c
int lc_load_config_stream(lc_config_t *config, FILE *fp);
```

This function loads data from a file stream into a configuration structure.

Required argument:
- config - address of a local lc_config_t variable.
- fp - file pointer.

(the function strictly uses the file pointer, if it is NULL, the function will return LC_ERROR)

Return value:
- LC_ERROR on error.
- LC_SUCCESS on success.

---

```c
int lc_dump_config(lc_config_t *config, const char *filepath);
```

This function dumps the data from the configuration structure into a file.

Required argument:
- config - address of a local lc_config_t variable.
- filepath - a string containing the path to file.

(the function can use either the filepath specified in the function arguments, or if it is NULL, then use the filepath in the configuration structure itself, which was given when calling lc_init_config())

Return value:
- LC_ERROR on error.
- LC_SUCCESS on success.

---

```c
int lc_dump_config_stream(lc_config_t *config, FILE *fp);
```

This function dumps the data from the config structure to the file stream

Required argument:
- config - address of a local lc_config_t variable.
- fp - file pointer.

(the function strictly uses the file pointer, if it is NULL, the function will return LC_ERROR)

Return value:
- LC_ERROR on error.
- LC_SUCCESS on success.

---

```c
void lc_print_config(const lc_config_t *config);
```

This function prints the linked list of variables from the configuration structure to the console output.

Required argument:
- config - address of a local lc_config_t variable.

---

```c
char* lc_get_error(const lc_config_t *config);
```

This function returns the description of the error as an allocated string.

Required argument:
- config - address of a local lc_config_t variable.

Return value:
- NULL on error.
- pointer to string(error description) on success.

(keep in mind, after use, you need to free the returned allocated string from memory.)

---

