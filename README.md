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

and on compilation, type to compiler/linker this option:
```shell
$ gcc [files...] -lconf [options...]
```

# documentation
you can see descriptions about functions in docs/library.md, how to build and install library in docs/compile.md.

and also type -h flag to build.sh, to see options for build library.
