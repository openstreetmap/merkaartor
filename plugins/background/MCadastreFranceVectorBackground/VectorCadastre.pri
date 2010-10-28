QT       += network

DEPENDPATH += $$PWD/VectorCadastre
INCLUDEPATH += $$PWD/VectorCadastre

SOURCES += \
    cadastrewrapper.cpp \
    graphicproducer.cpp \
    clippedpathitem.cpp \
    pdfgraphicview.cpp \
    cadastredownloaddialog.cpp

HEADERS  += \
    cadastrewrapper.h \
    graphicproducer.h \
    clippedpathitem.h \
    pdfgraphicview.h \
    cadastredownloaddialog.h

FORMS    += \
    cadastredownloaddialog.ui

LIBS += -lpodofo
