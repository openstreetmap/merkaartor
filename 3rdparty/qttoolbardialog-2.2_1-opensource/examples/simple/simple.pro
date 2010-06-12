TEMPLATE = app
DEPENDPATH += .
INCLUDEPATH += .

include(../../src/qttoolbardialog.pri)
# Input
SOURCES += main.cpp \
           mainwindow.cpp
HEADERS += mainwindow.h
RESOURCES += simple.qrc

