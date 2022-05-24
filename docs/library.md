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

### initialize configuration

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

### cleanup configuration

```c
void lc_clear_config(lc_config_t *config);
```
Configuration structure cleanup function.

Required argument:
- config - address of a local lc_config_t variable.
