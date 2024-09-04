#!/bin/bash -x

set -e

mkdir build && cd build
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DEXTRA_TESTS=OFF
make -j4
make test ARGS=--output-on-failure
make package
