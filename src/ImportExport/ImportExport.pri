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
    ImportExport/ImportCSVDialog.cpp

FORMS += \
    ExportDialog.ui \
    ImportExport/ImportCSVDialog.ui

HEADERS += \
    ImportExportGdal.h
SOURCES += \
    ImportExportGdal.cpp

contains (PROTOBUF, 1) {
#PBF
HEADERS += \
    ImportExportPBF.h \
    fileformat.pb.h \
    osmformat.pb.h

SOURCES += \
    ImportExportPBF.cpp \
    fileformat.pb.cc \
    osmformat.pb.cc

LIBS += -lz -lbz2 -lprotobuf
}
