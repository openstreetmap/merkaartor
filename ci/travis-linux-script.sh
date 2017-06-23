#!/bin/bash

source /opt/qt59/bin/qt59-env.sh

set -ev

#qtchooser -qt=qt$QT -run-tool=qmake
qmake
make
