INCLUDEPATH += Sync
DEPENDPATH += Sync

HEADERS += \
	SyncOSM.h \
	DownloadOSM.h \
	DirtyList.h \
    Sync/DirtyListExecutorOSC.h

SOURCES += \
	SyncOSM.cpp \
	DownloadOSM.cpp \
	DirtyList.cpp \
    Sync/DirtyListExecutorOSC.cpp

FORMS += \
	SyncListDialog.ui
