#!/bin/bash

source /opt/qt${QT_PREFIX}/bin/qt${QT_PREFIX}-env.sh

set -ev

mkdir build && cd build
cmake ..
make -j3
QT_QPA_PLATFORM=offscreen ctest --verbose
