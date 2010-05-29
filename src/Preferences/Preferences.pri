INCLUDEPATH += Preferences
DEPENDPATH += Preferences

contains(LIBPROXY, 1) {
    DEFINES += USE_LIBPROXY
    LIBS += -lproxy
}

HEADERS +=  \
    MerkaartorPreferences.h \
    PreferencesDialog.h \
    WMSPreferencesDialog.h \
    TMSPreferencesDialog.h \
    ProjPreferencesDialog.h \
    ProjectionsList.h \
    FilterList.h \
    WmsServersList.h \
    TmsServersList.h \
    BookmarksList.h \
    FilterPreferencesDialog.h

SOURCES +=  \
    MerkaartorPreferences.cpp \
    PreferencesDialog.cpp \
    WMSPreferencesDialog.cpp \
    TMSPreferencesDialog.cpp \
    ProjPreferencesDialog.cpp \
    ProjectionsList.cpp \
    FilterList.cpp \
    WmsServersList.cpp \
    TmsServersList.cpp \
    BookmarksList.cpp \
    FilterPreferencesDialog.cpp

FORMS +=  \
    PreferencesDialog.ui \
    WMSPreferencesDialog.ui \
    TMSPreferencesDialog.ui \
    ProjPreferencesDialog.ui \
    FilterPreferencesDialog.ui
