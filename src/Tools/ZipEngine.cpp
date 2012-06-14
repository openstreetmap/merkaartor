#include "ZipEngine.h"

#include <QUrl>
#include <QDebug>

QAbstractFileEngine* ZipEngineHandler::create(const QString& fileName) const {
    QUrl u(fileName);
    QString path;
    if (u.scheme() == "file")
        path = u.toLocalFile();
    else
        path=fileName;

    while(path.endsWith("/"))
        path.chop(1);
    QRegExp rx("(.*\\.(?:zip|msz))/(\\S\\S*)");
    if(!rx.exactMatch(path))
        return 0;
    QString archivePath = rx.cap(1);
    QString name = rx.cap(2);
    static QMap<QString,ZipArchive> cache;
    ZipArchive archive = cache[archivePath];
    if(!archive.handle) {
        archive.handle = new QuaZip(archivePath);
        if(!archive.handle)
            return 0;
        if (!archive.handle->open(QuaZip::mdUnzip))
            return 0;
        int n = archive.handle->getEntriesCount();
        archive.handle->goToFirstFile();
        for(int i=0;i<n;i++) {
            archive.files[archive.handle->getCurrentFileName()]=i;
            archive.handle->goToNextFile();
        }
        cache[archivePath] = archive;
    }
    int index = archive.files.value(name,-1);
    if(index<0) {
        name+='/';
        index=archive.files.value(name,-1);
    }
    return index>=0 ? new ZipEngine(archive,path,name,index) : 0;
}

ZipEngine::ZipEngine(ZipArchive archive,QString path, QString name,int index)
    : archive(archive), path(path), name(name), index(index), flags(ExistsFlag) {
    if(name.endsWith('/')) {
        flags|=DirectoryType;
        for(QMap<QString,int>::const_iterator i = ++archive.files.lowerBound(name)
            ;i!=archive.files.constEnd() && i.key().startsWith(name);++i)
            files << i.key().mid(name.length());
    } else {
        flags |= FileType;
        QuaZipFileInfo  fi;
        archive.handle->setCurrentFile(archive.files.key(index));
        if (archive.handle->getCurrentFileInfo(&fi)) {
            fileSize = fi.uncompressedSize;
        }
    }
}

bool ZipEngine::open( QIODevice::OpenMode mode)
{
    file = new QuaZipFile(archive.handle);
    archive.handle->setCurrentFile(archive.files.key(index));
    if (!file->open(QIODevice::ReadOnly)) {
        delete file;
        return false;
    }
    return true;
}

qint64 ZipEngine::read( char * data, qint64 maxlen )
{
    return file->read(data,maxlen);
}

bool ZipEngine::close()
{
    file->close(); delete file; return true;
}

QString ZipEngine::fileName(QAbstractFileEngine::FileName file) const
{
    switch (file) {
    case QAbstractFileEngine::DefaultName:
    case QAbstractFileEngine::AbsoluteName:
    case QAbstractFileEngine::CanonicalName:
        return path;
    case QAbstractFileEngine::BaseName:
        return path.section("/",-1);
    case QAbstractFileEngine::PathName:
    case QAbstractFileEngine::AbsolutePathName:
    case QAbstractFileEngine::CanonicalPathName:
        return path.section("/",0,-2);
    }
}

ZipEngineHandler::ZipEngineHandler()
{
}
