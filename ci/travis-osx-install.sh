#!/bin/bash -x

set -ev

#brew update

brew info gdal
brew info proj
brew info exiv2
brew info qt5
brew info cmake


# Uninstall ALL the GDAL. By default, there are more versions and macdeployqt
# tried to embed an older one, that relies upon libspatialite.5, that is no
# longer installed.
#brew uninstall --ignore-dependencies --force gdal exiv2 proj qt5

# To get GDAL 2.0: http://gis.stackexchange.com/questions/155403/install-gdal-2-0-on-a-macosx
#brew install gdal --HEAD

# Gdal and proj are already up to date, libspatialite is probably required by
# one of them and that screws macdeployqt later on...
#
# Moreover, we will ignore the results as homebrew fails as it wishes.  The
# build will fail later anyway if something doesn't install properly.
#brew install Dylibbundler proj exiv2 gdal qt5 || echo "Install might have failed. Ignoring"
brew install exiv2 || echo "Install might have failed. Ignoring"
brew upgrade cmake
#brew link --force qt5 || echo "Link might have failed. Ignoring."
