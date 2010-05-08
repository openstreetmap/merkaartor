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

#include "NavitZip.h"

#include <QDebug>
#include <QFile>

#include <zlib.h>

NavitZip::NavitZip()
    : indexNum(-1)
    , zipFile(0)
    , zipMem(0)
    , zipLen(0)
{
}

NavitZip::~NavitZip()
{
    if (zipFile)
        delete zipFile;
}

void* NavitZip::setZip(QString fn)
{
    if (zipFile)
        delete zipFile;
    zipFile = new QFile(fn);
    if (!zipFile->open(QIODevice::ReadOnly)) {
        return NULL;
    }

    zipLen = zipFile->size();

    zipMem = NULL;
    if (zipLen)
        zipMem = zipFile->map(0, zipLen);
    zipFile->close();

    if (!zipMem)
        return NULL;

    memcpy((void*)&footer, (void*)((quint32)zipMem + zipLen - sizeof(struct dirFooter)), sizeof(struct dirFooter));
    if (footer.magic != DIR_FOOTER_MAGIC) {
        delete zipFile;
        zipFile = NULL;
        return NULL;
    }

    initialize();

    return zipMem;
}

void NavitZip::initialize()
{
    qDebug() << "eof: " << (quint32)zipMem + zipLen;
    int num = 0;
    struct dirHeader* cdhdr;
    struct dirHeader* dirstart = (struct dirHeader*) ((quint32)zipMem + footer.directoryOffset);
    for (cdhdr=dirstart; (quint32)cdhdr<(quint32)dirstart+footer.directorySize;) {
        Q_ASSERT(cdhdr->magic == 0x02014b50);
        if (cdhdr->sizeUncompressed) {
            struct dirEntry ent;
            ent.offset = cdhdr->relativeFileOffset;
            directory.insert(num, ent);

            char* fn = (char*) ((quint32)cdhdr + sizeof(struct dirHeader));
            if (!strncmp(fn, "index", 5))
                indexNum = num;

        }
        Q_ASSERT((quint32)(cdhdr) + sizeof(struct dirHeader) + cdhdr->filenameLen + cdhdr->extraLen + cdhdr->commentLen < (quint32) (zipMem) + zipLen);
        cdhdr = (struct dirHeader*) ((quint32)cdhdr + sizeof(struct dirHeader) + cdhdr->filenameLen + cdhdr->extraLen + cdhdr->commentLen);
        ++num;
    }

    qDebug() << "actual num: " << num << "; expected num: " << footer.totDirRecords;
}

bool NavitZip::setCurrentFile(int aIndex)
{
    if (!directory.contains(aIndex))
        return false;

    curOffset = directory[aIndex].offset;
    memcpy((void*)&curFile, (void*)((quint32)zipMem + curOffset), sizeof(struct fileHeader1));

    return true;
}

QByteArray NavitZip::readCurrentFile()
{
    QByteArray ba(curFile.sizeUncompressed, 0);
    void* fileStartPtr = (void *)((quint32)zipMem + curOffset + sizeof(struct fileHeader1) + curFile.filenameLen);

    switch (curFile.compressionMethod) {
    case 0:
        memcpy(ba.data(), fileStartPtr, curFile.sizeUncompressed);
        break;
    case 8: {
        z_stream stream;
        int err;

        void* buf = malloc(curFile.sizeCompressed);
        memcpy(buf, fileStartPtr, curFile.sizeCompressed);
        stream.next_in = (Bytef*)buf;
        stream.avail_in = (uInt)curFile.sizeCompressed;
        stream.next_out = (Bytef*)ba.data();
        stream.avail_out = (uInt)curFile.sizeUncompressed;

        stream.zalloc = (alloc_func)0;
        stream.zfree = (free_func)0;

        err = inflateInit2(&stream, -MAX_WBITS);
        if (err != Z_OK) {
            qDebug("Decompression error %d\n", err);
            return QByteArray();
        }

        err = inflate(&stream, Z_FINISH);
        if (err != Z_STREAM_END) {
            inflateEnd(&stream);
            if (err == Z_NEED_DICT || (err == Z_BUF_ERROR && stream.avail_in == 0)) {
                qDebug("Decompression data error %d\n", err);
                return QByteArray();
            }
            qDebug("Decompression error %d\n", err);
            return QByteArray();
        }
//        *destLen = stream.total_out;

        err = inflateEnd(&stream);
        if (err != Z_OK) {
            qDebug("Decompression error %d\n", err);
            return QByteArray();
        }
        break;
    }
    case 99:
    default:
        qDebug("Unsupported compression method %d\n", curFile.compressionMethod);
    }
    return ba;
}
