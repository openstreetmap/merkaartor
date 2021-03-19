#!/bin/bash -x

set -e

mkdir build && cd build
cmake .. -G"Unix Makefiles"
make -j4
make test
make package
