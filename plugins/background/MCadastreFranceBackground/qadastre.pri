DEPENDPATH += $$PWD/qadastre
INCLUDEPATH += $$PWD/qadastre

QT += network webkit
SOURCES +=  \
    city.cpp \
    tile.cpp \
    searchdialog.cpp \
    cadastrewrapper.cpp
HEADERS += \
    city.h \
    tile.h \
    searchdialog.h \
    cadastrewrapper.h
FORMS +=  \
    searchdialog.ui
