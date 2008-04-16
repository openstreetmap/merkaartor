TEMPLATE = subdirs
CONFIG += ordered
!gtk-port:CONFIG += qt-port
qt-port {
    lessThan(QT_MINOR_VERSION, 4) {
        !win32-*:SUBDIRS += WebKit/qt/Plugins
    }
}
SUBDIRS += \
        WebCore

