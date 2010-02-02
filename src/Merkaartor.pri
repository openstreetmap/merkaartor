# MOBILE=1 - enable MOBILE
# GEOIMAGE=1 - enable geotagged images (requires exiv2)
# GPSD=1 - use gpsd as location provider
# NVIDIA_HACK=1 - used to solve nvidia specific slowdown
# GDAL=1 - enable GDAL (for, e.g., shapefile import; requires libgdal)
# FORCE_CUSTOM_STYLE=1 - force custom style (recommended on Linux until the "expanding dock" is solved upstream)
# FORCE_CUSTOM_STYLE=1 - force custom style (recommended on Linux until the "expanding dock" is solved upstream)
# Header files
HEADERS += \
	./IProgressWindow.h \
	./MainWindow.h \
	./Maps/Coord.h \
	./Document.h \
	./Maps/MapTypedef.h \
	./Maps/Painting.h \
	./Maps/Projection.h \
	./Maps/FeatureManipulations.h \
	./MapView.h \
	./PaintStyle/MasPaintStyle.h \
	./PaintStyle/PaintStyle.h \
	./PaintStyle/PaintStyleEditor.h \
	./PaintStyle/TagSelector.h \
	./TagModel.h \
	./Preferences/MerkaartorPreferences.h \
	./Preferences/PreferencesDialog.h \
	./Preferences/WMSPreferencesDialog.h \
	./Preferences/TMSPreferencesDialog.h \
	./Preferences/ProjectionsList.h \
	./Preferences/WmsServersList.h \
	./Preferences/TmsServersList.h \
	./Preferences/BookmarksList.h \
	./Utils/LineF.h \
	./Utils/ShortcutOverrideFilter.h \
	./Utils/SlippyMapWidget.h \
	./Utils/EditCompleterDelegate.h \
	./Utils/PictureViewerDialog.h \
	./Utils/PixmapWidget.h \
	./Utils/SelectionDialog.h \
	./Utils/SvgCache.h \
	./Utils/MDiscardableDialog.h \
	./GotoDialog.h \
	../include/ggl/extensions/gis/projections/impl/geocent.h \
	../interfaces/IMapAdapter.h \
	revision.h

# Source files
SOURCES += \
	./Maps/Coord.cpp \
	./Document.cpp \
	./Maps/Painting.cpp \
	./Maps/Projection.cpp \
	./Maps/FeatureManipulations.cpp \
	./MapView.cpp \
	./PaintStyle/MasPaintStyle.cpp \
	./PaintStyle/PaintStyle.cpp \
	./PaintStyle/PaintStyleEditor.cpp \
	./PaintStyle/TagSelector.cpp \
	./Main.cpp \
	./MainWindow.cpp \
	./TagModel.cpp \
	./Utils/ShortcutOverrideFilter.cpp \
	./Utils/SlippyMapWidget.cpp \
	./Utils/EditCompleterDelegate.cpp \
	./Utils/PictureViewerDialog.cpp \
	./Utils/PixmapWidget.cpp \
	./Utils/SelectionDialog.cpp \
	./Utils/SvgCache.cpp \
	./Utils/MDiscardableDialog.cpp \
	./Preferences/MerkaartorPreferences.cpp \
	./Preferences/PreferencesDialog.cpp \
	./Preferences/WMSPreferencesDialog.cpp \
	./Preferences/TMSPreferencesDialog.cpp \
	./Preferences/ProjectionsList.cpp \
	./Preferences/WmsServersList.cpp \
	./Preferences/TmsServersList.cpp \
	./Preferences/BookmarksList.cpp \
	./GotoDialog.cpp \
	../include/ggl/extensions/gis/projections/impl/geocent.c

# Forms
FORMS += ./AboutDialog.ui \
	./DownloadMapDialog.ui \
	./MainWindow.ui \
	./UploadMapDialog.ui \
	./GotoDialog.ui \
	./MultiProperties.ui \
	./PaintStyle/PaintStyleEditor.ui \
	./Preferences/PreferencesDialog.ui \
	./Preferences/WMSPreferencesDialog.ui \
	./Preferences/TMSPreferencesDialog.ui \
	./Utils/PictureViewerDialog.ui \
	./Utils/SelectionDialog.ui \

# Resource file(s)
RESOURCES += ../Icons/AllIcons.qrc \
	./Utils/Utils.qrc \
	../share/share.qrc

OTHER_FILES += ../CHANGELOG
