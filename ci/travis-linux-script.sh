#!/bin/bash

source /opt/qt${QT_PREFIX}/bin/qt${QT_PREFIX}-env.sh

set -ev

mkdir build && cd build
# Note: We need to specify the system cmake, as travis has older version in PATH before the system paths for some reason.
/usr/bin/cmake .. -DCMAKE_BUILD_TYPE=Release
make -j3
QT_QPA_PLATFORM=offscreen ctest --verbose
