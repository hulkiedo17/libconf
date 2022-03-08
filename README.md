# libconf
libconf - small library to process config files

# build and install
this library work only on linux(ubuntu).

before build you need to install: gcc, make, bash

to build and install type this commands:
```shell
$ ./configure.sh
$ make
$ sudo make install
```

to build tests, type this:
```shell
$ make test
```

# using in projects

after install you can use it in your own projects with this include:
```c
#include <libconf.h>
```

and on compilation, type to compiler/linker this option:
```shell
$ gcc [files...] -lconf [options...]
```
