INCLUDEPATH += $$MERKAARTOR_SRC_DIR/ImportExport
DEPENDPATH += $$MERKAARTOR_SRC_DIR/ImportExport

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
    ImportExportCSV.h \
    ImportCSVDialog.h

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
    ImportExportCSV.cpp \
    ImportCSVDialog.cpp

FORMS += \
    ExportDialog.ui \
    ImportCSVDialog.ui

isEmpty(MOBILE) {
  HEADERS += \
      ImportExportGdal.h
  SOURCES += \
      ImportExportGdal.cpp
}

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

!contains(FRISIUS,1) {
    HEADERS += \
        ImportExportOSC.h
    SOURCES += \
        ImportExportOSC.cpp
}
