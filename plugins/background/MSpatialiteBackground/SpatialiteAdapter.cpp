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
    tr("Supported formats")+" (*.sqlite *.spatialite)\n" \
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

inline uint qHash(IFeature::FId id)
{
    uint h1 = qHash(id.type);
    uint h2 = qHash(id.numId);
    return ((h1 << 16) | (h1 >> 16)) ^ h2;
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

    m_cache.setMaxCost(100000);

    m_tables
            << "ln_abutters"
            << "ln_aerialway"
            << "ln_aeroway"
            << "ln_amenity"
            << "ln_barrier"
            << "ln_boundary"
            << "ln_building"
            << "ln_cycleway"
            << "ln_generic"
            << "ln_highway"
            << "ln_historic"
            << "ln_junction"
            << "ln_landuse"
            << "ln_leisure"
            << "ln_man_made"
            << "ln_military"
            << "ln_natural"
            << "ln_parking"
            << "ln_place"
            << "ln_power"
            << "ln_railway"
            << "ln_route"
            << "ln_service"
            << "ln_shop"
            << "ln_sport"
            << "ln_tourism"
            << "ln_tracktype"
            << "ln_waterway"
            << "pg_aeroway"
            << "pg_amenity"
            << "pg_barrier"
            << "pg_boundary"
            << "pg_building"
            << "pg_generic"
            << "pg_highway"
            << "pg_historic"
            << "pg_landuse"
            << "pg_leisure"
            << "pg_man_made"
            << "pg_military"
            << "pg_natural"
            << "pg_railway"
            << "pg_shop"
            << "pg_sport"
            << "pg_tourism"
            << "pg_waterway"
            << "pt_addresses"
            << "pt_aerialway"
            << "pt_aeroway"
            << "pt_amenity"
            << "pt_barrier"
            << "pt_boundary"
            << "pt_building"
            << "pt_cycleway"
            << "pt_generic"
            << "pt_geological"
            << "pt_highway"
            << "pt_historic"
            << "pt_junction"
            << "pt_landuse"
            << "pt_leisure"
            << "pt_man_made"
            << "pt_military"
            << "pt_natural"
            << "pt_place"
            << "pt_power"
            << "pt_railway"
            << "pt_route"
            << "pt_service"
            << "pt_shop"
            << "pt_sport"
            << "pt_tourism"
            << "pt_traffic_calming"
            << "pt_traffic_sign"
            << "pt_waterway"
            ;
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

void SpatialiteAdapter::initTable(const QString& table)
{
    QString tag = table.mid(3);
    QString q = QString("select * from %1 where ROWID IN "
                        "(Select rowid from idx_%1_Geometry WHERE xmax > ? and ymax > ? and xmin < ? and ymin < ?);").arg(table);
    int ret = sqlite3_prepare_v2(m_handle, q.toUtf8().data(), q.size(), &m_stmtHandles[table], NULL);
    if (ret != SQLITE_OK) {
        qDebug() << "Sqlite: prepare error: " << ret;
        return;
    }
    q = QString("select distinct sub_type from %1").arg(table);
    sqlite3_stmt *pStmt;
    ret = sqlite3_prepare_v2(m_handle, q.toUtf8().data(), q.size(), &pStmt, NULL);
    if (ret != SQLITE_OK) {
        qDebug() << "Sqlite: prepare error: " << ret;
    }
    while (sqlite3_step(pStmt) == SQLITE_ROW) {
        QString sub_type((const char*)sqlite3_column_text(pStmt, 0));
        PrimitiveFeature f;
        f.Tags.append(qMakePair(tag, sub_type));
        for(int i=0; i< thePrimitivePainters.size(); ++i) {
            if (thePrimitivePainters[i].matchesTag(&f, 0)) {
                myStyles.insert(QString("%1%2").arg(tag).arg(sub_type), &thePrimitivePainters[i]);
                break;
            }
        }
    }
    sqlite3_finalize(pStmt);
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

    foreach (QString s, m_tables)
        initTable(s);

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

    double ScaleLon = src.width() / wgs84Bbox.width();
    double ScaleLat = src.height() / wgs84Bbox.height();

    double PLon = wgs84Bbox.center().x() * ScaleLon;
    double PLat = wgs84Bbox.center().y() * ScaleLat;
    double DeltaLon = src.width() / 2 - PLon;
    double DeltaLat = src.height() - (src.height() / 2 - PLat);

    double LengthOfOneDegreeLat = 6378137.0 * M_PI / 180;
    double LengthOfOneDegreeLon = LengthOfOneDegreeLat * fabs(cos(angToRad(wgs84Bbox.center().y())));
    double lonAnglePerM =  1 / LengthOfOneDegreeLon;
    m_PixelPerM = src.width() / (double)wgs84Bbox.width() * lonAnglePerM;
    qDebug() << "ppm: " << m_PixelPerM;

    m_transform.setMatrix(ScaleLon, 0, 0, 0, -ScaleLat, 0, DeltaLon, DeltaLat, 1);

    theFeatures.clear();
    foreach (QString s, m_tables)
        buildFeatures(s, &P, wgs84Bbox, wgs84Bbox, src);

    foreach(PrimitiveFeature* f, theFeatures) {
        if (f->theId.type & IFeature::Point)
            continue;

        PrimitivePainter* fpainter = myStyles[QString("%1%2").arg(f->Tags[0].first).arg(f->Tags[0].second)];
        if (fpainter->matchesZoom(m_PixelPerM)) {
            QPainterPath pp = m_transform.map(f->thePath);
                fpainter->drawBackground(&pp, &P, m_PixelPerM);
        }
    }
    foreach(PrimitiveFeature* f, theFeatures) {
        if (f->theId.type & IFeature::Point)
            continue;

        PrimitivePainter* fpainter = myStyles[QString("%1%2").arg(f->Tags[0].first).arg(f->Tags[0].second)];
        if (fpainter->matchesZoom(m_PixelPerM)) {
            QPainterPath pp = m_transform.map(f->thePath);
            if (!(f->theId.type & IFeature::Point))
                fpainter->drawForeground(&pp,& P, m_PixelPerM);
        }
    }
    foreach(PrimitiveFeature* f, theFeatures) {
        PrimitivePainter* fpainter = myStyles[QString("%1%2").arg(f->Tags[0].first).arg(f->Tags[0].second)];
        if (fpainter->matchesZoom(m_PixelPerM)) {
            if (f->theId.type & IFeature::Point) {
                QPointF pt = m_transform.map((QPointF)f->thePath.elementAt(0));
                fpainter->drawTouchup(&pt, &P, m_PixelPerM);
            } else {
                QPainterPath pp = m_transform.map(f->thePath);
                fpainter->drawTouchup(&pp, &P, m_PixelPerM);
            }
        }
    }
    P.end();

    foreach(PrimitiveFeature* f, theFeatures)
        m_cache.insert(f->theId, f);

    return pix;
}

void SpatialiteAdapter::buildFeatures(const QString& table, QPainter* P, const QRectF& fullbox, const QRectF& selbox, const QRect& src) const
{
    QString tag = table.mid(3);
//    QPainterPath clipPath;
//    clipPath.addRect(selbox/*.adjusted(-1000, -1000, 1000, 1000)*/);
    double x, y;

    sqlite3_stmt* pStmt = m_stmtHandles[table];
    if (!pStmt)
        return;

    sqlite3_bind_double(pStmt, 1, selbox.bottomLeft().x());
    sqlite3_bind_double(pStmt, 2, selbox.topRight().y());
    sqlite3_bind_double(pStmt, 3, selbox.topRight().x());
    sqlite3_bind_double(pStmt, 4, selbox.bottomLeft().y());

    while (sqlite3_step(pStmt) == SQLITE_ROW) {
        qint64 id = sqlite3_column_int64(pStmt, 0);
        if (m_cache.contains(IFeature::FId(IFeature::LineString, id))) {
            PrimitiveFeature* f = m_cache.take(IFeature::FId(IFeature::LineString, id));
            theFeatures << f;
            continue;
        }
        QString sub_type((const char*)sqlite3_column_text(pStmt, 1));
        if (!myStyles.contains(QString("%1%2").arg(tag).arg(sub_type)))
            continue;

        QString name((const char*)sqlite3_column_text(pStmt, 2));
        int blobSize = sqlite3_column_bytes(pStmt, 3);
        QByteArray ba((const char*)sqlite3_column_blob(pStmt, 3), blobSize);
        const unsigned char* blob = (const unsigned char*)ba.constData();

        gaiaGeomCollPtr coll = gaiaFromSpatiaLiteBlobWkb(blob, blobSize);
        Q_ASSERT(coll);

        gaiaPointPtr node = coll->FirstPoint;
        while (node) {
            PrimitiveFeature* f = new PrimitiveFeature();
            f->theId = IFeature::FId(IFeature::Point, id);
            f->theName = name;
            f->thePath.moveTo(node->X, node->Y);
            f->Tags.append(qMakePair(tag, sub_type));
            theFeatures << f;

            node = node->Next;
        }

        gaiaLinestringPtr way = coll->FirstLinestring;
        while (way) {
            if (way->Points) {
                PrimitiveFeature* f = new PrimitiveFeature();
                f->theId = IFeature::FId(IFeature::LineString, id);
                f->theName = name;
                f->Tags.append(qMakePair(tag, sub_type));

                gaiaGetPoint(way->Coords, 0, &x, &y);
                f->thePath.moveTo(x, y);
                for (int i=1; i<way->Points; ++i) {
                    gaiaGetPoint(way->Coords, i, &x, &y);
                    f->thePath.lineTo(x, y);
                }
                theFeatures << f;
            }

            way = way->Next;
        }

        gaiaPolygonPtr poly = coll->FirstPolygon;
        while (poly) {
            gaiaRingPtr ring = poly->Exterior;
            if (poly->NumInteriors)
                qDebug() << "has interiors!";
            if (ring->Points) {
                PrimitiveFeature* f = new PrimitiveFeature();
                f->theId = IFeature::FId(IFeature::LineString, id);
                f->theName = name;
                f->Tags.append(qMakePair(tag, sub_type));

                gaiaGetPoint(ring->Coords, 0, &x, &y);
                f->thePath.moveTo(x, y);
                for (int i=1; i<ring->Points; ++i) {
                    gaiaGetPoint(ring->Coords, i, &x, &y);
                    f->thePath.lineTo(x, y);
                }
//                path.closeSubpath();
                theFeatures << f;
            }

            poly = poly->Next;
        }
    }

    sqlite3_reset(pStmt);
    sqlite3_clear_bindings(pStmt);
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
