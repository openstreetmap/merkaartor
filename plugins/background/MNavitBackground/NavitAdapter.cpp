//***************************************************************
// CLass: NavitAdapter
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//******************************************************************

#include "NavitAdapter.h"

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

#include <math.h>

static const QUuid theUid ("{afc13af7-d538-48e1-9997-a2b45db5b3ff}");

#define FILTER_OPEN_SUPPORTED \
    tr("Supported formats")+" (*.bin)\n" \
    +tr("All Files (*)")

double angToRad(double a)
{
    return a*M_PI/180.;
}

QPoint NavitProject(QPointF i)
{
    int x = i.x()*6371000.0*M_PI/180;
    int y = log(tan(M_PI_4+i.y()*M_PI/360))*6371000.0;
    return QPoint(x, y);
}

QPointF mercatorProject(const QPointF& c)
{
    double x = angToRad(c.x()) / M_PI * 20037508.34;
    double y = log(tan(angToRad(c.y())) + 1/cos(angToRad(c.y()))) / M_PI * (20037508.34);

    return QPointF(x, y);
}

NavitAdapter::NavitAdapter()
{
    QAction* loadFile = new QAction(tr("Load Navit file..."), this);
    loadFile->setData(theUid.toString());
    connect(loadFile, SIGNAL(triggered()), SLOT(onLoadFile()));
    theMenu = new QMenu();
    theMenu->addAction(loadFile);

    loaded = false;

//    loaded = navit.setFilename("C:/home/cbro/Merkaartor/osm_bbox_11.3,47.9,11.7,48.2.bin");
    loaded = navit.setFilename("C:/home/cbro/Merkaartor/belgium.navit.bin");

}


NavitAdapter::~NavitAdapter()
{
}

void NavitAdapter::onLoadFile()
{
    QString fileName = QFileDialog::getOpenFileName(
                    NULL,
                    tr("Open Navit file"),
                    "", FILTER_OPEN_SUPPORTED);
    if (fileName.isEmpty())
        return;

    loaded = navit.setFilename(fileName);
    return;
}

QString	NavitAdapter::getHost() const
{
    return "";
}

QUuid NavitAdapter::getId() const
{
    return QUuid(theUid);
}

IMapAdapter::Type NavitAdapter::getType() const
{
    return IMapAdapter::DirectBackground;
}

QString	NavitAdapter::getName() const
{
    return "Navit";
}

QMenu* NavitAdapter::getMenu() const
{
    return theMenu;
}

QRectF NavitAdapter::getBoundingbox() const
{
//    return QRectF(QPointF(-20037508.34, -20037508.34), QPointF(20037508.34, 20037508.34));
    return QRectF(QPointF(-180., -85.), QPointF(180., 85.));
}

QString NavitAdapter::projection() const
{
    return "EPSG:4326";
}

QPixmap NavitAdapter::getPixmap(const QRectF& wgs84Bbox, const QRectF& projBbox, const QRect& src) const
{
    QPixmap pix(src.size());
    pix.fill(Qt::transparent);

    if (!loaded)
        return pix;

    QRectF pBox(NavitProject(wgs84Bbox.topLeft()), NavitProject(wgs84Bbox.bottomRight()));

    QList<NavitFeature> theFeats;
    navit.getFeatures(pBox.toRect(), theFeats);
//    navit.getFeatures("adbdbdacddbcad", theFeats);

    qDebug () << "Feats: " << theFeats.size();
    if (!theFeats.size())
        return pix;

    QTransform tfm;

    double ScaleLon = src.width() / pBox.width();
    double ScaleLat = src.height() / pBox.height();

    double PLon = pBox.center().x() * ScaleLon;
    double PLat = pBox.center().y() * ScaleLat;
    double DeltaLon = src.width() / 2 - PLon;
    double DeltaLat = src.height() - (src.height() / 2 - PLat);

    tfm.setMatrix(ScaleLon, 0, 0, 0, -ScaleLat, 0, DeltaLon, DeltaLat, 1);

    qDebug() << pBox.toRect();
    foreach (NavitAttribute a, theFeats[0].attributes) {
        if (a.type == attr_street_name)
            qDebug() << "Street_name: " << a.attribute;
    }
//    qDebug() << theFeats[0].coordinates;
//    qDebug() << tfm.map(QPolygon(theFeats[0].coordinates));

    QPainter P(&pix);
    P.setRenderHint(QPainter::Antialiasing);
    P.setTransform(tfm);
    P.setPen(Qt::black);
    foreach (NavitFeature f, theFeats) {
//        foreach (NavitAttribute a, f.attributes) {
//            if (a.type == attr_street_name)
//                qDebug() << "Street_name: " << a.attribute;
//        }

        if (f.coordinates.size() > 1) {
            QPolygon d(f.coordinates);
            if (!pBox.intersects(d.boundingRect()));
                continue;
            P.drawPolygon(QPolygon(f.coordinates));
        } else {
            if (!pBox.contains(f.coordinates[0]))
                continue;
            P.drawPoint(f.coordinates[0]);
        }
    }
    P.end();

    return pix;
}

IImageManager* NavitAdapter::getImageManager()
{
    return NULL;
}

void NavitAdapter::setImageManager(IImageManager* anImageManager)
{
}

void NavitAdapter::cleanup()
{
}

Q_EXPORT_PLUGIN2(MWalkingPapersBackgroundPlugin, NavitAdapter)
