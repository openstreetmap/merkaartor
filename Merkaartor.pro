CONFIG += debug_and_release
TEMPLATE = subdirs
SUBDIRS += src \
    plugins

equals(QT_MAJOR_VERSION, 4) {
    warning("!!! Using Qt 4.x !!!")
    warning(" Support for Qt 4.x will be discontinued soon. Please, use Qt 5.3 or newer to compile/run Merkaartor.")
}

warning("qmake support is deprecated, please use cmake build system whenever possible. qmake support will be removed in 0.20.0 release.")
