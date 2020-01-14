#!/bin/bash

source /opt/qt${QT_PREFIX}/bin/qt${QT_PREFIX}-env.sh

set -ev

#qtchooser -qt=qt$QT -run-tool=qmake
qmake
make -j3
