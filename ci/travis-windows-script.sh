#!/bin/bash -x

set -e

mkdir build && cd build
cmake .. -G"Unix Makefiles"
make -j4
cygcheck ./test-projection.exe
PROJ_DEBUG=3 ./test-projection
make test
make package
