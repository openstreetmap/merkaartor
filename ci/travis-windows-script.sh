#!/bin/bash -x

set -e

mkdir build && cd build
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
make -j4
make test
make package
