include(../common.pri)
INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

qttoolbardialog-uselib:!qttoolbardialog-buildlib {
    LIBS += -L$$QTTOOLBARDIALOG_LIBDIR -l$$QTTOOLBARDIALOG_LIBNAME
} else {
    SOURCES += $$PWD/qttoolbardialog.cpp
    HEADERS += $$PWD/qttoolbardialog.h
    FORMS += $$PWD/qttoolbardialog.ui
    RESOURCES += $$PWD/qttoolbardialog.qrc
}

win32 {
    contains(TEMPLATE, lib):contains(CONFIG, shared):DEFINES += QT_QTTOOLBARDIALOG_EXPORT
    else:qttoolbardialog-uselib:DEFINES += QT_QTTOOLBARDIALOG_IMPORT
}
