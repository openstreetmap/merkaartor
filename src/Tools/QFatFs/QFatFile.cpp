#include "QFatFile.h"

QFatFile::QFatFile(QFat* fat)
    : QBuffer()
    , m_fat(fat)
{
}

QFatFile::QFatFile(const QString& fileName, QFat* fat)
    : QBuffer()
    , m_fat(fat)
{
    setFilename(fileName);
}

void QFatFile::setFilename(const QString &filename)
{
    if (filename.length() == 0)
        return;

    QStringList tokens = filename.split("/", QString::SkipEmptyParts);
    m_name = tokens.takeLast();
    m_path = tokens.join("/");
}

bool QFatFile::open(QIODevice::OpenMode mode)
{
    if (!m_fat->isOpen())
        return false; //FatNotOpen;

    FatError ret = m_fat->getToc(m_path, m_name, m_toc);
    if (ret == FatFileNotFound) {
        if (mode & QIODevice::ReadOnly)
            return false; //FatFileNotFound;

        QDateTime dt = QDateTime::currentDateTime();
        m_toc.flags = FLAG_FILE;
        m_toc.creationTimestamp = dt.toTime_t();
        m_toc.modificationTimestamp = dt.toTime_t();
        m_toc.size = 0;
        m_toc.name = m_name;
        m_toc.startCluster = FAT_FAT_TYPE_NOVALUE;

        if (m_fat->addToc(m_path, m_name, m_toc) != FatNoError) {
            qDebug() << "QFatFile::open: Error as-dding toc";
            return false;
        }
    } else if (ret == FatNoError){
        buffer() = m_fat->readData(m_toc.startCluster, m_toc.size);
    } else
        return false;

    if (!QBuffer::open(mode))
        return false; //FatSysError;

    return true; //FatNoError;
}

void QFatFile::close()
{
    if (openMode() & QIODevice::WriteOnly) {
        QBuffer::close();
        m_fat->eraseData(m_toc.startCluster);
        if (buffer().size())
            m_toc.startCluster = m_fat->writeData(buffer(), m_toc.startCluster);
        else
            m_toc.startCluster = FAT_FAT_TYPE_NOVALUE;
        m_toc.size = size();
        m_toc.modificationTimestamp = QDateTime::currentDateTime().toTime_t();
        if (m_fat->addToc(m_path, m_name, m_toc) != FatNoError) {
            qDebug() << "QFatFile::close: Error adding toc";
            return;
        }
        m_fat->writeFat();
    } else
        QBuffer::close();

}
