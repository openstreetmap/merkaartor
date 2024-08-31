#!/bin/sh

set -ev

mkdir build && cd build
conda info
echo $PATH
ls /Users/runner/miniconda3/condabin
cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DEXTRA_TESTS=OFF
make -j4
make test
for i in {0..10}; do
    # make package seems to fail due to race conditions:
    # https://github.com/actions/runner-images/issues/7522
    # https://gitlab.kitware.com/cmake/cmake/-/issues/25671
    make package && break
    echo "Failed attempt #${i}. Retrying..."
done
echo "Passed on attempt #${i}."
