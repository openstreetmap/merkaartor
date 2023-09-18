#!/bin/bash

source /opt/qt${QT_PREFIX}/bin/qt${QT_PREFIX}-env.sh

set -ev

# Create the build directory either inside the git dir, or in /tmp if we are testing snapshot.
if [ $SNAPSHOT -eq 0 ]; then
	mkdir build && cd build
elif [ $SNAPSHOT -eq 1 ]; then
	git archive --prefix=merkaartor-snapshot/ --output /tmp/merkaartor-snapshot.tar HEAD
	cd /tmp
	tar -xf merkaartor-snapshot.tar
	mkdir merkaartor-snapshot/build && cd merkaartor-snapshot/build
else
	echo "Unknown SNAPSHOT value. Specify SNAPSHOT=0 to use git repo directly, SNAPSHOT=1 to use snapshot archive."
fi

# Note: We need to specify the system cmake, as travis has older version in PATH before the system paths for some reason.
/usr/bin/cmake .. -DCMAKE_BUILD_TYPE=Release -DEXTRA_TESTS=OFF -DCMAKE_UNITY_BUILD=${CMAKE_UNITY_BUILD}
make -j3
QT_QPA_PLATFORM=offscreen ctest --verbose
