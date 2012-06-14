include(QFatFs/QFatFs.pri)

INCLUDEPATH += $$MERKAARTOR_SRC_DIR/Tools
DEPENDPATH += $$MERKAARTOR_SRC_DIR/Tools

HEADERS += ZipEngine.h
SOURCES += ZipEngine.cpp

isEmpty(MOBILE) {
  #Header files
  HEADERS += \
      ActionsDialog.h

  #Source files
  SOURCES += \
      ActionsDialog.cpp
}


