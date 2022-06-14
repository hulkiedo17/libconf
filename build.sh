#!/usr/bin/env bash

debug() {
	mkdir -p build
	cd build || exit

	cmake "-DCMAKE_BUILD_TYPE=DEBUG" ".."
	make
	sudo make install
}

release() {
	mkdir -p build
	cd build || exit

	cmake "-DCMAKE_BUILD_TYPE=RELEASE" ".."
	make
	sudo make install
}

clean() {
	cd build || exit
	make clean
}

help() {
	printf "build.sh options:\n"
	printf "\t-d - build and install debug version\n"
	printf "\t-r - build and install release version\n"
	printf "\t-c - clean the build directory\n"
	printf "\t-h - show help message\n"
}

handle_options() {
	if [ -z "$*" ]; then
		debug
		exit 0
	fi

	while getopts "drch" opt; do
		case $opt in
			d) debug ;;
			r) release ;;
			c) clean ;;
			h) help ;;
			*) echo "unknown option" ;;
		esac
	done
}

handle_options "$@"
