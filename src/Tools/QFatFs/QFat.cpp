#include "QFat.h"

#include <QBuffer>

QDataStream &operator<<(QDataStream &ds, const FatTocEntry &toc)
{
    ds << toc.flags;
    ds << toc.creationTimestamp;
    ds << toc.modificationTimestamp;
    ds << toc.size;
    ds << toc.startCluster;
    ds << toc.name;

    return ds;
}

QDataStream &operator>>(QDataStream &ds, FatTocEntry &toc)
{
    ds >> toc.flags;
    ds >> toc.creationTimestamp;
    ds >> toc.modificationTimestamp;
    ds >> toc.size;
    ds >> toc.startCluster;
    ds >> toc.name;

    return ds;
}

QFat::QFat(const QString& filename, quint16 clusterCount, quint16 clusterSize)
    : m_filename(filename)
    , m_clusterCount(clusterCount)
    , m_clusterSize(clusterSize)
    , m_fatFile(0)
{
}

FatError QFat::open()
{
    FatError ret;

    m_fatFile = new QFile(m_filename);
    if (m_fatFile->exists()) {
        if (!m_fatFile->open(QIODevice::ReadWrite))
            return FatSysError;

        char magic[4]; // "QFAT"
        m_fatFile->read((char*)&magic, 4);
        if (magic[0] != 'Q' || magic[1] != 'F' || magic[2] != 'A' || magic[3] != 'T')
            return FatNotFatFile;

        m_ds.setDevice(m_fatFile);
        m_ds >> m_clusterCount;
        m_ds >> m_clusterSize;

        FAT_FAT_TYPE val;
        m_fat.reserve(m_clusterCount);
        for (int i=0; i<m_clusterCount; ++i) {
            m_ds >> val;
            m_fat << val;
        }

        m_startOfData = sizeof(FatHeader) + (m_clusterCount * sizeof(FAT_FAT_TYPE));
        m_rootToc = readTocData(0);
    } else {
        m_fat.reserve(m_clusterCount);
        for (int i=0; i<m_clusterCount; ++i)
            m_fat.append(0);

        m_startOfData = sizeof(FatHeader) + (m_clusterCount * sizeof(FAT_FAT_TYPE));
    }

    return FatNoError;
}

bool QFat::isOpen()
{
    if (!m_fatFile)
        return false;

    return true;
}

void QFat::writeFat()
{
    m_fatFile->seek(0);

    char magic[5] = "QFAT";
    m_fatFile->write(magic, 4);
    m_ds.setDevice(m_fatFile);
    m_ds << m_clusterCount;
    m_ds << m_clusterSize;

    for (int i=0; i<m_clusterCount; ++i)
        m_ds << m_fat[i];
}

void QFat::close()
{
    if (m_fatFile) {
        if (m_fatFile->isOpen()) {
            writeFat();
            m_fatFile->close();
        }
        delete m_fatFile;
        m_fatFile = NULL;
    }
}

FatError QFat::checkAndCreate(FAT_FAT_TYPE clusterNum)
{
    FatError ret;

    if (!m_fatFile)
        return FatNotOpen;

    if (!m_fatFile->isOpen()) {

        if (!m_fatFile->open(QIODevice::ReadWrite))
            return FatSysError;

        if (!m_fatFile->resize(m_startOfData + m_clusterSize))
            return FatOutOfSpace;

        char magic[5] = "QFAT";
        m_fatFile->write(magic, 4);
        m_ds.setDevice(m_fatFile);
        m_ds << m_clusterCount;
        m_ds << m_clusterSize;

        for (int i=0; i<m_clusterCount; ++i)
            m_ds << m_fat[i];

        writeTocData(m_rootToc, 0);
    }

    if (m_fatFile->size() < m_startOfData + (clusterNum+1 * m_clusterSize)) {
        if (!m_fatFile->resize(m_startOfData + (clusterNum+1 * m_clusterSize)))
            return FatOutOfSpace;
    }
    return FatNoError;
}

FAT_FAT_TYPE QFat::findFreeCluster()
{
    FAT_FAT_TYPE freeCluster = 1;
    for (;freeCluster < m_clusterCount; ++freeCluster)
        if (!m_fat[freeCluster])
            return freeCluster;

    return m_clusterCount;
}

void QFat::eraseData(FAT_FAT_TYPE clusterNum)
{
    if (clusterNum == FAT_FAT_TYPE_NOVALUE)
        return;

    FAT_FAT_TYPE cluster = clusterNum;
    do {
        FAT_FAT_TYPE nextCluster = m_fat[cluster];
        m_fat[cluster] = 0;
        cluster = nextCluster;
    } while (cluster && cluster != FAT_FAT_TYPE_NOVALUE);
}

QByteArray QFat::readData(FAT_FAT_TYPE clusterNum, qint32 maxSize)
{
    QByteArray data;
    if (clusterNum == FAT_FAT_TYPE_NOVALUE)
        return data;

    char readBuffer[m_clusterSize];
    qint32 alreadyRead = 0;
    qint32 justRead;
    quint16 nextCluster = clusterNum;
    m_fatFile->seek(m_startOfData + (nextCluster * m_clusterSize));
    do {
        if (maxSize == -1 || alreadyRead+m_clusterSize < maxSize)
            justRead = m_fatFile->read(readBuffer, m_clusterSize);
        else
            justRead = m_fatFile->read(readBuffer, maxSize-alreadyRead);
        alreadyRead += justRead;
        data.append(readBuffer, justRead);

        FAT_FAT_TYPE oldCluster = nextCluster;
        nextCluster = m_fat[nextCluster];
        if (nextCluster != FAT_FAT_TYPE_NOVALUE && nextCluster != oldCluster+1)
            m_fatFile->seek(m_startOfData + (nextCluster * m_clusterSize));
    } while (nextCluster && nextCluster != FAT_FAT_TYPE_NOVALUE);

    return data;
}

FAT_FAT_TYPE QFat::writeData(const QByteArray& data, FAT_FAT_TYPE reqCluster)
{
    FAT_FAT_TYPE startCluster;
    if (reqCluster == FAT_FAT_TYPE_NOVALUE)
        startCluster = findFreeCluster();
    else
        startCluster = reqCluster;
    if (startCluster == m_clusterCount)
        return m_clusterCount;

    quint64 written;
    quint32 idx = 0;
    FAT_FAT_TYPE cluster = startCluster;
    if (checkAndCreate(cluster) != FatNoError)
        return m_clusterCount;
    m_fatFile->seek(m_startOfData + (cluster * m_clusterSize));
    forever {
        written =  m_fatFile->write(data.data() + idx, ((idx + m_clusterSize) < data.size() ? m_clusterSize : data.size() - idx));
        if (written == m_clusterSize) {
            idx += written;
            m_fat[cluster] = 0xff;
            FAT_FAT_TYPE nextCluster = findFreeCluster();
            if (nextCluster == m_clusterCount)
                return m_clusterCount;
            if (checkAndCreate(nextCluster) != FatNoError)
                return m_clusterCount;
            m_fat[cluster] = nextCluster;
            if (nextCluster != cluster+1)
                m_fatFile->seek(m_startOfData + (nextCluster * m_clusterSize));
            cluster = nextCluster;
        } else {
            m_fat[cluster] = FAT_FAT_TYPE_NOVALUE;
            break;
        }
    }

    return startCluster;
}

FatTocEntries QFat::readTocData(quint16 clusterNum)
{
    QByteArray a = readData(clusterNum);
    QDataStream ds(a);
    FatTocEntries tocs;

    ds >> tocs;
    return tocs;
}

FAT_FAT_TYPE QFat::writeTocData(const FatTocEntries &toc, FAT_FAT_TYPE cluster)
{
    QByteArray a;
    QDataStream ds(&a, QIODevice::WriteOnly);
    ds << toc;

    return writeData(a, cluster);
}

FatError QFat::setCurrentTocs(const QString &path)
{
    if (path.isEmpty()) {
        m_curTocs = m_rootToc;
        m_curTocsPath = "/";
        m_curTocsCluster = 0;
        return FatNoError;
    }
    if (path == m_curTocsPath && !m_curTocsPath.isEmpty())
        return FatNoError;

    FatTocEntries curTocs = m_rootToc;
    FAT_FAT_TYPE curTocCluster = 0;

    QStringList levels = path.split("/", QString::SkipEmptyParts);

    bool found = false;
    for (int i=0; i<levels.size(); ++i) {
        for (int j=0; j<curTocs.size(); ++j) {
            if (curTocs[j].name == levels[i]) {
                if (curTocs[j].flags & FLAG_FOLDER) {
                    curTocCluster = curTocs[j].startCluster;
                    curTocs = readTocData(curTocCluster);
                    found = true;
                    break;
                } else {
                    break;
                }
            }
        }
        if (!found)
            break;
    }
    if (found) {
        m_curTocs = curTocs;
        m_curTocsCluster = curTocCluster;
        m_curTocsPath = path;
        return FatNoError;
    } else {
        m_curTocsPath.clear();
        return FatDirNotFound;
    }
}

FatError QFat::getToc(const QString &filename, FatTocEntry& toc)
{
    FatTocEntry emptyToc;
    if (filename.isEmpty())
        return FatFileNotFound;

    QStringList levels = filename.split("/", QString::SkipEmptyParts);
    QString name = levels.takeLast();
    QString path = levels.join("/");

    return getToc(path, name, toc);
}

FatError QFat::getToc(const QString &path, const QString& name, FatTocEntry& toc)
{
    FatError ret = setCurrentTocs(path);
    if (ret != FatNoError) {
        return ret;
    }

    for (int j=0; j<m_curTocs.size(); ++j) {
        if (m_curTocs[j].name == name) {
            toc = m_curTocs[j];
            return FatNoError;
        }
    }
    return FatFileNotFound;
}

FatError QFat::getTocEntries(const QString &reqpath, FatTocEntries& tocs)
{
    FatTocEntry toc;

    if (reqpath.isEmpty() || reqpath == "/") {
        tocs = m_rootToc;
    } else {
        QStringList levels = reqpath.split("/", QString::SkipEmptyParts);
        QString name = levels.takeLast();
        QString path = levels.join("/");

        FatError ret;
        ret = getToc(path, name, toc);
        if (ret != FatNoError)
            return FatDirNotFound;

        tocs = readTocData(toc.startCluster);
    }

    return FatNoError;
}


FatError QFat::addToc(const QString &filename, const FatTocEntry &toc)
{
    QStringList levels = filename.split("/", QString::SkipEmptyParts);
    QString name = levels.takeLast();
    QString path = levels.join("/");
    return addToc(path, name, toc);
}

FatError QFat::addToc(const QString &path, const QString& name, const FatTocEntry &toc)
{
    FatError ret = setCurrentTocs(path);
    if (ret != FatNoError)
        return ret;

    for (int j=0; j<m_curTocs.size(); ++j) {
        if (m_curTocs[j].name == name) {
            m_curTocs.removeAt(j);
        }
    }

    m_curTocs.push_back(toc);
    eraseData(m_curTocsCluster);
    FAT_FAT_TYPE clnum = writeTocData(m_curTocs, m_curTocsCluster);
    if (clnum == m_clusterCount)
        return FatOutOfSpace;
    if (m_curTocsCluster == 0)
        m_rootToc = m_curTocs;

    return FatNoError;
}

FatError QFat::deleteToc(const QString &filename)
{
    QStringList levels = filename.split("/", QString::SkipEmptyParts);
    QString name = levels.takeLast();
    QString path = levels.join("/");
    return deleteToc(path, name);
}

FatError QFat::deleteToc(const QString &path, const QString& name)
{
    FatError ret = setCurrentTocs(path);
    if (ret != FatNoError)
        return ret;

    for (int j=0; j<m_curTocs.size(); ++j) {
        if (m_curTocs[j].name == name) {
            m_curTocs.removeAt(j);
            eraseData(m_curTocsCluster);
            FAT_FAT_TYPE clnum = writeTocData(m_curTocs, m_curTocsCluster);
            if (clnum == m_clusterCount)
                return FatOutOfSpace;

            if (m_curTocsCluster == 0)
                m_rootToc = m_curTocs;

            return FatNoError;
        }
    }
    return FatFileNotFound;
}

FatError QFat::makeDir(const QString &reqpath)
{
    FatError ret;

    FatTocEntry toc;
    FatTocEntries emptyToc;
    ret = getToc(reqpath, toc);
    if (ret == FatNoError)
        return FatDirAlreadyExists;

    QStringList levels = reqpath.split("/", QString::SkipEmptyParts);
    QString name = levels.takeLast();
    QString path = levels.join("/");

    FAT_FAT_TYPE clnum = writeTocData(emptyToc, FAT_FAT_TYPE_NOVALUE);
    if (clnum == m_clusterCount)
        return FatOutOfSpace;

    toc.flags = FLAG_FOLDER;
    QDateTime dt = QDateTime::currentDateTime();
    toc.creationTimestamp = dt.toTime_t();
    toc.modificationTimestamp = dt.toTime_t();
    toc.size = 0;
    toc.name = name;
    toc.startCluster = clnum;

    if((ret = addToc(path, name, toc)) != FatNoError) {
        eraseData(clnum);
        return ret;
    }
    return FatNoError;
}

FatError QFat::makeDirRecursive(const QString &reqpath)
{
    FatError ret;

    FatTocEntry toc;
    ret = getToc(reqpath, toc);
    if (ret == FatNoError)
        return FatDirAlreadyExists;

    QString partPath;
    int i = 0;
    QStringList levels = reqpath.split("/", QString::SkipEmptyParts);

    while (i<levels.size()) {
        partPath = levels[0];
        for (int j=1; j<=i; ++j)
            partPath += "/" + levels[j];
        ret = makeDir(partPath);
        if (ret != FatNoError && ret != FatDirAlreadyExists)
            return ret;
        ++i;
    }

    return FatNoError;
}

FatError QFat::removeDir(const QString& reqpath)
{
    FatError ret;
    FatTocEntry toc;

    ret = setCurrentTocs(reqpath);
    if (ret != FatNoError)
        return ret;
    if (m_curTocs.size())
        return FatDirNotEmpty;

    ret = getToc(reqpath, toc);
    if (ret != FatNoError)
        return ret;

    eraseData(toc.startCluster);
    ret = deleteToc(reqpath);
    if (ret != FatNoError)
        return ret;

    return FatNoError;
}

FatError QFat::removeDirRecursive(const QString& reqpath)
{
    FatError ret;

    ret = setCurrentTocs(reqpath);
    if (ret != FatNoError)
        return ret;
    if (m_curTocs.size())
        return FatDirNotEmpty;

    QString partPath = reqpath;
    while (!partPath.isEmpty()) {
        ret = removeDir(partPath);
        if (ret != FatNoError)
            return ret;

        QStringList levels = partPath.split("/", QString::SkipEmptyParts);
        levels.takeLast();
        partPath = levels.join("/");
    }

    return FatNoError;
}

FatError QFat::removeFile(const QString& reqpath)
{
    FatError ret;
    FatTocEntry toc;

    ret = getToc(reqpath, toc);
    if (ret != FatNoError)
        return ret;

    eraseData(toc.startCluster);
    ret = deleteToc(reqpath);
    if (ret != FatNoError)
        return ret;

    return FatNoError;
}

QString QFat::statusToc(const QString& path, int indent)
{
    QString ret;
    QString fill;
    fill.fill(' ', indent*2);

    FatTocEntries curToc;
    getTocEntries(path, curToc);

    for (int i=0; i<curToc.size(); ++i) {
        if (curToc[i].flags & FLAG_FILE) {
            ret += QString("%3(%4) %1\t%2\n").arg(curToc[i].name).arg(curToc[i].size).arg(fill).arg(curToc[i].startCluster, sizeof(FAT_FAT_TYPE)*2, 16);
        } else if (curToc[i].flags & FLAG_FOLDER) {
            ret += QString("%3(%4) %1/\n").arg(curToc[i].name).arg(fill).arg(curToc[i].startCluster, sizeof(FAT_FAT_TYPE)*2, 16);
            ret += statusToc(path + curToc[i].name + "/", indent + 1);
        }
    }

    return ret;
}

QString QFat::status()
{
    QString ret;

    int countFat = 0;
    int col = 0;
    ret = QString("FAT in use:\n  ");
    for (int i=0; i<m_fat.size(); ++i) {
        if (m_fat[i]) {
            ret += QString("(%1)%2").arg(i, sizeof(FAT_FAT_TYPE)*2, 16).arg(m_fat[i], sizeof(FAT_FAT_TYPE)*2, 16);
            if (col<3) {
                ++col;
                ret += " ";
            } else {
                col = 0;
                ret += "\n  ";
            }
            countFat++;
        }
    }
    if (col)
        ret += "\n";
    ret += QString("  Count = %1\n").arg(countFat);
    ret += QString("  Fat Size: %1\n").arg(m_clusterCount*sizeof(FAT_FAT_TYPE));
    ret += "\n";
    ret += "TOC:\n";
    ret += statusToc("/", 1);
    ret += "\n" + QString("FatFile Size: %1\n").arg(m_fatFile->size());


    return ret;
}
