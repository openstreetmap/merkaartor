INCLUDEPATH += $$MERKAARTOR_SRC_DIR/NameFinder
DEPENDPATH += $$MERKAARTOR_SRC_DIR/NameFinder


SOURCES += xmlstreamreader.cpp \
    httpquery.cpp \
    namefindertablemodel.cpp \
    namefinderwidget.cpp
HEADERS += NameFinderResult.h \
    xmlstreamreader.h \
    httpquery.h \
    namefindertablemodel.h \
    namefinderwidget.h
FORMS += namefinderwidget.ui
