#ifndef QFATFILE_H
#define QFATFILE_H

#include <QtCore>

#include "QFat.h"

class QFatFile : public QBuffer
{
public:
    QFatFile(QFat* fat);
    QFatFile(const QString& fileName, QFat* fat);
    void setFilename(const QString& filename);

    bool open ( OpenMode mode );
    void close ();

private:
    QFat* m_fat;
    QString m_path;
    QString m_name;
    QDateTime m_creationTime;
    QDateTime m_modificationTime;

    FatTocEntry m_toc;

};

#endif // QFATFILE_H
