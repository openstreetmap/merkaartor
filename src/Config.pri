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
# GDAL=1    	      - enable GDAL (for, e.g., shapefile import; requires libgdal)
# USE_BUILTIN_BOOST=1 - use the Boost version (1.38) from Merkaartor rather than the system one (ony on Linux)
# GPSDLIB=1           - use gpsd libgps or libQgpsmm for access to a gpsd server

isEmpty(VERSION): VERSION="0.16"
isEmpty(SVNREV) {
	!contains(RELEASE,1) {
    	#SVNREV = $$system($$escape_expand(svn info \"http://svn.openstreetmap.org/applications/editors/merkaartor/\" | sed -n \"s/Last Changed Rev: \\([0-9]\\+\\)/\\1/p\"))
    	win32 {
        	system(echo $${LITERAL_HASH}define SVNREV \\ > revision.h && svnversion >> revision.h)
    	} else {
        	system('echo -n "$${LITERAL_HASH}define SVNREV " > revision.h && svnversion >> revision.h')
    	}
    	REVISION="-svn"
	} else {
    	DEFINES += RELEASE
    	REVISION=""
    	SVNREV=""
	}
} else {
    win32 {
       	system(echo $${LITERAL_HASH}define SVNREV $${SVNREV} > revision.h )
    } else {
       	system('echo -n "$${LITERAL_HASH}define SVNREV $${SVNREV}" > revision.h')
    }
	REVISION="-svn"
}

