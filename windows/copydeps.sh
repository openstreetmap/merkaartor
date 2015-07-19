#!/bin/sh

cp `cygcheck.exe binaries/bin/merkaartor.exe  | grep mingw.*dll$ | sed -e 's/^ *//' | sort -u` ./binaries/bin/
