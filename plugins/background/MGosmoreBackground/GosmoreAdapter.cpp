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
#include <QSettings>

#include <math.h>
#include <vector>
#include <stack>
#include <queue>

#include "MasPaintStyle.h"
#include "GosmoreFeature.h"

static const QUuid theUid ("{7b7185c5-46cc-4b67-85b7-7aeb7fb49f31}");
static const QString theName("Gosmore");

QUuid GosmoreAdapterFactory::getId() const
{
    return theUid;
}

QString	GosmoreAdapterFactory::getName() const
{
    return theName;
}

/********************************/


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
//    setFile("d:/gosmore?.pak");

    MasPaintStyle theStyle;
    theStyle.loadPainters(":/Styles/Mapnik.mas");
    for (int i=0; i<theStyle.painterSize(); ++i) {
        thePrimitivePainters.append(PrimitivePainter(*theStyle.getPainter(i)));
    }

    loadStyle(gosmore_note_yes);
    loadStyle(highway_residential);
    loadStyle(highway_unclassified);
    loadStyle(highway_tertiary);
    loadStyle(highway_secondary);
    loadStyle(highway_primary);
    loadStyle(highway_trunk);
    loadStyle(highway_footway);
    loadStyle(highway_service);
    loadStyle(highway_track);
    loadStyle(highway_cycleway);
    loadStyle(highway_pedestrian);
    loadStyle(highway_steps);
    loadStyle(highway_bridleway);
    loadStyle(railway_rail);
    loadStyle(railway_station);
    loadStyle(highway_mini_roundabout);
    loadStyle(highway_traffic_signals);
    loadStyle(highway_bus_stop);
    loadStyle(amenity_parking);
    loadStyle(amenity_fuel);
    loadStyle(amenity_school);
    loadStyle(place_village);
    loadStyle(place_suburb);
    loadStyle(shop_supermarket);
    loadStyle(religion_christian);
    loadStyle(religion_jewish);
    loadStyle(religion_muslim);
    loadStyle(amenity_pub);
    loadStyle(amenity_restaurant);
    loadStyle(power_tower);
    loadStyle(waterway_stream);
    loadStyle(amenity_grave_yard);
    loadStyle(amenity_crematorium);
    loadStyle(amenity_shelter);
    loadStyle(tourism_picnic_site);
    loadStyle(leisure_common);
    loadStyle(amenity_park_bench);
    loadStyle(tourism_viewpoint);
    loadStyle(tourism_artwork);
    loadStyle(tourism_museum);
    loadStyle(tourism_theme_park);
    loadStyle(tourism_zoo);
    loadStyle(leisure_playground);
    loadStyle(leisure_park);
    loadStyle(leisure_nature_reserve);
    loadStyle(leisure_miniature_golf);
    loadStyle(leisure_golf_course);
    loadStyle(leisure_sports_centre);
    loadStyle(leisure_stadium);
    loadStyle(leisure_pitch);
    loadStyle(leisure_track);
    loadStyle(sport_athletics);
    loadStyle(sport_10pin);
    loadStyle(sport_boules);
    loadStyle(sport_bowls);
    loadStyle(sport_baseball);
    loadStyle(sport_basketball);
    loadStyle(sport_cricket);
    loadStyle(sport_cricket_nets);
    loadStyle(sport_croquet);
    loadStyle(sport_dog_racing);
    loadStyle(sport_equestrian);
    loadStyle(sport_football);
    loadStyle(sport_soccer);
    loadStyle(sport_climbing);
    loadStyle(sport_gymnastics);
    loadStyle(sport_hockey);
    loadStyle(sport_horse_racing);
    loadStyle(sport_motor);
    loadStyle(sport_pelota);
    loadStyle(sport_rugby);
    loadStyle(sport_australian_football);
    loadStyle(sport_skating);
    loadStyle(sport_skateboard);
    loadStyle(sport_handball);
    loadStyle(sport_table_tennis);
    loadStyle(sport_tennis);
    loadStyle(sport_racquet);
    loadStyle(sport_badminton);
    loadStyle(sport_paintball);
    loadStyle(sport_shooting);
    loadStyle(sport_volleyball);
    loadStyle(sport_beachvolleyball);
    loadStyle(sport_archery);
    loadStyle(sport_skiing);
    loadStyle(sport_rowing);
    loadStyle(sport_sailing);
    loadStyle(sport_diving);
    loadStyle(sport_swimming);
    loadStyle(leisure_swimming_pool);
    loadStyle(leisure_water_park);
    loadStyle(leisure_marina);
    loadStyle(leisure_slipway);
    loadStyle(leisure_fishing);
    loadStyle(shop_bakery);
    loadStyle(shop_butcher);
    loadStyle(shop_florist);
    loadStyle(shop_groceries);
    loadStyle(shop_beverages);
    loadStyle(shop_clothes);
    loadStyle(shop_shoes);
    loadStyle(shop_jewelry);
    loadStyle(shop_books);
    loadStyle(shop_newsagent);
    loadStyle(shop_furniture);
    loadStyle(shop_hifi);
    loadStyle(shop_electronics);
    loadStyle(shop_computer);
    loadStyle(shop_video);
    loadStyle(shop_toys);
    loadStyle(shop_motorcycle);
    loadStyle(shop_car_repair);
    loadStyle(shop_doityourself);
    loadStyle(shop_garden_centre);
    loadStyle(shop_outdoor);
    loadStyle(shop_bicycle);
    loadStyle(shop_dry_cleaning);
    loadStyle(shop_laundry);
    loadStyle(shop_hairdresser);
    loadStyle(shop_travel_agency);
    loadStyle(shop_convenience);
    loadStyle(shop_mall);
    loadStyle(shop_department_store);
    loadStyle(amenity_biergarten);
    loadStyle(amenity_nightclub);
    loadStyle(amenity_bar);
    loadStyle(amenity_cafe);
    loadStyle(amenity_fast_food);
    loadStyle(amenity_ice_cream);
    loadStyle(amenity_bicycle_rental);
    loadStyle(amenity_car_rental);
    loadStyle(amenity_car_sharing);
    loadStyle(amenity_car_wash);
    loadStyle(amenity_taxi);
    loadStyle(amenity_telephone);
    loadStyle(amenity_post_office);
    loadStyle(amenity_post_box);
    loadStyle(tourism_information);
    loadStyle(amenity_toilets);
    loadStyle(amenity_recycling);
    loadStyle(amenity_fire_station);
    loadStyle(amenity_police);
    loadStyle(amenity_courthouse);
    loadStyle(amenity_prison);
    loadStyle(amenity_public_building);
    loadStyle(amenity_townhall);
    loadStyle(amenity_cinema);
    loadStyle(amenity_arts_centre);
    loadStyle(amenity_theatre);
    loadStyle(tourism_hotel);
    loadStyle(tourism_motel);
    loadStyle(tourism_guest_house);
    loadStyle(tourism_hostel);
    loadStyle(tourism_chalet);
    loadStyle(tourism_camp_site);
    loadStyle(tourism_caravan_site);
    loadStyle(amenity_pharmacy);
    loadStyle(amenity_dentist);
    loadStyle(amenity_doctor);
    loadStyle(amenity_hospital);
    loadStyle(amenity_bank);
    loadStyle(amenity_bureau_de_change);
    loadStyle(amenity_atm);
    loadStyle(amenity_drinking_water);
    loadStyle(amenity_fountain);
    loadStyle(natural_spring);
    loadStyle(amenity_university);
    loadStyle(amenity_college);
    loadStyle(amenity_kindergarten);
    loadStyle(highway_living_street);
    loadStyle(highway_motorway);
    loadStyle(highway_motorway_link);
    loadStyle(highway_trunk_link);
    loadStyle(highway_primary_link);
    loadStyle(barrier_bollard);
    loadStyle(barrier_gate);
    loadStyle(barrier_stile);
    loadStyle(barrier_cattle_grid);
    loadStyle(barrier_toll_booth);
    loadStyle(man_made_beacon);
    loadStyle(man_made_survey_point);
    loadStyle(man_made_tower);
    loadStyle(man_made_water_tower);
    loadStyle(man_made_gasometer);
    loadStyle(man_made_reservoir_covered);
    loadStyle(man_made_lighthouse);
    loadStyle(man_made_windmill);
    loadStyle(man_made_pier);
    loadStyle(man_made_pipeline);
    loadStyle(man_made_wastewater_plant);
    loadStyle(man_made_crane);
    loadStyle(building_yes);
    loadStyle(landuse_forest);
    loadStyle(landuse_residential);
    loadStyle(landuse_industrial);
    loadStyle(landuse_retail);
    loadStyle(landuse_commercial);
    loadStyle(landuse_construction);
    loadStyle(landuse_reservoir);
    loadStyle(natural_water);
    loadStyle(landuse_basin);
    loadStyle(landuse_landfill);
    loadStyle(landuse_quarry);
    loadStyle(landuse_cemetery);
    loadStyle(landuse_allotments);
    loadStyle(landuse_farm);
    loadStyle(landuse_farmyard);
    loadStyle(landuse_military);
    loadStyle(religion_bahai);
    loadStyle(religion_buddhist);
    loadStyle(religion_hindu);
    loadStyle(religion_jain);
    loadStyle(religion_sikh);
    loadStyle(religion_shinto);
    loadStyle(religion_taoist);
    loadStyle(highway_road);
    loadStyle(restriction_no_right_turn);
    loadStyle(restriction_no_left_turn);
    loadStyle(restriction_no_u_turn);
    loadStyle(restriction_no_straight_on);
    loadStyle(restriction_only_right_turn);
    loadStyle(restriction_only_left_turn);
    loadStyle(restriction_only_straight_on);
}

void GosmoreAdapter::loadStyle(int stylenr)
{
    GosmoreFeature f(stylenr);
    for (int i=0; i<thePrimitivePainters.size(); ++i) {
        if (thePrimitivePainters[i].matchesTag(&f, 0))
            myStyles[stylenr] = &thePrimitivePainters[i];
    }
}

GosmoreAdapter::~GosmoreAdapter()
{
}

void GosmoreAdapter::setFile(const QString& fn)
{
    delete pak;
    pak = new QFile(fn, this);
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

    setFile(fileName);
}

QString	GosmoreAdapter::getHost() const
{
    return QString();
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
    return theName;
}

QMenu* GosmoreAdapter::getMenu() const
{
    return theMenu;
}

QRectF GosmoreAdapter::getBoundingbox() const
{
    return QRectF(QPointF(-6378137.0/2, -6378137.0/2), QPointF(6378137.0/2, 6378137.0/2));
//    return QRectF(QPointF(-180., -85.), QPointF(180., 85.));
}

QString GosmoreAdapter::projection() const
{
    return "EPSG:3857";
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

    render(&P, wgs84Bbox, wgs84Bbox, src);

    P.end();
    return pix;
}

void GosmoreAdapter::render(QPainter* P, const QRectF& fullbox, const QRectF& selbox, const QRect& src) const
{
    if (!loaded)
        return;

    QPoint tl = gosmoreProject(fullbox.topLeft());
    QPoint br = gosmoreProject(fullbox.bottomRight());
    QRectF pBox(tl, br);

    QTransform tfm;

    double ScaleLon = src.width() / pBox.width();
    double ScaleLat = src.height() / pBox.height();

    double PLon = pBox.center().x() * ScaleLon;
    double PLat = pBox.center().y() * ScaleLat;
    double DeltaLon = src.width() / 2 - PLon;
    double DeltaLat = src.height() - (src.height() / 2 - PLat);

    double LengthOfOneDegreeLat = 6378137.0 * M_PI / 180;
    double LengthOfOneDegreeLon = LengthOfOneDegreeLat * fabs(cos(angToRad(fullbox.center().y())));
    double lonAnglePerM =  1 / LengthOfOneDegreeLon;
    double PixelPerM = src.width() / (double)fullbox.width() * lonAnglePerM;
//    qDebug() << PixelPerM;

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

    tl = gosmoreProject(selbox.topLeft());
    br = gosmoreProject(selbox.bottomRight());
    pBox = QRectF(tl, br).adjusted(-1000, -1000, 1000, 1000);
    QPainterPath clipPath;
    clipPath.addRect(pBox);

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
//            QPainterPath pp = tfm.map(pth);
            QPainterPath pp = tfm.map(clipPath.intersected(pth));
            if (myStyles[StyleNr(w)]) {
                if (myStyles[StyleNr(w)]->matchesZoom(PixelPerM)) {
                    myStyles[StyleNr(w)]->drawBackground(&pp, P, PixelPerM);
//                    myStyles[StyleNr(w)]->drawForeground(&pp, P, PixelPerM);
                    myStyles[StyleNr(w)]->drawTouchup(&pp, P, PixelPerM);
                    //          myStyles[StyleNr(w)]->drawLabel(&pp, P, PixelPerM);
                }
            } else {
                P->setPen(QPen(Qt::black, 2));
                P->drawPath(pp);
            }
//            P.setBrush(styleColour[Style (w) - style][0]);
//            P.setPen(Qt::NoPen);
//            P.drawPath(tfm.map(pth));
//            P.setBrush(styleColour[Style (w) - style][1]);
//            P.strokePath(tfm.map(pth), QPen(Style (w)->lineWidth));

            // Text placement: The basic idea is here : http://alienryderflex.com/polygon_fill/
        } // Polygon not empty
    } // For each area

    for (int l = 0; l < 12; l++) {
      for (; !dlist[l].empty (); dlist[l].pop ()) {
        ndType *nd = dlist[l].top ();
        wayType *w = Way (nd);
        QStringList strL(QString(QByteArray((char*)(w + 1) + 1)).split("\n"));

    // single-point node
        if (nd->other[0] == 0 && nd->other[1] == 0) {
            QPointF pt(nd->lon, nd->lat);
            //            P.setPen(QPen(Qt::red, 3));
            //            P.drawPoint(tfm.map(pt));

            if (myStyles[StyleNr(w)] && myStyles[StyleNr(w)]->matchesZoom(PixelPerM)) {
                QPointF pp = tfm.map(pt);
                myStyles[StyleNr(w)]->drawTouchup(&pp, P, PixelPerM);
                myStyles[StyleNr(w)]->drawLabel(&pp, P, PixelPerM, strL[0]);
            }
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

        //      P.setBrush(Qt::NoBrush);
        //      P.setPen(QPen(styleColour[Style (w) - style][1], Style (w)->lineWidth));
        //      P.drawPath(tfm.map(pth));

//            QPainterPath pp = tfm.map(pth);
            QPainterPath pp = tfm.map(clipPath.intersected(pth));
            if (myStyles[StyleNr(w)]) {
                if (myStyles[StyleNr(w)]->matchesZoom(PixelPerM)) {
//                    P->save();
//                    P->setCompositionMode(QPainter::CompositionMode_DestinationOver);
//                    myStyles[StyleNr(w)]->drawBackground(&pp, P, PixelPerM);
//                    P->restore();
                    myStyles[StyleNr(w)]->drawForeground(&pp, P, PixelPerM);
                    myStyles[StyleNr(w)]->drawTouchup(&pp, P, PixelPerM);
                    //                myStyles[StyleNr(w)]->drawLabel(&pp, &P, PixelPerM, strL[0]);
                }
            } else {
//                qDebug() << StyleNr(w);
                P->setPen(QPen(Qt::black, 2));
                P->drawPath(pp);
            }

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

bool GosmoreAdapter::toXML(QXmlStreamWriter& stream)
{
    bool OK = true;
    stream.writeStartElement("Source");
    if (loaded)
        stream.writeAttribute("filename", pak->fileName());
    stream.writeEndElement();

    return OK;
}

void GosmoreAdapter::fromXML(QXmlStreamReader& stream)
{
    QString fn;

    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "Source") {
            fn = stream.attributes().value("filename").toString();
        }
        stream.readNext();
    }

    if (!fn.isEmpty())
        setFile(fn);
}

QString GosmoreAdapter::toPropertiesHtml()
{
    QString h;

    if (pak)
        h += "<i>" + tr("Filename") + ": </i>" + pak->fileName();

    return h;
}

#ifndef _MOBILE
#if !(QT_VERSION >= QT_VERSION_CHECK(5,0,0))
Q_EXPORT_PLUGIN2(MGosmoreBackgroundPlugin, GosmoreAdapterFactory)
#endif
#endif
