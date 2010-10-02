# NODEBUG=1             - no debug target
# RELEASE=1             - release target (use with NODEBUG=1)
# TRANSDIR_MERKAARTOR - translations directory for merkaartor
#TRANSDIR_MERKAARTOR=c:/home/cbrowet/src/merkaartor_trunk
# TRANSDIR_SYSTEM     - translations directory for Qt itself
# OUTPUT_DIR          - base directory for local output files
# PREFIX              - base prefix for installation
symbian:NOUSEWEBKIT=1
# NOUSEWEBKIT=1         - disable use of WebKit (Yahoo adapter)
symbian:MOBILE=1
# MOBILE=1    	      - enable MOBILE
# GEOIMAGE=1          - enable geotagged images (requires exiv2)
# NVIDIA_HACK=1       - used to solve nvidia specific slowdown
# PROJ=1              - use PROJ4 library for projections (requires proj4)
# GDAL=1    	      - enable GDAL (for, e.g., shapefile import; requires libgdal)
# USE_BUILTIN_BOOST=1 - use the Boost version (1.38) from Merkaartor rather than the system one (ony on Linux)
# GPSDLIB=1           - use gpsd libgps or libQgpsmm for access to a gpsd server
# ZBAR=1              - use the ZBAR library to extract coordinates from barcode (>0.10 required)
# LIBPROXY=1          - use the libproxy library to find the system proxy
# PORTABLE=1          - build merkaartor as a portable app (all files in app dir)

isEmpty(VERSION): VERSION="0.17"

contains(RELEASE,1) {
    DEFINES += RELEASE
    SVNREV="release"
} else {
    isEmpty(SVNREV) {
        SVNREV = $$system(git describe --tags)
        REVISION=""
    } else {
        REVISION="-git"
    }
}

win32 {
    system(echo $${LITERAL_HASH}define SVNREV $${SVNREV} > revision.h )
} else {
    system('echo -n "$${LITERAL_HASH}define SVNREV $${SVNREV}" > revision.h')
}
