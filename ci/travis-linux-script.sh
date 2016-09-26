#!/bin/bash

source /opt/qt57/bin/qt57-env.sh

set -ev

#qtchooser -qt=qt$QT -run-tool=qmake
qmake
make
