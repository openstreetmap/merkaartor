#!/bin/sh

set -ev

brew update

if [ $QT == 4 ]; then
    QT_PKG=qt
else
    QT_PKG=homebrew/versions/qt52
fi

# Uninstall ALL the GDAL. By default, there are more versions and macdeployqt
# tried to embed an older one, that relies upon libspatialite.5, that is no
# longer installed.
brew uninstall --force gdal

# To get GDAL 2.0: http://gis.stackexchange.com/questions/155403/install-gdal-2-0-on-a-macosx
#brew install gdal --HEAD

# Gdal and proj are already up to date, libspatialite is probably required by
# one of them and that screws macdeployqt later on...
brew install Dylibbundler gdal $QT_PKG
brew link --force $QT_PKG

find /usr/local -name "qmake*"
find /usr/local -name "proj_api.h"
