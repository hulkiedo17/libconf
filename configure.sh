#!/bin/bash

prefix=/usr/local
debug=true

handle_options() {
    for arg in $@; do
        case "$arg" in
            --prefix=*)
                prefix=`echo $arg | sed 's/--prefix=/'`
                ;;
            --debug)
                debug=true
                ;;
            --release)
                debug=false
                ;;
            --help)
                echo "usage: ./configure.sh [options...]"
                echo "options:"
                echo "  --prefix=<path>:    installation path prefix"
                echo "  --debug:            compile debug version, enabled by default"
                echo "  --release:          compile release version"
                echo "all invalid options are silently ignored"
                exit 0
                ;;
        esac
    done
}

made_makefile() {
    mkdir -p build

    echo "generation makefile..."

    echo ".POSIX:" >Makefile
    echo "PREFIX = $prefix" >>Makefile

    if $debug_flag; then
        echo "DEBUG = TRUE" >>Makefile
    else
        echo "DEBUG = FALSE" >>Makefile
    fi

    cat Makefile.in >>Makefile

    echo "configuration complete, type make to build"
}

handle_options "$@"
made_makefile