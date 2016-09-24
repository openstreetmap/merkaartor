CONFIG += debug_and_release
TEMPLATE = subdirs
SUBDIRS += src \
    plugins

equals(QT_MAJOR_VERSION, 4) {
    warning("!!! Using Qt 4.x !!!")
    warning(" Support for Qt 4.x will be discontinued soon. Please, use Qt 5.3 or newer to compile/run Merkaartor.")
}
