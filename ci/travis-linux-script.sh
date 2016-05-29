#!/bin/sh

set -ev

qtchooser -qt=qt$QT -run-tool=qmake
make
