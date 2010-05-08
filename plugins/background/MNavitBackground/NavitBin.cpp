//***************************************************************
// CLass: NavitBin
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//******************************************************************

#include "NavitBin.h"

#include <QCoreApplication>
#include <QDebug>
#include <QMessageBox>

#include <QDataStream>
#include <QPair>

NavitBin::NavitBin()
{
}

bool NavitBin::setFilename(const QString& filename)
{
    bool ok;
    tileIndex.clear();

    zip = new QuaZip(filename);
    if(!zip->open(QuaZip::mdUnzip)) {
        QMessageBox::critical(0,QCoreApplication::translate("NavitBackground","Not a valid file"),QCoreApplication::translate("NavitBackground","Cannot load file."));
        return false;
    }
    file = new QuaZipFile(zip);

    for(bool more=zip->goToFirstFile(); more; more=zip->goToNextFile())
        tileIndex.append(zip->getCurrentFileName());

    ok = readTile("index");
    indexTile = theTiles["index"];

    return ok;
}

bool NavitBin::readTile(int index) const
{
    if (theTiles.contains(tileIndex[index]))
        return true;

    return readTile(tileIndex[index]);
}

bool NavitBin::readTile(QString fn) const
{
    qDebug() << "Reading: "  << fn;

    if (!zip->setCurrentFile(fn))
        return false;
    if(!file->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(0,QCoreApplication::translate("NavitBackground","Not a valid file"),QCoreApplication::translate("NavitBackground","Cannot load file: ").arg(file->getZipError()));
        return false;
    }

    NavitTile aTile;

    qint32 len;
    quint32 type;
    qint32 coordLen;
    qint32 x, y;
    qint32 attrLen;
    quint32 attrType;
    qint8 attr;

    QByteArray ba = file->readAll();
    QDataStream data(ba);
    data.setByteOrder(QDataStream::LittleEndian);
    while (!data.atEnd()) {
        NavitFeature aFeat;
        data >> len;
        data >> type;
//        qDebug() << "-- type: " << QString("%1").arg(type, 0, 16);
        aFeat.type = type;
        data >> coordLen;
        for (int i=0; i<coordLen/2; ++i) {
            data >> x >> y;
            aFeat.coordinates << QPoint(x, y);
        }
        for (int j=2+1+coordLen; j<len; j+=2) {
            QByteArray attribute;
            data >> attrLen;
            data >> attrType;
//            qDebug() << "-- attrType: " << QString("%1").arg(attrType, 0, 16);
            if (type == type_submap && attrType == attr_zipfile_ref) {
                quint32 zipref;
                data >> zipref;
                aTile.pointers.append(qMakePair(QRect(aFeat.coordinates[0], aFeat.coordinates[1]), zipref));
//                qDebug() << " ------ attr_zipfile_ref: " << zipref;
//                foreach (QPoint p, aFeat.coordinates) {
//                    qDebug() << " -- Coord: " << p;
//                }
            } else {
                for (int i=0; i<(attrLen-1)*sizeof(qint32); ++i) {
                    data >> attr;
                    attribute.append(attr);
                }
                aFeat.attributes << NavitAttribute(attrType, attribute);
            }
            j += attrLen-1;
        }
        aTile.features.append(aFeat);
    }
    file->close();

    theTiles.insert(fn, aTile);

    return true;
}

bool NavitBin::getFeatures(const QString& tileRef, QList <NavitFeature>& theFeats) const
{
    readTile(tileRef);
    NavitTile t = theTiles[tileRef];
    foreach(NavitFeature f, t.features) {
        if (f.type & 0x00010000) { // POI
            theFeats.append(f);
        } else if (f.type & 0x80000000) { // Line
            theFeats.append(f);
        } else if (f.type & 0xc0000000) { // Area
            theFeats.append(f);
        }
    }

}

bool NavitBin::getFeatures(const QRect& pBox, QList <NavitFeature>& theFeats) const
{
    QString tileRef;
    tileRef.fill('_', 14);
    QRect tileRect = QRect(QPoint(-20015087, -20015087), QPoint(20015087, 20015087));

    int lvl = -1;
    bool ok = false;
    while (!ok && lvl < 14) {
        ++lvl;
        QSize tmpSize = tileRect.size() /2;
        //        qDebug() << "ref: " << tileRef << "; rect: " << tileRect << "; sz: " << tmpSize;
        QRect c = QRect(tileRect.topLeft().x() + tmpSize.width(), tileRect.topLeft().y(), tmpSize.width(), tmpSize.height());
        if (c.contains(pBox)) {
            tileRect = c;
            tileRef.replace(lvl, 1, 'c');
            getFeatures(tileRef, theFeats);
            continue;
        }
        QRect d = QRect(tileRect.topLeft().x(), tileRect.topLeft().y(), tmpSize.width(), tmpSize.height());
        if (d.contains(pBox)) {
            tileRect = d;
            tileRef.replace(lvl, 1, 'd');
            getFeatures(tileRef, theFeats);
            continue;
        }
        QRect a = QRect(tileRect.topLeft().x() + tmpSize.width(), tileRect.topLeft().y() + tmpSize.height(), tmpSize.width(), tmpSize.height());
        if (a.contains(pBox)) {
            tileRect = a;
            tileRef.replace(lvl, 1, 'a');
            getFeatures(tileRef, theFeats);
            continue;
        }
        QRect b = QRect(tileRect.topLeft().x(), tileRect.topLeft().y() + tmpSize.height(), tmpSize.width(), tmpSize.height());
        if (b.contains(pBox)) {
            tileRect = b;
            tileRef.replace(lvl, 1, 'b');
            getFeatures(tileRef, theFeats);
            continue;
        }
        ok = true;
    }
    qDebug() << "lvl: " << lvl << "; tile: " << tileRef << "; pbox: " << pBox;

//    NavitTile t = indexTile;
//    bool stop = false;
//    while (!stop) {
//        foreach(NavitFeature f, t.features) {
//            if (f.type & 0x00010000) { // POI
//                theNodes.append(qMakePair(f.type, f.coordinates.at(0)));
//            } else if (f.type & 0x80000000) { // Line
//                thePolys.append(qMakePair(f.type, QPolygon(f.coordinates)));
//            } else if (f.type & 0xc0000000) { // Area
//                thePolys.append(qMakePair(f.type, QPolygon(f.coordinates)));
//            }
//        }
//        stop = true;
//        for (int i=t.pointers.size()-1; i>=0; --i) {
//            if (t.pointers[i].first.contains(box)) {
//                readTile(t.pointers[i].second);
//                t = theTiles[t.pointers[i].second];
//                stop = false;
//                break;
//            }
//        }
//    }
    return true;
}
