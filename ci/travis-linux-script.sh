#!/bin/bash

source /opt/qt59/bin/qt59-env.sh

set -ev

#qtchooser -qt=qt$QT -run-tool=qmake
qmake CONFIG+=release PREFIX=/usr
make -j$(nproc)
make INSTALL_ROOT=appdir -j$(nproc) install ; find src/appdir/
wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
export VERSION=$(git rev-parse --short HEAD) # linuxdeployqt uses this for naming the file
( cd src ; ../linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -appimage)
find src/appdir -executable -type f -exec ldd {} \; | grep " => /usr" | cut -d " " -f 2-3 | sort | uniq
# curl --upload-file src/Merkaartor*.AppImage https://transfer.sh/Merkaartor-git.$(git rev-parse --short HEAD)-x86_64.AppImage
wget -c https://github.com/probonopd/uploadtool/raw/master/upload.sh
bash upload.sh src/Merkaartor*.AppImage*
