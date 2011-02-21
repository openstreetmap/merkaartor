INCLUDEPATH += $$MERKAARTOR_SRC_DIR/Tools
DEPENDPATH += $$MERKAARTOR_SRC_DIR/Tools

isEmpty(MOBILE) {
  #Header files
  HEADERS += \
      ActionsDialog.h

  #Source files
  SOURCES += \
      ActionsDialog.cpp
}


