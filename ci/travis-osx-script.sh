#!/bin/sh

set -ev

mkdir build && cd build
QT_PREFIX=`brew --prefix qt${QT_MAJOR}`
find /usr/local -name "QtD*.framework"
cmake .. -G"Unix Makefiles" -DCMAKE_PREFIX_PATH="$QT_PREFIX" -DCMAKE_BUILD_TYPE=Release -DEXTRA_TESTS=OFF
make -j4
make test
make package
