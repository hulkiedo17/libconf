## build.sh options

options:
- -d - build and install debug version
- -r - build and install release version(not recommended for now)
- -c - clean the build directory
- -h - show help message

If you call build.sh without options, the -d flag will be used by default.

## cmake usage

also, instead of a script, you can call cmake itself, here is an example:
```shell
$ mkdir build && cd build
$ cmake ..
$ make
$ sudo make install
```

or, instead of a make, you can call cmake again:
```shell
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
$ cmake --install .
```

## cmake options

you can choose the version of library, debug or release:
```shell
$ cmake -DCMAKE_BUILD_TYPE=<RELEASE|DEBUG> ..
```

and type install prefix:
```shell
$ cmake -DCMAKE_INSTALL_PREFIX=/path/to/dir/ ..
```

