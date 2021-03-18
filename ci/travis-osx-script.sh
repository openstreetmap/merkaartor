#!/bin/sh

set -ev

mkdir build && cd build
cmake .. -G"Unix Makefiles" -DCMAKE_PREFIX_PATH="`brew --prefix qt5`"
make -j4
make test
make package
