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
#include <QStringList>

#include <math.h>

#include "MasPaintStyle.h"

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

static QString attrmap(
    "n	*=*			point_unkn\n"
    "n	Annehmlichkeit=Hochsitz	poi_hunting_stand\n"
    "n	addr:housenumber=*	house_number\n"
    "n	aeroway=aerodrome	poi_airport\n"
    "n	aeroway=airport		poi_airport\n"
    "n	aeroway=helipad		poi_heliport\n"
    "n	aeroway=terminal	poi_airport\n"
    "n	amenity=atm		poi_bank\n"
    "n	amenity=bank		poi_bank\n"
    "n	amenity=bench		poi_bench\n"
    "n	amenity=biergarten	poi_biergarten\n"
    "n	amenity=bus_station	poi_bus_station\n"
    "n	amenity=cafe		poi_cafe\n"
    "n	amenity=cinema		poi_cinema\n"
    "n	amenity=college		poi_school_college\n"
    "n	amenity=courthouse	poi_justice\n"
    "n	amenity=drinking_water	poi_potable_water\n"
    "n	amenity=fast_food	poi_fastfood\n"
    "n	amenity=fire_station	poi_firebrigade\n"
    "n	amenity=fountain	poi_fountain\n"
    "n	amenity=fuel		poi_fuel\n"
    "n	amenity=grave_yard	poi_cemetery\n"
    "n	amenity=hospital	poi_hospital\n"
    "n	amenity=hunting_stand	poi_hunting_stand\n"
    "n	amenity=kindergarten	poi_kindergarten\n"
    "n	amenity=library		poi_library\n"
    "n	amenity=nightclub	poi_nightclub\n"
    "n	amenity=park_bench	poi_bench\n"
    "n	amenity=parking		poi_car_parking\n"
    "n	amenity=pharmacy	poi_pharmacy\n"
    "n	amenity=place_of_worship,religion=christian	poi_church\n"
    "n	amenity=police		poi_police\n"
    "n	amenity=post_box	poi_post_box\n"
    "n	amenity=post_office	poi_post_office\n"
    "n	amenity=prison		poi_prison\n"
    "n	amenity=pub		poi_bar\n"
    "n	amenity=public_building	poi_public_office\n"
    "n	amenity=recycling	poi_recycling\n"
    "n	amenity=restaurant	poi_restaurant\n"
    "n	amenity=school		poi_school\n"
    "n	amenity=shelter		poi_shelter\n"
    "n	amenity=taxi		poi_taxi\n"
    "n	amenity=tec_common	tec_common\n"
    "n	amenity=telephone	poi_telephone\n"
    "n	amenity=theatre		poi_theater\n"
    "n	amenity=toilets		poi_restroom\n"
    "n	amenity=toilets		poi_toilets\n"
    "n	amenity=townhall	poi_townhall\n"
    "n	amenity=university	poi_school_university\n"
    "n	amenity=vending_machine	poi_vending_machine\n"
    "n	barrier=bollard		barrier_bollard\n"
    "n	barrier=cycle_barrier	barrier_cycle\n"
    "n	barrier=lift_gate	barrier_lift_gate\n"
    "n	car=car_rental		poi_car_rent\n"
    "n	highway=bus_station	poi_bus_station\n"
    "n	highway=bus_stop	poi_bus_stop\n"
    "n	highway=mini_roundabout	mini_roundabout\n"
    "n	highway=motorway_junction	highway_exit\n"
    "n	highway=stop		traffic_sign_stop\n"
    "n	highway=traffic_signals	traffic_signals\n"
    "n	highway=turning_circle	turning_circle\n"
    "n	historic=boundary_stone	poi_boundary_stone\n"
    "n	historic=castle		poi_castle\n"
    "n	historic=memorial	poi_memorial\n"
    "n	historic=monument	poi_monument\n"
    "n	historic=*		poi_ruins\n"
    "n	landuse=cemetery	poi_cemetery\n"
    "n	leisure=fishing		poi_fish\n"
    "n	leisure=golf_course	poi_golf\n"
    "n	leisure=marina		poi_marine\n"
    "n	leisure=playground	poi_playground\n"
    "n	leisure=slipway		poi_boat_ramp\n"
    "n	leisure=sports_centre	poi_sport\n"
    "n	leisure=stadium		poi_stadium\n"
    "n	man_made=tower		poi_tower\n"
    "n	military=airfield	poi_military\n"
    "n	military=barracks	poi_military\n"
    "n	military=bunker		poi_military\n"
    "n	military=danger_area	poi_danger_area\n"
    "n	military=range		poi_military\n"
    "n	natural=bay		poi_bay\n"
    "n	natural=peak		poi_peak\n"
    "n	natural=tree		poi_tree\n"
    "n	place=city		town_label_2e5\n"
    "n	place=hamlet		town_label_2e2\n"
    "n	place=locality		town_label_2e0\n"
    "n	place=suburb		district_label\n"
    "n	place=town		town_label_2e4\n"
    "n	place=village		town_label_2e3\n"
    "n	power=tower		power_tower\n"
    "n	power=sub_station	power_substation\n"
    "n	railway=halt		poi_rail_halt\n"
    "n	railway=level_crossing	poi_level_crossing\n"
    "n	railway=station		poi_rail_station\n"
    "n	railway=tram_stop	poi_rail_tram_stop\n"
    "n	shop=baker		poi_shop_baker\n"
    "n	shop=bakery		poi_shop_baker\n"
    "n	shop=beverages		poi_shop_beverages\n"
    "n	shop=bicycle		poi_shop_bicycle\n"
    "n	shop=butcher		poi_shop_butcher\n"
    "n	shop=car		poi_car_dealer_parts\n"
    "n	shop=car_repair		poi_repair_service\n"
    "n	shop=clothes		poi_shop_apparel\n"
    "n	shop=convenience	poi_shop_grocery\n"
    "n	shop=drogist		poi_shop_drugstore\n"
    "n	shop=florist		poi_shop_florist\n"
    "n	shop=fruit		poi_shop_fruit\n"
    "n	shop=furniture		poi_shop_furniture\n"
    "n	shop=garden_centre	poi_shop_handg\n"
    "n	shop=hardware		poi_shop_handg\n"
    "n	shop=hairdresser	poi_hairdresser\n"
    "n	shop=kiosk		poi_shop_kiosk\n"
    "n	shop=optician		poi_shop_optician\n"
    "n	shop=parfum		poi_shop_parfum\n"
    "n	shop=photo		poi_shop_photo\n"
    "n	shop=shoes		poi_shop_shoes\n"
    "n	shop=supermarket	poi_shopping\n"
    "n	sport=baseball		poi_baseball\n"
    "n	sport=basketball	poi_basketball\n"
    "n	sport=climbing		poi_climbing\n"
    "n	sport=golf		poi_golf\n"
    "n	sport=motor_sports	poi_motor_sport\n"
    "n	sport=skiing		poi_skiing\n"
    "n	sport=soccer		poi_soccer\n"
    "n	sport=stadium		poi_stadium\n"
    "n	sport=swimming		poi_swimming\n"
    "n	sport=tennis		poi_tennis\n"
    "n	tourism=attraction	poi_attraction\n"
    "n	tourism=camp_site	poi_camp_rv\n"
    "n	tourism=caravan_site	poi_camp_rv\n"
    "n	tourism=guest_house	poi_guesthouse\n"
    "n	tourism=hostel		poi_hostel\n"
    "n	tourism=hotel		poi_hotel\n"
    "n	tourism=information	poi_information\n"
    "n	tourism=motel		poi_motel\n"
    "n	tourism=museum		poi_museum_history\n"
    "n	tourism=picnic_site	poi_picnic\n"
    "n	tourism=theme_park	poi_resort\n"
    "n	tourism=viewpoint	poi_viewpoint\n"
    "n	tourism=zoo		poi_zoo\n"
    "n	traffic_sign=city_limit	traffic_sign_city_limit\n"
    "w	*=*			street_unkn\n"
    "w	addr:interpolation=even	house_number_interpolation_even\n"
    "w	addr:interpolation=odd	house_number_interpolation_odd\n"
    "w	addr:interpolation=all	house_number_interpolation_all\n"
    "w	addr:interpolation=alphabetic	house_number_interpolation_alphabetic\n"
    "w	aerialway=cable_car	lift_cable_car\n"
    "w	aerialway=chair_lift	lift_chair\n"
    "w	aerialway=drag_lift	lift_drag\n"
    "w	aeroway=aerodrome	poly_airport\n"
    "w	aeroway=apron		poly_apron\n"
    "w	aeroway=runway		aeroway_runway\n"
    "w	aeroway=taxiway		aeroway_taxiway\n"
    "w	aeroway=terminal	poly_terminal\n"
    "w	amenity=college		poly_college\n"
    "w	amenity=grave_yard	poly_cemetery\n"
    "w	amenity=parking		poly_car_parking\n"
    "w	amenity=place_of_worship	poly_building\n"
    "w	amenity=university	poly_university\n"
    "w	boundary=administrative	border_country\n"
    "w	boundary=civil		border_civil\n"
    "w	boundary=national_park	border_national_park\n"
    "w	boundary=political	border_political\n"
    "w	building=*		poly_building\n"
    "w	contour_ext=elevation_major	height_line_1\n"
    "w	contour_ext=elevation_medium	height_line_2\n"
    "w	contour_ext=elevation_minor	height_line_3\n"
#if 0 /* FIXME: Implement this as attribute */
    "w	cycleway=track	        cycleway\n"
#endif
    "w	highway=bridleway	bridleway\n"
    "w	highway=bus_guideway	bus_guideway\n"
    "w	highway=construction	street_construction\n"
    "w	highway=cyclepath	cycleway\n"
    "w	highway=cycleway	cycleway\n"
    "w	highway=footway		footway\n"
    "w	highway=footway,piste:type=nordic	footway_and_piste_nordic\n"
    "w	highway=living_street	living_street\n"
    "w	highway=minor		street_1_land\n"
    "w	highway=parking_lane	street_parking_lane\n"
    "w	highway=path				path\n"
    "w	highway=path,bicycle=designated		cycleway\n"
    "w	highway=path,foot=designated		footway\n"
    "w	highway=path,horse=designated		bridleway\n"
    "w	highway=path,sac_scale=alpine_hiking			hiking_alpine\n"
    "w	highway=path,sac_scale=demanding_alpine_hiking		hiking_alpine_demanding\n"
    "w	highway=path,sac_scale=demanding_mountain_hiking	hiking_mountain_demanding\n"
    "w	highway=path,sac_scale=difficult_alpine_hiking		hiking_alpine_difficult\n"
    "w	highway=path,sac_scale=hiking				hiking\n"
    "w	highway=path,sac_scale=mountain_hiking			hiking_mountain\n"
    "w	highway=pedestrian			street_pedestrian\n"
    "w	highway=pedestrian,area=1		poly_pedestrian\n"
    "w	highway=plaza				poly_plaza\n"
    "w	highway=motorway			highway_land\n"
    "w	highway=motorway,rural=0		highway_city\n"
    "w	highway=motorway_link			ramp\n"
    "w	highway=trunk				street_4_land\n"
    "w	highway=trunk,name=*,rural=1		street_4_land\n"
    "w	highway=trunk,name=*			street_4_city\n"
    "w	highway=trunk,rural=0			street_4_city\n"
    "w	highway=trunk_link			ramp\n"
    "w	highway=primary				street_4_land\n"
    "w	highway=primary,name=*,rural=1		street_4_land\n"
    "w	highway=primary,name=*			street_4_city\n"
    "w	highway=primary,rural=0			street_4_city\n"
    "w	highway=primary_link			ramp\n"
    "w	highway=secondary			street_3_land\n"
    "w	highway=secondary,name=*,rural=1	street_3_land\n"
    "w	highway=secondary,name=*		street_3_city\n"
    "w	highway=secondary,rural=0		street_3_city\n"
    "w	highway=secondary,area=1		poly_street_3\n"
    "w	highway=secondary_link			ramp\n"
    "w	highway=tertiary			street_2_land\n"
    "w	highway=tertiary,name=*,rural=1		street_2_land\n"
    "w	highway=tertiary,name=*			street_2_city\n"
    "w	highway=tertiary,rural=0		street_2_city\n"
    "w	highway=tertiary,area=1			poly_street_2\n"
    "w	highway=tertiary_link			ramp\n"
    "w	highway=residential			street_1_city\n"
    "w	highway=residential,area=1		poly_street_1\n"
    "w	highway=unclassified			street_1_city\n"
    "w	highway=unclassified,area=1		poly_street_1\n"
    "w	highway=road				street_1_city\n"
    "w	highway=service				street_service\n"
    "w	highway=service,area=1			poly_service\n"
    "w	highway=service,service=parking_aisle	street_parking_lane\n"
    "w	highway=track				track_gravelled\n"
    "w	highway=track,surface=grass		track_grass\n"
    "w	highway=track,surface=gravel		track_gravelled\n"
    "w	highway=track,surface=ground		track_ground\n"
    "w	highway=track,surface=paved		track_paved\n"
    "w	highway=track,surface=unpaved		track_unpaved\n"
    "w	highway=track,tracktype=grade1		track_paved\n"
    "w	highway=track,tracktype=grade2		track_gravelled\n"
    "w	highway=track,tracktype=grade3		track_unpaved\n"
    "w	highway=track,tracktype=grade4		track_ground\n"
    "w	highway=track,tracktype=grade5		track_grass\n"
    "w	highway=track,surface=paved,tracktype=grade1		track_paved\n"
    "w	highway=track,surface=gravel,tracktype=grade2		track_gravelled\n"
    "w	highway=track,surface=unpaved,tracktype=grade3		track_unpaved\n"
    "w	highway=track,surface=ground,tracktype=grade4		track_ground\n"
    "w	highway=track,surface=grass,tracktype=grade5		track_grass\n"
    "w	highway=unsurfaced			track_gravelled\n"
    "w	highway=steps				steps\n"
    "w	historic=archaeological_site	poly_archaeological_site\n"
    "w	historic=battlefield	poly_battlefield\n"
    "w	historic=ruins		poly_ruins\n"
    "w	historic=town gate	poly_building\n"
    "w	landuse=allotments	poly_allotments\n"
    "w	landuse=basin		poly_basin\n"
    "w	landuse=brownfield	poly_brownfield\n"
    "w	landuse=cemetery	poly_cemetery\n"
    "w	landuse=commercial	poly_commercial\n"
    "w	landuse=construction	poly_construction\n"
    "w	landuse=farm		poly_farm\n"
    "w	landuse=farmland	poly_farm\n"
    "w	landuse=farmyard	poly_town\n"
    "w	landuse=forest		poly_wood\n"
    "w	landuse=greenfield	poly_greenfield\n"
    "w	landuse=industrial	poly_industry\n"
    "w	landuse=landfill	poly_landfill\n"
    "w	landuse=military	poly_military\n"
    "w	landuse=plaza		poly_plaza\n"
    "w	landuse=quarry		poly_quarry\n"
    "w	landuse=railway		poly_railway\n"
    "w	landuse=recreation_ground		poly_recreation_ground\n"
    "w	landuse=reservoir	poly_reservoir\n"
    "w	landuse=residential	poly_town\n"
    "w	landuse=residential,area=1	poly_town\n"
    "w	landuse=retail		poly_retail\n"
    "w	landuse=village_green	poly_village_green\n"
    "w	landuse=vineyard	poly_farm\n"
    "w	leisure=common		poly_common\n"
    "w	leisure=fishing		poly_fishing\n"
    "w	leisure=garden		poly_garden\n"
    "w	leisure=golf_course	poly_golf_course\n"
    "w	leisure=marina		poly_marina\n"
    "w	leisure=nature_reserve	poly_nature_reserve\n"
    "w	leisure=park		poly_park\n"
    "w	leisure=pitch		poly_sports_pitch\n"
    "w	leisure=playground	poly_playground\n"
    "w	leisure=sports_centre	poly_sport\n"
    "w	leisure=stadium		poly_sports_stadium\n"
    "w	leisure=track		poly_sports_track\n"
    "w	leisure=water_park	poly_water_park\n"
    "w	military=airfield	poly_airfield\n"
    "w	military=barracks	poly_barracks\n"
    "w	military=danger_area	poly_danger_area\n"
    "w	military=naval_base	poly_naval_base\n"
    "w	military=range		poly_range\n"
    "w	natural=beach		poly_beach\n"
    "w	natural=coastline	water_line\n"
    "w	natural=fell		poly_fell\n"
    "w	natural=glacier		poly_glacier\n"
    "w	natural=heath		poly_heath\n"
    "w	natural=land		poly_land\n"
    "w	natural=marsh		poly_marsh\n"
    "w	natural=mud		poly_mud\n"
    "w	natural=scree		poly_scree\n"
    "w	natural=scrub		poly_scrub\n"
    "w	natural=water		poly_water\n"
    "w	natural=wood		poly_wood\n"
    "w	piste:type=downhill,piste:difficulty=advanced		piste_downhill_advanced\n"
    "w	piste:type=downhill,piste:difficulty=easy		piste_downhill_easy\n"
    "w	piste:type=downhill,piste:difficulty=expert		piste_downhill_expert\n"
    "w	piste:type=downhill,piste:difficulty=freeride		piste_downhill_freeride\n"
    "w	piste:type=downhill,piste:difficulty=intermediate	piste_downhill_intermediate\n"
    "w	piste:type=downhill,piste:difficulty=novice		piste_downhill_novice\n"
    "w	piste:type=nordic	piste_nordic\n"
    "w	place=suburb		poly_place1\n"
    "w	place=hamlet		poly_place2\n"
    "w	place=village		poly_place3\n"
    "w	place=municipality	poly_place4\n"
    "w	place=town		poly_place5\n"
    "w	place=city		poly_place6\n"
    "w	power=line		powerline\n"
    "w	railway=abandoned	rail_abandoned\n"
    "w	railway=disused		rail_disused\n"
    "w	railway=light_rail	rail_light\n"
    "w	railway=monorail	rail_mono\n"
    "w	railway=narrow_gauge	rail_narrow_gauge\n"
    "w	railway=preserved	rail_preserved\n"
    "w	railway=rail		rail\n"
    "w	railway=subway		rail_subway\n"
    "w	railway=tram		rail_tram\n"
    "w	route=ferry		ferry\n"
    "w	route=ski		piste_nordic\n"
    "w	sport=*			poly_sport\n"
    "w	tourism=artwork		poly_artwork\n"
    "w	tourism=attraction	poly_attraction\n"
    "w	tourism=camp_site	poly_camp_site\n"
    "w	tourism=caravan_site	poly_caravan_site\n"
    "w	tourism=picnic_site	poly_picnic_site\n"
    "w	tourism=theme_park	poly_theme_park\n"
    "w	tourism=zoo		poly_zoo\n"
    "w	waterway=canal		water_canal\n"
    "w	waterway=drain		water_drain\n"
    "w	waterway=river		water_river\n"
    "w	waterway=riverbank	poly_water\n"
    "w	waterway=stream		water_stream\n"
    "w	barrier=ditch	ditch\n"
    "w	barrier=hedge	hedge\n"
    "w	barrier=fence	fence\n"
    "w	barrier=wall	wall\n"
    "w	barrier=retaining_wall	retaining_wall\n"
    "w	barrier=city_wall	city_wall\n"
);

struct item_name {
        enum item_type item;
        char *name;
};

struct item_name item_names[]={
#define ITEM2(x,y) ITEM(y)
#define ITEM(x) { type_##x, #x },
#include "item_def.h"
#undef ITEM2
#undef ITEM
};

enum item_type item_from_name(const char *name)
{
    int i;

    for (i=0 ; i < sizeof(item_names)/sizeof(struct item_name) ; i++) {
        if (! strcmp(item_names[i].name, name))
            return item_names[i].item;
    }
    return type_none;
}

/*****/

NavitAdapter::NavitAdapter()
{
    QAction* loadFile = new QAction(tr("Load Navit file..."), this);
    loadFile->setData(theUid.toString());
    connect(loadFile, SIGNAL(triggered()), SLOT(onLoadFile()));
    theMenu = new QMenu();
    theMenu->addAction(loadFile);

    loaded = false;

//    loaded = navit.setFilename("C:/home/cbro/Merkaartor/osm_bbox_11.3,47.9,11.7,48.2.bin");
//    loaded = navit.setFilename("C:/home/cbro/Merkaartor/osm_bbox_4.2,50.7,4.6,50.9.bin");
//    loaded = navit.setFilename("C:/home/cbro/Merkaartor/belgium.navit.bin");

    MasPaintStyle theStyle;
    theStyle.loadPainters(":/Styles/Mapnik.mas");
    for (int i=0; i<theStyle.painterSize(); ++i) {
        thePrimitivePainters.append(PrimitivePainter(*theStyle.getPainter(i)));
    }

    QStringList osmAttr = attrmap.split("\n", QString::SkipEmptyParts);
    foreach (QString l, osmAttr) {
        QStringList flds = l.split("\t", QString::SkipEmptyParts);
        item_type typ = item_from_name(flds[2].toLatin1().data());
        if (typ == type_none)
            continue;
        NavitFeature f;
        QStringList sl = flds[1].split(",");
        foreach(QString t, sl) {
            QStringList kv = t.split("=");
            f.Tags.append(qMakePair(kv[0], kv[1]));
        }
        for(int i=0; i< thePrimitivePainters.size(); ++i) {
            if (thePrimitivePainters[i].matchesTag(&f, 0)) {
                myStyles.insert((quint32)typ, &thePrimitivePainters[i]);
                break;
            }
        }
    }
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
    return QRectF(QPointF(-6371000.0/2, -6371000.0/2), QPointF(6371000.0/2, 6371000.0/2));
}

QString NavitAdapter::projection() const
{
    return "EPSG:3857";
}

QPixmap NavitAdapter::getPixmap(const QRectF& wgs84Bbox, const QRectF& /*projBbox*/, const QRect& src) const
{
    if (!loaded)
        return QPixmap();

    qDebug() << "wgs84: " << wgs84Bbox;
    qDebug() << "src: " << src;
    QRectF pBox(NavitProject(wgs84Bbox.topLeft()), NavitProject(wgs84Bbox.bottomRight()));

    QList<NavitFeature> theFeats;
    navit.getFeatures(pBox.toRect(), theFeats);

    qDebug () << "Feats: " << theFeats.size();
    if (!theFeats.size())
        return QPixmap();

    QTransform tfm;

    double ScaleLon = src.width() / pBox.width();
    double ScaleLat = src.height() / pBox.height();

    double PLon = pBox.center().x() * ScaleLon;
    double PLat = pBox.center().y() * ScaleLat;
    double DeltaLon = src.width() / 2 - PLon;
    double DeltaLat = src.height() - (src.height() / 2 - PLat);

    double LengthOfOneDegreeLat = 6378137.0 * M_PI / 180;
    double LengthOfOneDegreeLon = LengthOfOneDegreeLat * fabs(cos(angToRad(wgs84Bbox.center().y())));
    double lonAnglePerM =  1 / LengthOfOneDegreeLon;
    double PixelPerM = src.width() / (double)wgs84Bbox.width() * lonAnglePerM;

    tfm.setMatrix(ScaleLon, 0, 0, 0, -ScaleLat, 0, DeltaLon, DeltaLat, 1);

    QPixmap pix(src.size());
    pix.fill(Qt::transparent);
    QPainter P(&pix);
    P.setRenderHint(QPainter::Antialiasing);

    QPainterPath clipPath;
    clipPath.addRect(pBox.adjusted(-1000, -1000, 1000, 1000));
    foreach (NavitFeature f, theFeats) {
        if (f.coordinates.size() > 1) {
            QPolygonF d(f.coordinates);
            QPainterPath aPath;
            aPath.addPolygon(d);
//            QRect br = d.boundingRect();
//            qDebug() << "brect: " << br;
            aPath = aPath.intersected(clipPath);
//            if (!aPath.intersects(pBox))
//                continue;
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
            if (!aPath.isEmpty()) {
                if (myStyles.contains(f.type)) {
                    if (myStyles[f.type]->matchesZoom(PixelPerM)) {
                        QPainterPath pp = tfm.map(aPath);
                        myStyles[f.type]->drawBackground(&pp, &P, PixelPerM);
                        myStyles[f.type]->drawForeground(&pp, &P, PixelPerM);
                        myStyles[f.type]->drawTouchup(&pp, &P, PixelPerM);
                        //          f.painter()->drawLabel(&pp, &P, PixelPerM);
                    }
                }
            }
        } else {
            if (f.coordinates.size() == 1) {
                if (!pBox.contains(f.coordinates[0]))
                    continue;
                //                P.setPen(QPen(Qt::red, 5));
                //                P.drawPoint(f.coordinates[0]);
                if (myStyles.contains(f.type)) {
                    if (myStyles[f.type]->matchesZoom(PixelPerM)) {
                        QPointF pp = tfm.map(f.coordinates[0]);
                        myStyles[f.type]->drawTouchup(&pp, &P, PixelPerM);
                        //                myStyles[StyleNr(w)]->drawLabel(&pp, &P, PixelPerM, strL[0]);
                    }
                }
            }
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

bool NavitAdapter::toXML(QDomElement xParent)
{
    bool OK = true;

    QDomElement fs = xParent.ownerDocument().createElement("Images");
    xParent.appendChild(fs);
    if (loaded)
        fs.setAttribute("filename", navit.filename());

    return OK;
}

void NavitAdapter::fromXML(const QDomElement xParent)
{
    QDomElement fs = xParent.firstChildElement();
    while(!fs.isNull()) {
        if (fs.tagName() == "Images") {
            QString fn = fs.attribute("filename");
            if (!fn.isEmpty())
                loaded = navit.setFilename(fn);
        }

        fs = fs.nextSiblingElement();
    }
}

QString NavitAdapter::toPropertiesHtml()
{
    QString h;

    if (loaded)
        h += "<i>" + tr("Filename") + ": </i>" + navit.filename();

    return h;
}

Q_EXPORT_PLUGIN2(MWalkingPapersBackgroundPlugin, NavitAdapter)
