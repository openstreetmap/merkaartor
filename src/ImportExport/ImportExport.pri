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
    ImportExportOSC.h \
    ImportExportCSV.h \
    ImportExportOsmBin.h \
    ImportExport/ImportCSVDialog.h

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
    ImportExportOSC.cpp \
    ImportExportCSV.cpp \
    ImportExportOsmBin.cpp \
    ImportExport/ImportCSVDialog.cpp

FORMS += \
    ExportDialog.ui \
    ImportExport/ImportCSVDialog.ui

contains (GDAL, 1) {
    HEADERS += \
        ImportExportSHP.h
    SOURCES += \
        ImportExportSHP.cpp
}
