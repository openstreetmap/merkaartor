#!/bin/sh

set -ev

git tag
QMAKE=`find /usr/local -name "qmake" | head -n 1`
$QMAKE SPATIALITE=0
make

find ./binaries

cd binaries/bin
mv plugins merkaartor.app/Contents/

which hdiutil

DEPLOY=`find /usr/local -name "macdeployqt" | head -n 1`
$DEPLOY merkaartor.app -verbose=3

du -s -h merkaartor.app
hdiutil create "merkaartor.dmg" -srcfolder merkaartor.app -format UDZO -volname merkaartor -verbose -size 130m

VERSION=`git describe --tags`
mv merkaartor.dmg "merkaartor-${VERSION}-qt${QT}.dmg"

# Prepare deployment description
cat > deploy.json <<EOF
{
    "package": {
        "name": "Merkaartor",
        "repo": "nightly",
        "subject": "krakonos"
    },

    "version": {
        "name": "$VERSION"
    },

    "files":
        [
        {"includePattern": "binaries/bin/(merkaartor-.*.dmg)", "uploadPattern":"\$1"}
        ],
    "publish": true
}
EOF
