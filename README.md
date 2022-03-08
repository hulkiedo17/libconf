# libconf
libconf - small library to process config files

# build and install
this library work only on linux(ubuntu).

before build you need to install: gcc, make, bash

to build and install type this command:
```shell
$ ./build.sh
```

this command build library, tests, and installed library to default path

# using in projects

after install you can use it in your own projects with this include:
```c
#include <libconf.h>
```

and on compilation, type to compiler/linked this option:
```shell
$ gcc [files...] -lconf [options...]
```
