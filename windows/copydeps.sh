#!/bin/sh

# Copy Merkaartor and it's binary dependencies
cp `cygcheck.exe binaries/bin/merkaartor.exe  | grep mingw.*dll$ | sed -e 's/^ *//' | sort -u` ./binaries/bin/

# Copy qwindows.dll, as it's loaded dynamically at runtime and not discovered by cygcheck
mkdir binaries/bin/platforms
cp /mingw*/share/qt5/plugins/platforms/qwindows.dll binaries/bin/platforms/

# Copy translations
mkdir binaries/bin/translations
cp translations/*.qm binaries/bin/translations/
cp /mingw*/share/qt5/translations/q*.qm binaries/bin/translations/
