#include <stdio.h>
#include <stdlib.h>
#include "libconf.h"

int main(void) {	
	/*write_to_file(file, "hello", "world");
	append_to_file(file, "hello1", "world1");
	append_to_file(file, "hello2", "world2");
	append_to_file(file, "hello3", "world3");

	char* value = read_variable(file, "hello2");
	printf("value = %s\n", value);

	char* buf = read_file_to_buffer(file);
	buf = delete_variable_from_buffer(buf, "OS");
	//printf("buff = %s", buf);
	write_buffer_to_file(file, buf);
	free(buf);

	char* buf = read_file_to_buffer(file);
	buf = insert_variable_to_buffer(buf, "OS", "Linux");
	append_buffer_to_file(file, buf);

	free(buf);

	char* buf = read_file_to_buffer(file);
	buf = insert_variable_to_buffer(buf, "Arch", "Linux");
	buf = insert_variable_to_buffer(buf, "name", "value");
	buf = insert_variable_to_buffer(buf, "name1", "value1");
	write_buffer_to_file(file, buf);
	free(buf);

	write_to_file(file, "EDITOR", "vim");
	append_to_file(file, "USER", "hulki");*/

	//print_file(file);

	char* file = "test2.txt";
	/*write_to_file(file, "name1", "value2");
	append_to_file(file, "name2", "value2");
	append_to_file(file, "name3", "value3");
	append_to_file(file, "name4", "value4");*/

	//print_file(file);

	/*char* value = read_variable(file, "name1");
	printf("val = %s\n", value);
	free(value);*/

	char* buf = read_file_to_buffer(file);

	/*buf = insert_variable_to_buffer(buf, "name5", "value5");
	buf = insert_variable_to_buffer(buf, "name6", "value6");
	buf = insert_variable_to_buffer(buf, "name7", "value7");
	buf = insert_variable_to_buffer(buf, "name8", "value8");

	write_buffer_to_file(file, buf);
	free(buf);

	buf = read_file_to_buffer(file);
	buf = delete_variable_from_buffer(buf, "name4");
	buf = delete_variable_from_buffer(buf, "name5");
	buf = delete_variable_from_buffer(buf, "name8");
	buf = delete_variable_from_buffer(buf, "name1");

	write_buffer_to_file(file, buf);
	free(buf);

	append_to_file(file, "name1", "value1");
	append_to_file(file, "name4", "value4");
	append_to_file(file, "name5", "value5");
	append_to_file(file, "name8", "value8");*/

	printf("buf: %s\n", buf);
	free(buf);

	/*buf = delete_variable_from_buffer(buf, "name2");
	buf = delete_variable_from_buffer(buf, "name3");
	buf = delete_variable_from_buffer(buf, "name6");
	buf = delete_variable_from_buffer(buf, "name7");
	buf = delete_variable_from_buffer(buf, "name1");
	buf = delete_variable_from_buffer(buf, "name4");
	buf = delete_variable_from_buffer(buf, "name5");
	buf = delete_variable_from_buffer(buf, "name8");

	write_buffer_to_file(file, buf);
	free(buf);*/



	// test with full file
	// test with empty file

	// test the last var
	// test the first var
	// test var in the middle

	// add feature that you cannot insert the same variables

	return 0;
}

// TODO:
// check memleak insert_variable_to_buffer()
// check memleak delete_variable_from_buffer()
// check memleak append_buffer_to_file()
// check memleak write_buffer_to_file()
// check memleak read_variable()
// check memleak append_to_file()
// check memleak write_to_file()
// check memleak create_variable_string()
// check memleak find_variable_position()
// check memleak copy_buffer()
// check memleak put_str_to_buffer()
// check memleak put_str_to_file()