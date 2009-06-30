VERSION="0.14"
REVISION="-pre1"

# NODEBUG=1             - no debug target
# TRANSDIR_MERKAARTOR - translations directory for merkaartor
#TRANSDIR_MERKAARTOR=c:/home/cbrowet/src/merkaartor_trunk
# TRANSDIR_SYSTEM     - translations directory for Qt itself
# OUTPUT_DIR          - base directory for local output files
# PREFIX              - base prefix for installation
symbian:NOUSEWEBKIT=1
# NOUSEWEBKIT=1         - disable use of WebKit (Yahoo adapter)
# OSMARENDER=1        - enable osmarender (requires libxml2 / libxslt)
symbian:MOBILE=1
# MOBILE=1    	      - enable MOBILE
# GEOIMAGE=1          - enable geotagged images (requires exiv2)
# GPSD=1              - use gpsd as location provider
# NVIDIA_HACK=1       - used to solve nvidia specific slowdown
# GDAL=1    	      - enable GDAL (for, e.g., shapefile import; requires libgdal)
# FORCE_CUSTOM_STYLE=1 - force custom style (recommended on Linux until the "expanding dock" is solved upstream)
