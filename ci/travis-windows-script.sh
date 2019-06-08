#!/bin/bash -x

set -e

qmake.exe -r
make -j8 release

lrelease src/src.pro
sh windows/copydeps.sh
makensis.exe windows/installer.nsi
#windows/upload-to-bintray.pl windows/merkaartor-*.exe

# Prepare deployment description
VERSION=`git describe --tags`
cat > binaries/bin/deploy.json <<EOF
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
        {"includePattern": "windows/(merkaartor-.*.exe)", "uploadPattern":"\$1"}
        ],
    "publish": true
}
EOF
