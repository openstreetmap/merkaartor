#!/bin/sh

set -ev

mkdir build && cd build
cmake .. -G"Unix Makefiles" -DCMAKE_PREFIX_PATH="`brew --prefix qt5`"
make VEROBSE=1 -j4
make VERBOSE=1 package

VERSION=`git describe --tags`

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
        {"includePattern": "build/(merkaartor-.*.dmg)", "uploadPattern":"\$1"},
        {"includePattern": "build/(merkaartor-.*.zip)", "uploadPattern":"\$1"}
        ],
    "publish": true
}
EOF
