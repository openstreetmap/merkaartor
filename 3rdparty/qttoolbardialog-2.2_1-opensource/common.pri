infile(config.pri, SOLUTIONS_LIBRARY, yes): CONFIG += qttoolbardialog-uselib
TEMPLATE += fakelib
QTTOOLBARDIALOG_LIBNAME = $$qtLibraryTarget(QtSolutions_ToolbarDialog-2.2)
TEMPLATE -= fakelib
QTTOOLBARDIALOG_LIBDIR = $$PWD/lib
unix:qttoolbardialog-uselib:!qttoolbardialog-buildlib:QMAKE_RPATHDIR += $$QTTOOLBARDIALOG_LIBDIR
