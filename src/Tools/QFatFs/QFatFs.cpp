#include "QFatFs.h"

static QMap<QString,QFat*> cache;

QFatFsHandler::~QFatFsHandler()
{
    QMapIterator<QString,QFat*> i(cache);
    while (i.hasNext()) {
        i.next();
        i.value()->close();
        delete i.value();
    }
}

QAbstractFileEngine * QFatFsHandler::create(const QString &fileName) const
{

    QUrl u(fileName);
    if (!u.isValid())
            return NULL;
    if (u.scheme() != "fat")
        return NULL;

    QString fatPath;
    QString path = u.path();
    if (!u.host().isEmpty())
        fatPath = u.host() + u.path();
    else {
        fatPath = u.path();
    }
    if (fatPath.startsWith("/"))
        fatPath.remove(0, 1);

    QString name = u.fragment();

    QMutexLocker locker(&mutex);
    QFat* fat = cache[fatPath];
    if(!fat) {
        fat = new QFat(fatPath, m_clusterCount, m_clusterSize);
        if(!fat) return 0;
        if (fat->open() != FatNoError)
            return 0;
        cache[fatPath] = fat;
    }
    return new QFatEngine(fat,fatPath, name);
}

/*******************************/

FatIterator::FatIterator(QDir::Filters filters, const QStringList &nameFilters, QFat *fat, const QString& fatpath, const QString &name)
    : QAbstractFileEngineIterator(filters,nameFilters)
    , m_fat(fat)
    , m_fatpath(fatpath)
    , m_name(name)

{
    m_curPath = m_name;
    m_fat->getTocEntries(m_name, m_tocs);
    m_curIndex = -1;
}

int FatIterator::getNextIndex() const
{
    for (int i=m_curIndex+1; i<m_tocs.size(); ++i) {
        if (filters() & QDir::AllEntries)
            return i;
        else if (filters() & QDir::Dirs) {
            if (m_tocs[i].flags & FLAG_FOLDER)
                return i;
        } else if (filters() & QDir::Files) {
            if (m_tocs[i].flags & FLAG_FILE)
                return i;
        }
    }
    return m_tocs.size();
}

bool FatIterator::hasNext() const
{
    return (getNextIndex() < m_tocs.size());
}

QString FatIterator::next()
{
    m_curIndex = getNextIndex();
    QString path = "fat:///" + m_fatpath + "#" + m_name;
    if(!path.endsWith("/")) path += "/";
    return path + m_tocs[m_curIndex].name;
}

QString FatIterator::currentFileName() const
{
    return m_tocs[m_curIndex].name;
}

/**************************/

QMutex glob_mutex;

QFatEngine::QFatEngine(QFat* fat, const QString& path, const QString& name)
    : m_fat(fat)
    , m_fatpath(path)
    , m_name(name)
    , m_flags(0xffffffff)

{
    m_flags &= (int)m_fat->m_fatFile->permissions();
    if (m_name.isEmpty() || m_name == "/") {
        m_flags |= RootFlag;
        m_flags |= ExistsFlag;
        m_flags |= DirectoryType;
    } else {
        QMutexLocker locker(&glob_mutex);
        FatError ret = m_fat->getToc(m_name, m_toc);
        if (ret == FatNoError && m_toc.flags) {
            m_flags |= ExistsFlag;
            if (m_toc.flags & FLAG_FILE)
                m_flags |= FileType;
            else if (m_toc.flags & FLAG_FOLDER) {
                m_flags |= DirectoryType;
            }
        } else {
            if(m_name.endsWith('/')) {
                m_flags|= DirectoryType;
            } else {
                m_flags |= FileType;
            }
        }
    }
}

bool QFatEngine::open( QIODevice::OpenMode mode)
{
    QMutexLocker locker(&glob_mutex);

    fatFile = new QFatFile(m_name, m_fat);
    if (!fatFile->open(mode)) {
        delete fatFile;
        return false;
    }
    return true;
}

qint64 QFatEngine::read( char * data, qint64 maxlen )
{
    QMutexLocker locker(&glob_mutex);
    return fatFile->read(data,maxlen);
}

qint64 QFatEngine::readLine(char *data, qint64 maxlen)
{
    QMutexLocker locker(&glob_mutex);
    return fatFile->readLine(data,maxlen);
}

qint64 QFatEngine::write(const char *data, qint64 len)
{
    QMutexLocker locker(&glob_mutex);
    return fatFile->write(data,len);
}

bool QFatEngine::close()
{
    QMutexLocker locker(&glob_mutex);
    fatFile->close();
    delete fatFile;
    return true;
}

QAbstractFileEngine::Iterator * QFatEngine::beginEntryList(QDir::Filters filters, const QStringList &filterNames)
{
    QMutexLocker locker(&glob_mutex);
    return new FatIterator(filters, filterNames, m_fat, m_fatpath, m_name);
}

QDateTime QFatEngine::fileTime(QAbstractFileEngine::FileTime time) const
{
    switch (time) {
    case QAbstractFileEngine::CreationTime:
        return QDateTime::fromSecsSinceEpoch(m_toc.creationTimestamp);
    case QAbstractFileEngine::ModificationTime:
        return QDateTime::fromSecsSinceEpoch(m_toc.modificationTimestamp);
    default:
        return QDateTime();
    }
}

bool QFatEngine::mkdir(const QString &dirName, bool createParentDirectories) const
{
    QUrl u(dirName);
    if (!u.isValid() || u.scheme() != "fat")
        return false;

    bool ret;

    QMutexLocker locker(&glob_mutex);

    if (createParentDirectories)
        ret = (m_fat->makeDirRecursive(u.fragment()) == FatNoError) ? true : false;
    else
        ret = (m_fat->makeDir(u.fragment()) == FatNoError) ? true : false;
    m_fat->writeFat();

    return ret;
}

bool QFatEngine::rmdir(const QString &dirName, bool recurseParentDirectories) const
{
    QUrl u(dirName);
    if (!u.isValid() || u.scheme() != "fat")
        return false;

    bool ret;

    QMutexLocker locker(&glob_mutex);

    if (recurseParentDirectories)
        ret = (m_fat->removeDirRecursive(u.fragment()) == FatNoError) ? true : false;
    else
        ret = (m_fat->removeDir(u.fragment()) == FatNoError) ? true : false;
    m_fat->writeFat();

    return ret;
}


bool QFatEngine::extension(QAbstractFileEngine::Extension extension, const QAbstractFileEngine::ExtensionOption *option, QAbstractFileEngine::ExtensionReturn *output)
{
    switch (extension) {
    case QAbstractFileEngine::AtEndExtension:
        return fatFile->atEnd();
    default:
        return false;
    }
}

bool QFatEngine::remove()
{
    QMutexLocker locker(&glob_mutex);
    if (m_fat->removeFile(m_name) != FatNoError)
        return false;
    m_fat->writeFat();

    return true;
}

QString QFatEngine::fileName(QAbstractFileEngine::FileName file) const
{
    QStringList tokens;
    tokens = m_name.split("/");
    QString name = tokens.takeLast();
    QString path = tokens.join("/");

    switch (file) {
    case QAbstractFileEngine::DefaultName:
    case QAbstractFileEngine::AbsoluteName:
    case QAbstractFileEngine::CanonicalName:
        return "fat:///" + m_fatpath + "#" + m_name;
    case QAbstractFileEngine::BaseName:
        return name;
    case QAbstractFileEngine::PathName:
    case QAbstractFileEngine::AbsolutePathName:
    case QAbstractFileEngine::CanonicalPathName:
        return "fat:///" + m_fatpath + "#" + path;
    }
}

bool QFatEngine::flush()
{
    return true;
}

bool QFatEngine::seek(qint64 offset)
{
    return fatFile->seek(offset);
}


