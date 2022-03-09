# libconf
libconf - small library to process config files

# build and install
this library work only on linux.

before build you need to install: gcc, make, cmake, bash

to build and install type this commands:
```shell
$ ./build.sh
```

# using in projects
after install you can use it in your own projects with this include:
```c
#include <libconf.h>
```

and on compilation, type to compiler/linked this option:
```shell
$ gcc [files...] -lconf [options...]
```

# help
you can see help in build.sh with -h option.
