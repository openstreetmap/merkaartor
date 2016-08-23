#!/bin/sh

# Copy Merkaartor and it's binary dependencies
cp `cygcheck.exe binaries/bin/merkaartor.exe  | grep mingw.*dll$ | sed -e 's/^ *//' | sort -u` ./binaries/bin/


# Run Windeployqt, mostly to copy all the plugins required by qt, and some more libs
# The ICU hack is there due to a bug in windeployqt, that won't find the lib
# with "lib" prefix. We later delete it not to have it twice.
ICU=`echo /mingw64/bin/libicudt[0-9][0-9].dll`
cp "$ICU" "${ICU/libicudt/icudt}"
ICU=`echo /mingw64/bin/libicuuc[0-9][0-9].dll`
cp "$ICU" "${ICU/libicudt/icuuc}"
windeployqt binaries/bin/merkaartor.exe
rm binaries/bin/icudt[0-9][0-9].dll
rm binaries/bin/icuuc[0-9][0-9].dll

# Copy translations, qt ones are already copied by windeployqt
mkdir binaries/bin/translations
cp translations/*.qm binaries/bin/translations/
