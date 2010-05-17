//***************************************************************
// CLass: NavitZip
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//******************************************************************

#ifndef NAVITZIP_H
#define NAVITZIP_H

#include <QHash>
#include <QByteArray>

class QFile;

#define FILE_MAGIC 0x04034b50
#define DIR_HEADER_MAGIC 0x02014b50
#define DIR_FOOTER_MAGIC 0x06054b50

struct header2 {
    QString filename;
    QByteArray extra;
    QString comment;
};

struct fileHeader1 {
    quint32 magic; // 0x04034b50
    quint16 minVer;
    quint16 flag;
    quint16 compressionMethod;
    quint16 modTime;
    quint16 modDate;
    quint32 crc32;
    quint32 sizeCompressed;
    quint32 sizeUncompressed;
    quint16 filenameLen;
    quint16 extraLen;
} __attribute__ ((packed));

struct dirHeader {
    quint32 magic; // 0x02014b50
    quint16 creator;
    quint16 minVer;
    quint16 flag;
    quint16 compressionMethod;
    quint16 modTime;
    quint16 modDate;
    quint32 crc32;
    quint32 sizeCompressed;
    quint32 sizeUncompressed;
    quint16 filenameLen;
    quint16 extraLen;
    quint16 commentLen;
    quint16 diskNumber;
    quint16 internalAttributes;
    quint32 externalAttributes;
    quint32 relativeFileOffset;
} __attribute__ ((packed));

struct dirEntry {
    quint32 offset;
//    fileHeader1 hdr1;
//    header2 hdr2;
};

struct dirExtension {
    quint16 tag;
    quint16 size;
    quint64 zipofst;
} __attribute__ ((packed));

struct dirFooter {
    quint32 magic; // 0x06054b50
    quint16 diskNumber;
    quint16 dirDisk;
    quint16 numDirRecordsOnDisk;
    quint16 totDirRecords;
    quint32 directorySize;
    quint32 directoryOffset;
    quint16 zipCommentLen;
    char zipComment[0];
} __attribute__ ((packed));

class NavitZip
{
public:
    NavitZip();
    ~NavitZip();

    void* setZip(QString fn);
    bool setCurrentFile(int aIndex);
    QByteArray readCurrentFile();

public:
    struct fileHeader1 curFile;
    quint32 curOffset;
    quint32 indexNum;

protected:
    void initialize();

private:
    QFile* zipFile;
    void* zipMem;
    long zipLen;
    struct dirFooter footer;

    QHash < quint32, dirEntry > directory;
};

#endif // NAVITZIP_H
