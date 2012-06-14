#ifndef QFAT_H
#define QFAT_H

#include <QtCore>

#define FAT_FAT_TYPE quint16
#define FAT_FAT_TYPE_NOVALUE 0xffff

// Big fat: max 512Mb
//#define FAT_CLUSTER_COUNT 65535
//#define FAT_CLUSTER_SIZE 8192

//Small Fat: max 10Mb
//#define FAT_CLUSTER_COUNT 5120
//#define FAT_CLUSTER_SIZE 2048

struct FatHeader
{
    char magic[4]; // "QFAT"
    quint16 cluster_count;
    quint16 cluster_size;
};

#define FLAG_FILE 0x20
#define FLAG_FOLDER 0x10

enum FatError
{
    FatNoError,
    FatSysError,
    FatNotFatFile,
    FatNotOpen,
    FatFileNotFound,
    FatDirNotFound,
    FatDirNotEmpty,
    FatDirAlreadyExists,
    FatOutOfSpace
};

struct FatTocEntry
{
    FatTocEntry() : flags(0) {}

    quint8 flags;
    quint32 creationTimestamp;
    quint32 modificationTimestamp;
    quint32 size;
    FAT_FAT_TYPE startCluster;
    QString name;
};

typedef QList<FatTocEntry> FatTocEntries;

QDataStream &operator<<(QDataStream &ds, const FatTocEntry &toc);
QDataStream &operator>>(QDataStream &ds, FatTocEntry &toc);

class QFat
{
    friend class QFatEngine;

public:
    QFat(const QString& filename, quint16 clusterCount=5120, quint16 clusterSize=2048);

    FatError open();
    void close();
    FatError checkAndCreate(FAT_FAT_TYPE clusterNum);
    bool isOpen();

    FAT_FAT_TYPE findFreeCluster();
    void eraseData(FAT_FAT_TYPE clusterNum);
    FAT_FAT_TYPE writeData(const QByteArray &data, FAT_FAT_TYPE cluster=FAT_FAT_TYPE_NOVALUE);
    QByteArray readData(FAT_FAT_TYPE clusterNum, qint32 maxSize=-1);

    FatTocEntries readTocData(FAT_FAT_TYPE clusterNum);
    FAT_FAT_TYPE writeTocData(const FatTocEntries& toc, FAT_FAT_TYPE cluster);

    FatError setCurrentTocs(const QString& path);
    FatError getToc(const QString &path, const QString& name, FatTocEntry& toc);
    FatError getToc(const QString& filename, FatTocEntry& toc);
    FatError getTocEntries(const QString &repath, FatTocEntries &tocs);
    FatError addToc(const QString &filename, const FatTocEntry& toc);
    FatError addToc(const QString &path, const QString& name, const FatTocEntry &toc);
    FatError deleteToc(const QString &filename);
    FatError deleteToc(const QString &path, const QString& name);

    FatError makeDir(const QString& path);
    FatError makeDirRecursive(const QString &reqpath);

    FatError removeDir(const QString &reqpath);
    FatError removeDirRecursive(const QString &reqpath);
    FatError removeFile(const QString &reqpath);

    QString status();
    QString statusToc(const QString& path, int indent);
    QString fileName() const { return m_filename; }

    void writeFat();
protected:
    QString m_filename;
    QFile* m_fatFile;
    QDataStream m_ds;

    quint16 m_clusterCount;
    quint16 m_clusterSize;
    QList<FAT_FAT_TYPE> m_fat;
    quint64 m_startOfData;

    FatTocEntries m_rootToc;
    FatTocEntries m_curTocs;
    QString m_curTocsPath;
    FAT_FAT_TYPE m_curTocsCluster;
};

#endif // QFAT_H
