#!/bin/bash

mkdir -p build

make
make "test"
sudo make "install"

