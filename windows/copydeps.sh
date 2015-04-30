#!/bin/sh

cp `cygcheck.exe binaries/bin/merkaartor.exe  | grep mingw64.*dll$ | sed -e 's/^ *//' | sort -u` ./binaries/bin/
