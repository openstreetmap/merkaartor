//***************************************************************
// CLass: SpatialiteAdapter
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//******************************************************************

#include "SpatialiteAdapter.h"

#include <QCoreApplication>
#include <QtPlugin>
#include <QAction>
#include <QFileDialog>
#include <QPainter>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>
#include <QBuffer>
#include <QPair>
#include <QStringList>

#include <math.h>
#include "spatialite/gaiageo.h"

#include "MasPaintStyle.h"
#include "PrimitiveFeature.h"

static const QUuid theUid ("{4509fe81-47d0-4487-b6d0-e8910f2f1f7d}");
static const QString theName("Spatialite");

QUuid SpatialiteAdapterFactory::getId() const
{
    return theUid;
}

QString	SpatialiteAdapterFactory::getName() const
{
    return theName;
}

/******/

#define FILTER_OPEN_SUPPORTED \
    tr("Supported formats")+" (*.sqlite)\n" \
    +tr("All Files (*)")

double angToRad(double a)
{
    return a*M_PI/180.;
}

QPointF mercatorProject(const QPointF& c)
{
    double x = angToRad(c.x()) / M_PI * 20037508.34;
    double y = log(tan(angToRad(c.y())) + 1/cos(angToRad(c.y()))) / M_PI * (20037508.34);

    return QPointF(x, y);
}

/*****/

SpatialiteAdapter::SpatialiteAdapter()
{
    /*
    VERY IMPORTANT:
    you must initialize the SpatiaLite extension [and related]
    BEFORE attempting to perform any other SQLite call
    */
    spatialite_init (0);

    /* showing the SQLite version */
    qDebug ("SQLite version: %s", sqlite3_libversion ());
    /* showing the SpatiaLite version */
    qDebug ("SpatiaLite version: %s", spatialite_version ());

    QAction* loadFile = new QAction(tr("Load Spatialite db..."), this);
    loadFile->setData(theUid.toString());
    connect(loadFile, SIGNAL(triggered()), SLOT(onLoadFile()));
    theMenu = new QMenu();
    theMenu->addAction(loadFile);

    m_loaded = false;

    MasPaintStyle theStyle;
    theStyle.loadPainters(":/Styles/Mapnik.mas");
    for (int i=0; i<theStyle.painterSize(); ++i) {
        thePrimitivePainters.append(PrimitivePainter(*theStyle.getPainter(i)));
    }
}


SpatialiteAdapter::~SpatialiteAdapter()
{
    if (m_loaded)
        sqlite3_close(m_handle);
}

void SpatialiteAdapter::onLoadFile()
{
    QString fileName = QFileDialog::getOpenFileName(
                    NULL,
                    tr("Open Spatialite db"),
                    "", FILTER_OPEN_SUPPORTED);
    if (fileName.isEmpty())
        return;

    setFile(fileName);
}

void SpatialiteAdapter::setFile(const QString& fn)
{
    if (m_loaded)
        sqlite3_close(m_handle);
    m_loaded = false;

    int ret = sqlite3_open_v2 (fn.toUtf8().data(), &m_handle, SQLITE_OPEN_READONLY, NULL);
    if (ret != SQLITE_OK)
    {
        QMessageBox::critical(0,QCoreApplication::translate("SpatialiteBackground","No valid file"),QCoreApplication::translate("SpatialiteBackground","Cannot open db."));
        sqlite3_close (m_handle);
        return;
    }
    m_dbName = fn;
    m_loaded = true;

    QString q = QString("select * from ln_highway where MBRIntersects(BuildMBR(?, ?, ?, ?),Geometry)");
    ret = sqlite3_prepare_v2(m_handle, q.toUtf8().data(), q.size(), &m_highwayStmt, NULL);
    if (ret != SQLITE_OK) {
        qDebug() << "Sqlite: prepare error: " << ret;
    }

    q = QString("select distinct sub_type from ln_highway");
    sqlite3_stmt *pStmt;
    ret = sqlite3_prepare_v2(m_handle, q.toUtf8().data(), q.size(), &pStmt, NULL);
    if (ret != SQLITE_OK) {
        qDebug() << "Sqlite: prepare error: " << ret;
    }

    while (sqlite3_step(pStmt) == SQLITE_ROW) {
        QString sub_type((const char*)sqlite3_column_text(pStmt, 0));
        PrimitiveFeature f;
        f.Tags.append(qMakePair(QString("highway"), sub_type));
        for(int i=0; i< thePrimitivePainters.size(); ++i) {
            if (thePrimitivePainters[i].matchesTag(&f, 0)) {
                myStyles.insert(sub_type, &thePrimitivePainters[i]);
                break;
            }
        }
    }
    sqlite3_finalize(pStmt);

    emit (forceRefresh());
}

QString	SpatialiteAdapter::getHost() const
{
    return "";
}

QUuid SpatialiteAdapter::getId() const
{
    return QUuid(theUid);
}

IMapAdapter::Type SpatialiteAdapter::getType() const
{
    return IMapAdapter::DirectBackground;
}

QString	SpatialiteAdapter::getName() const
{
    return theName;
}

QMenu* SpatialiteAdapter::getMenu() const
{
    return theMenu;
}

QRectF SpatialiteAdapter::getBoundingbox() const
{
    return QRectF(QPointF(-180.00, -90.00), QPointF(180.00, 90.00));
}

QString SpatialiteAdapter::projection() const
{
    return "EPSG:4326";
}

QPixmap SpatialiteAdapter::getPixmap(const QRectF& wgs84Bbox, const QRectF& /*projBbox*/, const QRect& src) const
{
    if (!m_loaded)
        return QPixmap();

    QPixmap pix(src.size());
    pix.fill(Qt::transparent);
    QPainter P(&pix);
    P.setRenderHint(QPainter::Antialiasing);

    render(&P, wgs84Bbox, wgs84Bbox, src);

    P.end();
    return pix;
}

void SpatialiteAdapter::render(QPainter* P, const QRectF& fullbox, const QRectF& selbox, const QRect& src) const
{
    if (!m_loaded)
        return;

    QTransform tfm;

    double ScaleLon = src.width() / fullbox.width();
    double ScaleLat = src.height() / fullbox.height();

    double PLon = fullbox.center().x() * ScaleLon;
    double PLat = fullbox.center().y() * ScaleLat;
    double DeltaLon = src.width() / 2 - PLon;
    double DeltaLat = src.height() - (src.height() / 2 - PLat);

    double LengthOfOneDegreeLat = 6378137.0 * M_PI / 180;
    double LengthOfOneDegreeLon = LengthOfOneDegreeLat * fabs(cos(angToRad(fullbox.center().y())));
    double lonAnglePerM =  1 / LengthOfOneDegreeLon;
    double PixelPerM = src.width() / (double)fullbox.width() * lonAnglePerM;

    tfm.setMatrix(ScaleLon, 0, 0, 0, -ScaleLat, 0, DeltaLon, DeltaLat, 1);

    QPainterPath clipPath;
    clipPath.addRect(selbox/*.adjusted(-1000, -1000, 1000, 1000)*/);

    sqlite3_bind_double(m_highwayStmt, 1, selbox.bottomLeft().x());
    sqlite3_bind_double(m_highwayStmt, 2, selbox.topRight().y());
    sqlite3_bind_double(m_highwayStmt, 3, selbox.topRight().x());
    sqlite3_bind_double(m_highwayStmt, 4, selbox.bottomLeft().y());

    while (sqlite3_step(m_highwayStmt) == SQLITE_ROW) {
        qint64 id = sqlite3_column_int64(m_highwayStmt, 0);
        QString sub_type((const char*)sqlite3_column_text(m_highwayStmt, 1));
        QString name((const char*)sqlite3_column_text(m_highwayStmt, 2));
        int blobSize = sqlite3_column_bytes(m_highwayStmt, 3);
        QByteArray ba((const char*)sqlite3_column_blob(m_highwayStmt, 3), blobSize);
        const unsigned char* blob = (const unsigned char*)ba.constData();

        gaiaGeomCollPtr coll = gaiaFromSpatiaLiteBlobWkb(blob, blobSize);
        Q_ASSERT(coll);

        gaiaPointPtr gaianode = coll->FirstPoint;

        gaiaLinestringPtr way = coll->FirstLinestring;
        double x, y;
        while (way) {
            if (way->Points) {
                QPainterPath path;
                gaiaGetPoint(way->Coords, 0, &x, &y);
                path.moveTo(x, y);
                for (int i=1; i<way->Points; ++i) {
                    gaiaGetPoint(way->Coords, i, &x, &y);
                    path.lineTo(x, y);
                }
                if (!path.isEmpty()) {
                    if (myStyles.contains(sub_type)) {
                        PrimitivePainter* fpainter = myStyles[sub_type];
                        if (fpainter->matchesZoom(PixelPerM)) {
                            QPainterPath pp = tfm.map(path.intersected(clipPath));
                            fpainter->drawBackground(&pp, P, PixelPerM);
                            fpainter->drawForeground(&pp, P, PixelPerM);
                            fpainter->drawTouchup(&pp, P, PixelPerM);
                            //          f.painter()->drawLabel(&pp, P, PixelPerM);
                        }
                    }
                }
            }

            way = way->Next;
        }
    }

    sqlite3_reset(m_highwayStmt);

//    QRectF fBox(NavitProject(fullbox.topLeft()), NavitProject(fullbox.bottomRight()));
//    QRectF sBox(NavitProject(selbox.topLeft()), NavitProject(selbox.bottomRight()));

//    QList<NavitFeature> theFeats;
//    navit.getFeatures(sBox.toRect(), theFeats);

//    qDebug () << "Feats: " << theFeats.size();
//    if (!theFeats.size())
//        return;

//    QTransform tfm;

//    double ScaleLon = src.width() / fBox.width();
//    double ScaleLat = src.height() / fBox.height();

//    double PLon = fBox.center().x() * ScaleLon;
//    double PLat = fBox.center().y() * ScaleLat;
//    double DeltaLon = src.width() / 2 - PLon;
//    double DeltaLat = src.height() - (src.height() / 2 - PLat);

//    double LengthOfOneDegreeLat = 6378137.0 * M_PI / 180;
//    double LengthOfOneDegreeLon = LengthOfOneDegreeLat * fabs(cos(angToRad(fullbox.center().y())));
//    double lonAnglePerM =  1 / LengthOfOneDegreeLon;
//    double PixelPerM = src.width() / (double)fullbox.width() * lonAnglePerM;

//    tfm.setMatrix(ScaleLon, 0, 0, 0, -ScaleLat, 0, DeltaLon, DeltaLat, 1);

//    QPainterPath clipPath;
//    clipPath.addRect(sBox.adjusted(-1000, -1000, 1000, 1000));
//    foreach (NavitFeature f, theFeats) {
//        if (f.coordinates.size() > 1) {
//            QPolygonF d(f.coordinates);
//            QPainterPath aPath;
//            aPath.addPolygon(d);
////            QRect br = d.boundingRect();
////            qDebug() << "brect: " << br;
//            aPath = aPath.intersected(clipPath);
////            if (!aPath.intersects(pBox))
////                continue;
////            if ((f.type & 0xc0000000) == 0xc0000000) {
////                P.setPen(QPen(Qt::lightGray, 1));
////                aPath.closeSubpath();
////                P.drawPath(aPath);
//////                P.drawPolygon(QPolygon(f.coordinates));
////            } else {
////                P.setPen(QPen(Qt::blue, 2));
//////                P.drawPolyline(f.coordinates);
////                P.drawPath(aPath);
////            }
//            if (!aPath.isEmpty()) {
//                if (myStyles.contains(f.type)) {
//                    if (myStyles[f.type]->matchesZoom(PixelPerM)) {
//                        QPainterPath pp = tfm.map(aPath);
//                        if ((f.type & 0xc0000000) == 0xc0000000) {
//                            pp.closeSubpath();
//                            myStyles[f.type]->drawBackground(&pp, P, PixelPerM);
//                        } else {
//                            myStyles[f.type]->drawForeground(&pp, P, PixelPerM);
//                        }
//                        myStyles[f.type]->drawTouchup(&pp, P, PixelPerM);
//                        //          f.painter()->drawLabel(&pp, P, PixelPerM);
//                    }
//                }
//            }
//        } else {
//            if (f.coordinates.size() == 1) {
//                if (!sBox.contains(f.coordinates[0]))
//                    continue;
//                //                P.setPen(QPen(Qt::red, 5));
//                //                P.drawPoint(f.coordinates[0]);
//                if (myStyles.contains(f.type)) {
//                    if (myStyles[f.type]->matchesZoom(PixelPerM)) {
//                        QPointF pp = tfm.map(f.coordinates[0]);
//                        myStyles[f.type]->drawTouchup(&pp, P, PixelPerM);
//                        //                myStyles[StyleNr(w)]->drawLabel(&pp, &P, PixelPerM, strL[0]);
//                    }
//                }
//            }
//        }
//    }
}

IImageManager* SpatialiteAdapter::getImageManager()
{
    return NULL;
}

void SpatialiteAdapter::setImageManager(IImageManager* anImageManager)
{
}

void SpatialiteAdapter::cleanup()
{
}

bool SpatialiteAdapter::toXML(QDomElement xParent)
{
    bool OK = true;

    QDomElement fs = xParent.ownerDocument().createElement("Database");
    xParent.appendChild(fs);
    if (m_loaded)
        fs.setAttribute("filename", m_dbName);

    return OK;
}

void SpatialiteAdapter::fromXML(const QDomElement xParent)
{
    QDomElement fs = xParent.firstChildElement();
    while(!fs.isNull()) {
        if (fs.tagName() == "Database") {
            QString fn = fs.attribute("filename");
            if (!fn.isEmpty())
                setFile(fn);
        }

        fs = fs.nextSiblingElement();
    }
}

QString SpatialiteAdapter::toPropertiesHtml()
{
    QString h;

    if (m_loaded)
        h += "<i>" + tr("Filename") + ": </i>" + m_dbName;

    return h;
}

#ifndef _MOBILE
Q_EXPORT_PLUGIN2(MSpatialiteBackgroundPlugin, SpatialiteAdapterFactory)
#endif
