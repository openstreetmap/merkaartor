#ifndef ZIPENGINE_H
#define ZIPENGINE_H

#include <QAbstractFileEngine>
#include <QMap>

// From et-map-editor by Matthias Fauconneau (https://gitorious.org/et-map-editor)

#include <quazip.h>
#include <quazipfile.h>

class ZipIterator : public QAbstractFileEngineIterator {
public:
    ZipIterator(QDir::Filters filters, const QStringList &nameFilters, QStringList files) :
        QAbstractFileEngineIterator(filters,nameFilters), files(files), index(-1) {}
    bool hasNext() const { return index < files.size() - 1; }
    QString next() { if (!hasNext()) return QString(); index++; return currentFilePath(); }
    QString currentFileName() const { return files[index]; }
private:
    QStringList files;
    int index;
};

struct ZipArchive {
    QuaZip* handle;
    QMap<QString,int> files;
};

class ZipEngineHandler : public QAbstractFileEngineHandler {
public:
    ZipEngineHandler();
    QAbstractFileEngine* create(const QString& fileName) const;
};

class ZipEngine : public QAbstractFileEngine {
public:
    ZipEngine(ZipArchive,QString,QString,int);
    bool open( QIODevice::OpenMode );
    qint64 read( char * data, qint64 maxlen );
    bool close();
    qint64 size() const { return fileSize; }
    FileFlags fileFlags(FileFlags) const { return flags; }
    QString fileName(QAbstractFileEngine::FileName file) const;
    Iterator* beginEntryList(QDir::Filters filters, const QStringList &filterNames) {
        return new ZipIterator(filters, filterNames, files);
    }
private:
    ZipArchive archive;
    QString path;
    QString name;
    int index;
    QuaZipFile* file;
    QStringList files;
    FileFlags flags;
    qint64 fileSize;
};

#endif // ZIPENGINE_H
