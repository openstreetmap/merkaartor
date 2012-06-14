#ifndef QFATFS_H
#define QFATFS_H

#include <QtCore>

#include "QFat.h"
#include "QFatFile.h"

class FatIterator : public QAbstractFileEngineIterator
{
public:
    FatIterator(QDir::Filters filters, const QStringList &nameFilters, QFat* fat, const QString& path, const QString& name);

    bool hasNext() const;
    QString next();
    QString currentFileName() const;

private:
    QFat* m_fat;
    QString m_fatpath;
    QString m_name;
    QString m_curPath;

    FatTocEntries m_tocs;
    int m_curIndex;

protected:
    int getNextIndex() const;

};

class QFatFsHandler : public QAbstractFileEngineHandler {
public:
    QFatFsHandler(quint16 clusterCount=5120, quint16 clusterSize=2048)
        : m_clusterCount(clusterCount), m_clusterSize(clusterSize) {}
    ~QFatFsHandler();
    QAbstractFileEngine* create(const QString& fileName) const;

private:
    quint16 m_clusterCount;
    quint16 m_clusterSize;

    mutable QMutex mutex;

};

class QFatEngine : public QAbstractFileEngine
{
public:
    explicit QFatEngine(QFat* fat, const QString& path, const QString& name);

    bool open(QIODevice::OpenMode mode);
    qint64 read(char *data, qint64 maxlen);
    qint64 readLine ( char * data, qint64 maxlen );
    qint64 write (const char * data, qint64 len);
    bool close();
    bool remove();
    bool flush ();
    bool setSize ( qint64 /*size*/ ) { return true; }

    QString fileName(FileName file) const;
    Iterator* beginEntryList(QDir::Filters filters, const QStringList &filterNames);

    bool caseSensitive () const { return true; }
    qint64 size() const { return m_toc.size; }
    QDateTime fileTime ( FileTime time ) const;
    FileFlags fileFlags(FileFlags) const { return m_flags; }
    bool supportsExtension ( Extension  ) const { return QAbstractFileEngine::AtEndExtension; }
    bool extension ( Extension extension, const ExtensionOption * option = 0, ExtensionReturn * output = 0 );
    bool isSequential () const { return false; }

    bool mkdir ( const QString & dirName, bool createParentDirectories ) const;
    bool rmdir ( const QString & dirName, bool recurseParentDirectories ) const;

    qint64 pos () const { return fatFile->pos(); }
    bool seek ( qint64 offset );
signals:

public slots:

private:
    QFat* m_fat;
    QString m_fatpath;
    QString m_name;
    FileFlags m_flags;
    FatTocEntry m_toc;

    QFatFile* fatFile;
};

#endif // QFATFS_H
