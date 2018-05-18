#!/bin/bash -x

set -ev

brew update

if [ $QT == 4 ]; then
    QT_PKG=qt
else
    #QT_PKG=homebrew/versions/qt52
    QT_PKG=qt5
fi

# Uninstall ALL the GDAL. By default, there are more versions and macdeployqt
# tried to embed an older one, that relies upon libspatialite.5, that is no
# longer installed.
brew uninstall --ignore-dependencies --force gdal

# To get GDAL 2.0: http://gis.stackexchange.com/questions/155403/install-gdal-2-0-on-a-macosx
#brew install gdal --HEAD

# Gdal and proj are already up to date, libspatialite is probably required by
# one of them and that screws macdeployqt later on...
#
# Moreover, we will ignore the results as homebrew fails as it wishes.  The
# build will fail later anyway if something doesn't install properly.
brew install Dylibbundler gdal $QT_PKG || echo "Install might have failed. Ignoring"
brew link --force $QT_PKG || echo "Link might have failed. Ignoring."

find /usr/local -name "qmake*"
find /usr/local -name "proj_api.h"
