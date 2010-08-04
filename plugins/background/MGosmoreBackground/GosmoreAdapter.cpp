//***************************************************************
// CLass: GosmoreAdapter
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//******************************************************************

#include "GosmoreAdapter.h"

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
#include <vector>
#include <stack>
#include <queue>

static const QUuid theUid ("{7b7185c5-46cc-4b67-85b7-7aeb7fb49f31}");

#define FILTER_OPEN_SUPPORTED \
    tr("Supported formats")+" (*.pak)\n" \
    +tr("All Files (*)")

struct linePtType {
    QPoint pt;
    int cumulative;
    linePtType (QPoint _pt, int _c) : pt (_pt), cumulative (_c) {}
};


#define Depth(lon,lat) \
  (int)(yadj + (lat) * (myint) cosa - (lon) * (myint) sina)
#define X1(lon,lat) \
  (int)(xadj + (lon) * (myint) cosa + (lat) * (myint) sina)
#define AdjDepth(lon,lat) (Depth (lon, lat) < PIX45 * HEIGHT * MUL / 5000 \
  && Depth (lon, lat) > -PIX45 * HEIGHT * MUL / 5000 ? \
  PIX45 * HEIGHT * MUL / 5000 : Depth (lon, lat))
#define Y(lon,lat) (Display3D ? PIX45 * HEIGHT * MUL / AdjDepth (lon, lat) \
: yadj + (int)(((lon) * (__int64) sina - (lat) * (__int64) cosa) >> 32))
#define X(lon,lat) (Display3D ? clip.width / 2 + \
 ((AdjDepth (lon, lat) > 0 ? 1 : -1) * \
    (X1 (lon, lat) / 32000 - AdjDepth (lon, lat) / XFix) > 0 ? 32000 : \
  (AdjDepth (lon, lat) > 0 ? 1 : -1) * \
    (X1 (lon, lat) / 32000 + AdjDepth (lon, lat) / XFix) < 0 ? -32000 : \
 X1(lon,lat) / (AdjDepth (lon, lat) / XFix)) \
: xadj + (int)(((lon) * (__int64) cosa + (lat) * (__int64) sina) >> 32))

double angToRad(double a)
{
    return a*M_PI/180.;
}

QPoint gosmoreProject(const QPointF& c)
{
    double x = c.x() / 180.* INT_MAX;
    double y = log(tan(angToRad(c.y())) + 1/cos(angToRad(c.y()))) / M_PI * (INT_MAX);

    return QPoint(qRound(x), qRound(y));
}

GosmoreAdapter::GosmoreAdapter()
    : pak(0)
{
    QAction* loadFile = new QAction(tr("Load Gosmore file..."), this);
    loadFile->setData(theUid.toString());
    connect(loadFile, SIGNAL(triggered()), SLOT(onLoadFile()));
    theMenu = new QMenu();
    theMenu->addAction(loadFile);

    loaded = false;
//    setPak("d:/gosmore?.pak");
}


GosmoreAdapter::~GosmoreAdapter()
{
}

void GosmoreAdapter::setPak(QString fileName)
{
    if (pak)
        delete pak;
    pak = new QFile(fileName, this);
    if (!pak->open(QIODevice::ReadOnly)) {
        QMessageBox::critical(0,QCoreApplication::translate("GosmoreAdapter","No valid file"),QCoreApplication::translate("GosmoreAdapter","File not found."));
        return;
    }

    long sz = pak->size();
    gosmap = NULL;
    if (sz)
        gosmap = pak->map(0, sz);
    pak->close();

    if (!GosmInit(gosmap, sz)) {
        QMessageBox::critical(0,QCoreApplication::translate("GosmoreAdapter","No valid file"),QCoreApplication::translate("GosmoreAdapter","Cannot initialize file."));
        return;
    }

    loaded = true;
    return;
}

void GosmoreAdapter::onLoadFile()
{
    QString fileName = QFileDialog::getOpenFileName(
                    NULL,
                    tr("Open Gosmore file"),
                    "", FILTER_OPEN_SUPPORTED);
    if (fileName.isEmpty())
        return;

    setPak(fileName);
}

QString	GosmoreAdapter::getHost() const
{
    return "";
}

QUuid GosmoreAdapter::getId() const
{
    return QUuid(theUid);
}

IMapAdapter::Type GosmoreAdapter::getType() const
{
    return IMapAdapter::DirectBackground;
}

QString	GosmoreAdapter::getName() const
{
    return "Gosmore";
}

QMenu* GosmoreAdapter::getMenu() const
{
    return theMenu;
}

QRectF GosmoreAdapter::getBoundingbox() const
{
//    return QRectF(QPointF(-20037508.34, -20037508.34), QPointF(20037508.34, 20037508.34));
    return QRectF(QPointF(-180., -85.), QPointF(180., 85.));
}

QString GosmoreAdapter::projection() const
{
    return "EPSG:4326";
}

int WaySizeCmp (ndType **a, ndType **b)
{
  return Way (*a)->dlat * (__int64) Way (*a)->dlon >
         Way (*b)->dlat * (__int64) Way (*b)->dlon ? 1 : -1;
}

void SetColour (QColor *c, int hexTrip)
{
    int red =    (hexTrip >> 16)        * 0x101;
    int green = ((hexTrip >> 8) & 0xff) * 0x101;
    int blue =   (hexTrip       & 0xff) * 0x101;
    c->setRgb(qRgb(red, green, blue));
}

QPixmap GosmoreAdapter::getPixmap(const QRectF& wgs84Bbox, const QRectF& /*projBbox*/, const QRect& src) const
{

    if (!loaded)
        return QPixmap();

    QPixmap pix(src.size());
    pix.fill(Qt::transparent);
    QPainter P(&pix);
    P.setRenderHint(QPainter::Antialiasing);

    QPoint tl = gosmoreProject(wgs84Bbox.topLeft());
    QPoint br = gosmoreProject(wgs84Bbox.bottomRight());
    QRectF pBox(tl, br);

    QTransform tfm;

    double ScaleLon = src.width() / pBox.width();
    double ScaleLat = src.height() / pBox.height();

    double PLon = pBox.center().x() * ScaleLon;
    double PLat = pBox.center().y() * ScaleLat;
    double DeltaLon = src.width() / 2 - PLon;
    double DeltaLat = src.height() - (src.height() / 2 - PLat);

    tfm.setMatrix(ScaleLon, 0, 0, 0, -ScaleLat, 0, DeltaLon, DeltaLat, 1);

//    P.setTransform(tfm);

    static QColor styleColour[2 << STYLE_BITS][2];
    for (int i = 0; i < stylecount; i++) {
        for (int j = 0; j < 2; j++) {
            SetColour (&styleColour[i][j],
                       !j ? style[i].areaColour
                           : style[i].lineColour != -1 ? style[i].lineColour
                               : (style[i].areaColour >> 1) & 0xefefef); // Dark border for polys
        }
    }

    OsmItr itr (tl.x() - 1000, tl.y() - 1000,
                br.x() + 1000, br.y() + 1000);

    std::vector<ndType*> area;
    std::stack<ndType*> dlist[12];
    long cnt = 0;
    while (Next (itr)) {
        ndType *nd = itr.nd[0];
        wayType *w = Way (nd);

//        qDebug() << "Max style: " << Style (w)->scaleMax;
//        if (Style (w)->scaleMax < zoom / clip.width * 350 / (DetailLevel + 6)
//            && !Display3D && w->dlat < zoom / clip.width * 20 &&
//                             w->dlon < zoom / clip.width * 20) continue;
        // With 3D, the icons are filtered only much later when we know z.
        if (nd->other[0] != 0) {
          nd = itr.nd[0] + itr.nd[0]->other[0];
          if (nd->lat == INT_MIN) nd = itr.nd[0]; // Node excluded from build
          else if (itr.left <= nd->lon && nd->lon < itr.right &&
              itr.top  <= nd->lat && nd->lat < itr.bottom) continue;
        } // Only process this way when the Itr gives us the first node, or
        // the first node that's inside the viewing area
        if (nd->other[0] == 0 && nd->other[1] == 0) dlist[11].push (nd);
        else if (Style (w)->areaColour != -1) area.push_back (nd);
        else dlist[Layer (w) + 5].push (nd);
        ++cnt;
      }
//    qDebug() << "cnt: " << cnt;

    qsort (&area[0], area.size (), sizeof (area[0]),
      (int (*)(const void *a, const void *b))WaySizeCmp);

    for (; !area.empty(); area.pop_back ()) {
        ndType *nd = area.back ();
        wayType *w = Way (nd);
        while (nd->other[0] != 0) nd += nd->other[0];
        QPainterPath pth;
        bool hasFirst = false;
        for (; nd->other[1] != 0; nd += nd->other[1]) {
            if (nd->lat != INT_MIN) {
                if (hasFirst)
                    pth.lineTo(nd->lon, nd->lat);
                else {
                    pth.moveTo(nd->lon, nd->lat);
                    hasFirst = true;
                }
            }
        }
        pth.closeSubpath();

        if (!pth.isEmpty()) {
            P.setBrush(styleColour[Style (w) - style][0]);
            P.setPen(Qt::NoPen);
            P.drawPath(tfm.map(pth));
            P.setBrush(styleColour[Style (w) - style][1]);
            P.strokePath(tfm.map(pth), QPen(Style (w)->lineWidth));

            // Text placement: The basic idea is here : http://alienryderflex.com/polygon_fill/
        } // Polygon not empty
    } // For each area

    for (int l = 0; l < 12; l++) {
      for (; !dlist[l].empty (); dlist[l].pop ()) {
        ndType *nd = dlist[l].top ();
        wayType *w = Way (nd);

    // single-point node
        if (nd->other[0] == 0 && nd->other[1] == 0) {
            P.setPen(QPen(Qt::red, 3));
            QPoint pt(nd->lon, nd->lat);
            P.drawPoint(tfm.map(pt));

        }
    // ways (including areas on WinMob : FIXME)
        else if (nd->other[1] != 0) {
      // perform validation (on non-areas)
      ndType *orig = nd;
      nd = orig;

      QPainterPath pth;
      pth.moveTo(nd->lon, nd->lat);
      do {
          ndType *next = nd + nd->other[1];
          if (next->lat == INT_MIN) break; // Node excluded from build
          pth.lineTo(next->lon, next->lat);
          nd = next;
      } while (itr.left <= nd->lon && nd->lon < itr.right &&
               itr.top  <= nd->lat && nd->lat < itr.bottom &&
               nd->other[1] != 0);

//      P.setBrush(styleColour[firstElemStyle][1]);
//      P.setPen(10);
//      P.drawPath(tfm.map(pth));
      P.setBrush(Qt::NoBrush);
      P.setPen(QPen(styleColour[Style (w) - style][1], Style (w)->lineWidth));
      P.drawPath(tfm.map(pth));

  } /* If it has one or more segments */

      } /* for each way / icon */
    } // For each layer



//    qDebug() << "wgs84: " << wgs84Bbox;
//    qDebug() << "src: " << src;
////    QRectF pBox(GosmoreProject(wgs84Bbox.topLeft()), GosmoreProject(wgs84Bbox.bottomRight()));
//    QRectF pBox(GosmoreProject(wgs84Bbox.topLeft()), GosmoreProject(wgs84Bbox.bottomRight()));

//    QList<GosmoreFeature> theFeats;
//    Gosmore.getFeatures(pBox.toRect(), theFeats);
////    Gosmore.getFeatures("adbdbdacddbcad", theFeats);

//    qDebug () << "Feats: " << theFeats.size();
//    if (!theFeats.size())
//        return QPixmap();

//    QTransform tfm;

//    double ScaleLon = src.width() / pBox.width();
//    double ScaleLat = src.height() / pBox.height();

//    double PLon = pBox.center().x() * ScaleLon;
//    double PLat = pBox.center().y() * ScaleLat;
//    double DeltaLon = src.width() / 2 - PLon;
//    double DeltaLat = src.height() - (src.height() / 2 - PLat);

//    tfm.setMatrix(ScaleLon, 0, 0, 0, -ScaleLat, 0, DeltaLon, DeltaLat, 1);

//    qDebug() << pBox.toRect();
//    foreach (GosmoreAttribute a, theFeats[0].attributes) {
//        if (a.type == attr_street_name)
//            qDebug() << "Street_name: " << a.attribute;
//    }
////    qDebug() << "type: " << QString("0x%1").arg(theFeats[0].type, 0, 16);
////    qDebug() << theFeats[10].coordinates;
////    qDebug() << QPolygon(theFeats[10].coordinates).boundingRect();
////    qDebug() << tfm.map(QPolygon(theFeats[10].coordinates));

//    pix.fill(Qt::transparent);
//    QPainter P(&pix);
//    P.setRenderHint(QPainter::Antialiasing);
//    P.setTransform(tfm);
////    P.setClipRect(pBox);
////    P.setClipping(true);

////    QRect ipBox = pBox.toRect();
//    QPainterPath clipPath;
//    clipPath.addRect(pBox);
//    foreach (GosmoreFeature f, theFeats) {
////        foreach (GosmoreAttribute a, f.attributes) {
////            if (a.type == attr_street_name)
////                qDebug() << "Street_name: " << a.attribute;
////        }

//        if (f.coordinates.size() > 1) {
//            QPolygonF d(f.coordinates);
//            QPainterPath aPath;
//            aPath.addPolygon(d);
////            QRect br = d.boundingRect();
////            qDebug() << "brect: " << br;
//            aPath = aPath.intersected(clipPath);
////            if (!aPath.intersects(pBox))
////                continue;
//            if ((f.type & 0xc0000000) == 0xc0000000) {
//                P.setPen(QPen(Qt::lightGray, 1));
//                aPath.closeSubpath();
//                P.drawPath(aPath);
////                P.drawPolygon(QPolygon(f.coordinates));
//            } else {
//                P.setPen(QPen(Qt::blue, 2));
////                P.drawPolyline(f.coordinates);
//                P.drawPath(aPath);
//            }
//        } else {
//            if (f.coordinates.size() == 1) {
//                if (!pBox.contains(f.coordinates[0]))
//                    continue;
//                P.setPen(QPen(Qt::red, 5));
//                P.drawPoint(f.coordinates[0]);
//            }
//        }
//    }
    P.end();

    return pix;
}

IImageManager* GosmoreAdapter::getImageManager()
{
    return NULL;
}

void GosmoreAdapter::setImageManager(IImageManager*)
{
}

void GosmoreAdapter::cleanup()
{
}

bool GosmoreAdapter::toXML(QDomElement xParent)
{
    bool OK = true;

    QDomElement fs = xParent.ownerDocument().createElement("Images");
    xParent.appendChild(fs);
    if (pak)
        fs.setAttribute("filename", pak->fileName());

    return OK;
}

void GosmoreAdapter::fromXML(const QDomElement xParent)
{
    QDomElement fs = xParent.firstChildElement();
    while(!fs.isNull()) {
        if (fs.tagName() == "Images") {
            QString fn = fs.attribute("filename");
            setPak(fn);
        }

        fs = fs.nextSiblingElement();
    }
}

QString GosmoreAdapter::toPropertiesHtml()
{
    QString h;

    if (pak)
        h += "<i>" + tr("Filename") + ": </i>" + pak->fileName();

    return h;
}

Q_EXPORT_PLUGIN2(MGosmoreBackgroundPlugin, GosmoreAdapter)
