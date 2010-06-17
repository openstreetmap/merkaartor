# Header files
HEADERS += NativeRenderDialog.h \
    MapRenderer.h \
    Render/qmyprintpreviewdialog.h

# Source files
SOURCES += NativeRenderDialog.cpp \
    MapRenderer.cpp \
    Render/qmyprintpreviewdialog.cpp

# Forms
FORMS += NativeRenderDialog.ui
QT += svg

RESOURCES += \
    Render/qmyprintdialog.qrc
