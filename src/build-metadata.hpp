#ifndef __BUILD_METADATA_HPP__
#define __BUILD_METADATA_HPP__

#include <QString>

class BuildMetadata {
    public:
        static char const * const VERSION;
        static char const * const REVISION;
        static char const * const PRODUCT;
        static const int VERSION_MAJOR;
        static const int VERSION_MINOR;
        static const int VERSION_PATCH;
        static const int VERSION_BUILD;
        static QString GetShareDir();
        static QString GetLibDir();
    protected:
        static char const * const SHARE_DIR;
        static char const * const LIB_DIR;
};

#endif
