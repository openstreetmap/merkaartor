INCLUDEPATH += $$MERKAARTOR_SRC_DIR/Sync
DEPENDPATH += $$MERKAARTOR_SRC_DIR/Sync

HEADERS += \
    DownloadOSM.h

SOURCES += \
    DownloadOSM.cpp

!contains(FRISIUS,1) {
  HEADERS += \
    DirtyListExecutorOSC.h \
    DirtyList.h

  SOURCES += \
    DirtyListExecutorOSC.cpp \
    DirtyList.cpp

  FORMS += \
    SyncListDialog.ui
}
