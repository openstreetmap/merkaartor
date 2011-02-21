INCLUDEPATH += $$MERKAARTOR_SRC_DIR/Preferences
DEPENDPATH += $$MERKAARTOR_SRC_DIR/Preferences

contains(LIBPROXY, 1) {
    DEFINES += USE_LIBPROXY
    LIBS += -lproxy
}
HEADERS += MerkaartorPreferences.h \
    PreferencesDialog.h \
    WMSPreferencesDialog.h \
    TMSPreferencesDialog.h \
    ProjectionsList.h \
    FilterList.h \
    WmsServersList.h \
    TmsServersList.h \
    BookmarksList.h \
    FilterPreferencesDialog.h
SOURCES += MerkaartorPreferences.cpp \
    PreferencesDialog.cpp \
    WMSPreferencesDialog.cpp \
    TMSPreferencesDialog.cpp \
    ProjectionsList.cpp \
    FilterList.cpp \
    WmsServersList.cpp \
    TmsServersList.cpp \
    BookmarksList.cpp \
    FilterPreferencesDialog.cpp
FORMS += PreferencesDialog.ui \
    WMSPreferencesDialog.ui \
    TMSPreferencesDialog.ui \
    FilterPreferencesDialog.ui \
    OsmServerWidget.ui

isEmpty(MOBILE) {
  HEADERS += \
     ProjPreferencesDialog.h

  SOURCES += \
     ProjPreferencesDialog.cpp

  FORMS += \
     ProjPreferencesDialog.ui
}
