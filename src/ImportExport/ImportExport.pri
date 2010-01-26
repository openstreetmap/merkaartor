INCLUDEPATH += ImportExport
DEPENDPATH += ImportExport

#Header files
HEADERS += \
	ExportOSM.h \
	ImportGPX.h \
	ImportNGT.h \
	ImportOSM.h \
	ImportNGT.h \
	IImportExport.h \
	ImportNMEA.h \
	ExportGPX.h \
	ImportExportKML.h \
	ImportExportOsmBin.h

#Source files
SOURCES += \
	ExportOSM.cpp \
	ImportGPX.cpp \
	ImportOSM.cpp \
	ImportNGT.cpp \
	IImportExport.cpp \
	ImportNMEA.cpp \
	ExportGPX.cpp \
	ImportExportKML.cpp \
	ImportExportOsmBin.cpp

FORMS += \
	ExportDialog.ui

contains (GDAL, 1) {
	HEADERS += \
		ImportExportSHP.h
	SOURCES += \
		ImportExportSHP.cpp
}
