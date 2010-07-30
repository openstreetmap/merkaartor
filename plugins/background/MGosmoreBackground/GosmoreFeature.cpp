#include "GosmoreFeature.h"

GosmoreFeature::GosmoreFeature(int stylenr)
{
    switch (stylenr)
    {
    case gosmore_note_yes:
        Tags.append(qMakePair(QString("gosmore_note"),QString("yes")));
        break;
    case highway_residential:
        Tags.append(qMakePair(QString("highway"),QString("residential")));
        break;
    case highway_unclassified:
        Tags.append(qMakePair(QString("highway"),QString("unclassified")));
        break;
    case highway_tertiary:
        Tags.append(qMakePair(QString("highway"),QString("tertiary")));
        break;
    case highway_secondary:
        Tags.append(qMakePair(QString("highway"),QString("secondary")));
        break;
    case highway_primary:
        Tags.append(qMakePair(QString("highway"),QString("primary")));
        break;
    case highway_trunk:
        Tags.append(qMakePair(QString("highway"),QString("trunk")));
        break;
    case highway_footway:
        Tags.append(qMakePair(QString("highway"),QString("footway")));
        break;
    case highway_service:
        Tags.append(qMakePair(QString("highway"),QString("service")));
        break;
    case highway_track:
        Tags.append(qMakePair(QString("highway"),QString("track")));
        break;
    case highway_cycleway:
        Tags.append(qMakePair(QString("highway"),QString("cycleway")));
        break;
    case highway_pedestrian:
        Tags.append(qMakePair(QString("highway"),QString("pedestrian")));
        break;
    case highway_steps:
        Tags.append(qMakePair(QString("highway"),QString("steps")));
        break;
    case highway_bridleway:
        Tags.append(qMakePair(QString("highway"),QString("bridleway")));
        break;
    case railway_rail:
        Tags.append(qMakePair(QString("railway"),QString("rail")));
        break;
    case railway_station:
        Tags.append(qMakePair(QString("railway"),QString("station")));
        break;
    case highway_mini_roundabout:
        Tags.append(qMakePair(QString("highway"),QString("mini_roundabout")));
        break;
    case highway_traffic_signals:
        Tags.append(qMakePair(QString("highway"),QString("traffic_signals")));
        break;
    case highway_bus_stop:
        Tags.append(qMakePair(QString("highway"),QString("bus_stop")));
        break;
    case amenity_parking:
        Tags.append(qMakePair(QString("amenity"),QString("parking")));
        break;
    case amenity_fuel:
        Tags.append(qMakePair(QString("amenity"),QString("fuel")));
        break;
    case amenity_school:
        Tags.append(qMakePair(QString("amenity"),QString("school")));
        break;
    case place_village:
        Tags.append(qMakePair(QString("place"),QString("village")));
        break;
    case place_suburb:
        Tags.append(qMakePair(QString("place"),QString("suburb")));
        break;
    case shop_supermarket:
        Tags.append(qMakePair(QString("shop"),QString("supermarket")));
        break;
    case religion_christian:
        Tags.append(qMakePair(QString("religion"),QString("christian")));
        Tags.append(qMakePair(QString("amenity"),QString("place_of_worship")));
        break;
    case religion_jewish:
        Tags.append(qMakePair(QString("religion"),QString("jewish")));
        Tags.append(qMakePair(QString("amenity"),QString("place_of_worship")));
        break;
    case religion_muslim:
        Tags.append(qMakePair(QString("religion"),QString("muslim")));
        Tags.append(qMakePair(QString("amenity"),QString("place_of_worship")));
        break;
    case amenity_pub:
        Tags.append(qMakePair(QString("amenity"),QString("pub")));
        break;
    case amenity_restaurant:
        Tags.append(qMakePair(QString("amenity"),QString("restaurant")));
        break;
    case power_tower:
        Tags.append(qMakePair(QString("power"),QString("tower")));
        break;
    case waterway_stream:
        Tags.append(qMakePair(QString("waterway"),QString("stream")));
        break;
    case amenity_grave_yard:
        Tags.append(qMakePair(QString("amenity"),QString("grave_yard")));
        break;
    case amenity_crematorium:
        Tags.append(qMakePair(QString("amenity"),QString("crematorium")));
        break;
    case amenity_shelter:
        Tags.append(qMakePair(QString("amenity"),QString("shelter")));
        break;
    case tourism_picnic_site:
        Tags.append(qMakePair(QString("tourism"),QString("picnic_site")));
        break;
    case leisure_common:
        Tags.append(qMakePair(QString("leisure"),QString("common")));
        break;
    case amenity_park_bench:
        Tags.append(qMakePair(QString("amenity"),QString("park_bench")));
        break;
    case tourism_viewpoint:
        Tags.append(qMakePair(QString("tourism"),QString("viewpoint")));
        break;
    case tourism_artwork:
        Tags.append(qMakePair(QString("tourism"),QString("artwork")));
        break;
    case tourism_museum:
        Tags.append(qMakePair(QString("tourism"),QString("museum")));
        break;
    case tourism_theme_park:
        Tags.append(qMakePair(QString("tourism"),QString("theme_park")));
        break;
    case tourism_zoo:
        Tags.append(qMakePair(QString("tourism"),QString("zoo")));
        break;
    case leisure_playground:
        Tags.append(qMakePair(QString("leisure"),QString("playground")));
        break;
    case leisure_park:
        Tags.append(qMakePair(QString("leisure"),QString("park")));
        break;
    case leisure_nature_reserve:
        Tags.append(qMakePair(QString("leisure"),QString("nature_reserve")));
        break;
    case leisure_miniature_golf:
        Tags.append(qMakePair(QString("leisure"),QString("miniature_golf")));
        break;
    case leisure_golf_course:
        Tags.append(qMakePair(QString("leisure"),QString("golf_course")));
        break;
    case leisure_sports_centre:
        Tags.append(qMakePair(QString("leisure"),QString("sports_centre")));
        break;
    case leisure_stadium:
        Tags.append(qMakePair(QString("leisure"),QString("stadium")));
        break;
    case leisure_pitch:
        Tags.append(qMakePair(QString("leisure"),QString("pitch")));
        break;
    case leisure_track:
        Tags.append(qMakePair(QString("leisure"),QString("track")));
        break;
    case sport_athletics:
        Tags.append(qMakePair(QString("sport"),QString("athletics")));
        break;
    case sport_10pin:
        Tags.append(qMakePair(QString("sport"),QString("10pin")));
        break;
    case sport_boules:
        Tags.append(qMakePair(QString("sport"),QString("boules")));
        break;
    case sport_bowls:
        Tags.append(qMakePair(QString("sport"),QString("bowls")));
        break;
    case sport_baseball:
        Tags.append(qMakePair(QString("sport"),QString("baseball")));
        break;
    case sport_basketball:
        Tags.append(qMakePair(QString("sport"),QString("basketball")));
        break;
    case sport_cricket:
        Tags.append(qMakePair(QString("sport"),QString("cricket")));
        break;
    case sport_cricket_nets:
        Tags.append(qMakePair(QString("sport"),QString("cricket_nets")));
        break;
    case sport_croquet:
        Tags.append(qMakePair(QString("sport"),QString("croquet")));
        break;
    case sport_dog_racing:
        Tags.append(qMakePair(QString("sport"),QString("dog_racing")));
        break;
    case sport_equestrian:
        Tags.append(qMakePair(QString("sport"),QString("equestrian")));
        break;
    case sport_football:
        Tags.append(qMakePair(QString("sport"),QString("football")));
        break;
    case sport_soccer:
        Tags.append(qMakePair(QString("sport"),QString("soccer")));
        break;
    case sport_climbing:
        Tags.append(qMakePair(QString("sport"),QString("climbing")));
        break;
    case sport_gymnastics:
        Tags.append(qMakePair(QString("sport"),QString("gymnastics")));
        break;
    case sport_hockey:
        Tags.append(qMakePair(QString("sport"),QString("hockey")));
        break;
    case sport_horse_racing:
        Tags.append(qMakePair(QString("sport"),QString("horse_racing")));
        break;
    case sport_motor:
        Tags.append(qMakePair(QString("sport"),QString("motor")));
        break;
    case sport_pelota:
        Tags.append(qMakePair(QString("sport"),QString("pelota")));
        break;
    case sport_rugby:
        Tags.append(qMakePair(QString("sport"),QString("rugby")));
        break;
    case sport_australian_football:
        Tags.append(qMakePair(QString("sport"),QString("australian_football")));
        break;
    case sport_skating:
        Tags.append(qMakePair(QString("sport"),QString("skating")));
        break;
    case sport_skateboard:
        Tags.append(qMakePair(QString("sport"),QString("skateboard")));
        break;
    case sport_handball:
        Tags.append(qMakePair(QString("sport"),QString("handball")));
        break;
    case sport_table_tennis:
        Tags.append(qMakePair(QString("sport"),QString("table_tennis")));
        break;
    case sport_tennis:
        Tags.append(qMakePair(QString("sport"),QString("tennis")));
        break;
    case sport_racquet:
        Tags.append(qMakePair(QString("sport"),QString("racquet")));
        break;
    case sport_badminton:
        Tags.append(qMakePair(QString("sport"),QString("badminton")));
        break;
    case sport_paintball:
        Tags.append(qMakePair(QString("sport"),QString("paintball")));
        break;
    case sport_shooting:
        Tags.append(qMakePair(QString("sport"),QString("shooting")));
        break;
    case sport_volleyball:
        Tags.append(qMakePair(QString("sport"),QString("volleyball")));
        break;
    case sport_beachvolleyball:
        Tags.append(qMakePair(QString("sport"),QString("beachvolleyball")));
        break;
    case sport_archery:
        Tags.append(qMakePair(QString("sport"),QString("archery")));
        break;
    case sport_skiing:
        Tags.append(qMakePair(QString("sport"),QString("skiing")));
        break;
    case sport_rowing:
        Tags.append(qMakePair(QString("sport"),QString("rowing")));
        break;
    case sport_sailing:
        Tags.append(qMakePair(QString("sport"),QString("sailing")));
        break;
    case sport_diving:
        Tags.append(qMakePair(QString("sport"),QString("diving")));
        break;
    case sport_swimming:
        Tags.append(qMakePair(QString("sport"),QString("swimming")));
        break;
    case leisure_swimming_pool:
        Tags.append(qMakePair(QString("leisure"),QString("swimming_pool")));
        break;
    case leisure_water_park:
        Tags.append(qMakePair(QString("leisure"),QString("water_park")));
        break;
    case leisure_marina:
        Tags.append(qMakePair(QString("leisure"),QString("marina")));
        break;
    case leisure_slipway:
        Tags.append(qMakePair(QString("leisure"),QString("slipway")));
        break;
    case leisure_fishing:
        Tags.append(qMakePair(QString("leisure"),QString("fishing")));
        break;
    case shop_bakery:
        Tags.append(qMakePair(QString("shop"),QString("bakery")));
        break;
    case shop_butcher:
        Tags.append(qMakePair(QString("shop"),QString("butcher")));
        break;
    case shop_florist:
        Tags.append(qMakePair(QString("shop"),QString("florist")));
        break;
    case shop_groceries:
        Tags.append(qMakePair(QString("shop"),QString("groceries")));
        break;
    case shop_beverages:
        Tags.append(qMakePair(QString("shop"),QString("beverages")));
        break;
    case shop_clothes:
        Tags.append(qMakePair(QString("shop"),QString("clothes")));
        break;
    case shop_shoes:
        Tags.append(qMakePair(QString("shop"),QString("shoes")));
        break;
    case shop_jewelry:
        Tags.append(qMakePair(QString("shop"),QString("jewelry")));
        break;
    case shop_books:
        Tags.append(qMakePair(QString("shop"),QString("books")));
        break;
    case shop_newsagent:
        Tags.append(qMakePair(QString("shop"),QString("newsagent")));
        break;
    case shop_furniture:
        Tags.append(qMakePair(QString("shop"),QString("furniture")));
        break;
    case shop_hifi:
        Tags.append(qMakePair(QString("shop"),QString("hifi")));
        break;
    case shop_electronics:
        Tags.append(qMakePair(QString("shop"),QString("electronics")));
        break;
    case shop_computer:
        Tags.append(qMakePair(QString("shop"),QString("computer")));
        break;
    case shop_video:
        Tags.append(qMakePair(QString("shop"),QString("video")));
        break;
    case shop_toys:
        Tags.append(qMakePair(QString("shop"),QString("toys")));
        break;
    case shop_motorcycle:
        Tags.append(qMakePair(QString("shop"),QString("motorcycle")));
        break;
    case shop_car_repair:
        Tags.append(qMakePair(QString("shop"),QString("car_repair")));
        break;
    case shop_doityourself:
        Tags.append(qMakePair(QString("shop"),QString("doityourself")));
        break;
    case shop_garden_centre:
        Tags.append(qMakePair(QString("shop"),QString("garden_centre")));
        break;
    case shop_outdoor:
        Tags.append(qMakePair(QString("shop"),QString("outdoor")));
        break;
    case shop_bicycle:
        Tags.append(qMakePair(QString("shop"),QString("bicycle")));
        break;
    case shop_dry_cleaning:
        Tags.append(qMakePair(QString("shop"),QString("dry_cleaning")));
        break;
    case shop_laundry:
        Tags.append(qMakePair(QString("shop"),QString("laundry")));
        break;
    case shop_hairdresser:
        Tags.append(qMakePair(QString("shop"),QString("hairdresser")));
        break;
    case shop_travel_agency:
        Tags.append(qMakePair(QString("shop"),QString("travel_agency")));
        break;
    case shop_convenience:
        Tags.append(qMakePair(QString("shop"),QString("convenience")));
        break;
    case shop_mall:
        Tags.append(qMakePair(QString("shop"),QString("mall")));
        break;
    case shop_department_store:
        Tags.append(qMakePair(QString("shop"),QString("department_store")));
        break;
    case amenity_biergarten:
        Tags.append(qMakePair(QString("amenity"),QString("biergarten")));
        break;
    case amenity_nightclub:
        Tags.append(qMakePair(QString("amenity"),QString("nightclub")));
        break;
    case amenity_bar:
        Tags.append(qMakePair(QString("amenity"),QString("bar")));
        break;
    case amenity_cafe:
        Tags.append(qMakePair(QString("amenity"),QString("cafe")));
        break;
    case amenity_fast_food:
        Tags.append(qMakePair(QString("amenity"),QString("fast_food")));
        break;
    case amenity_ice_cream:
        Tags.append(qMakePair(QString("amenity"),QString("ice_cream")));
        break;
    case amenity_bicycle_rental:
        Tags.append(qMakePair(QString("amenity"),QString("bicycle_rental")));
        break;
    case amenity_car_rental:
        Tags.append(qMakePair(QString("amenity"),QString("car_rental")));
        break;
    case amenity_car_sharing:
        Tags.append(qMakePair(QString("amenity"),QString("car_sharing")));
        break;
    case amenity_car_wash:
        Tags.append(qMakePair(QString("amenity"),QString("car_wash")));
        break;
    case amenity_taxi:
        Tags.append(qMakePair(QString("amenity"),QString("taxi")));
        break;
    case amenity_telephone:
        Tags.append(qMakePair(QString("amenity"),QString("telephone")));
        break;
    case amenity_post_office:
        Tags.append(qMakePair(QString("amenity"),QString("post_office")));
        break;
    case amenity_post_box:
        Tags.append(qMakePair(QString("amenity"),QString("post_box")));
        break;
    case tourism_information:
        Tags.append(qMakePair(QString("tourism"),QString("information")));
        break;
    case amenity_toilets:
        Tags.append(qMakePair(QString("amenity"),QString("toilets")));
        break;
    case amenity_recycling:
        Tags.append(qMakePair(QString("amenity"),QString("recycling")));
        break;
    case amenity_fire_station:
        Tags.append(qMakePair(QString("amenity"),QString("fire_station")));
        break;
    case amenity_police:
        Tags.append(qMakePair(QString("amenity"),QString("police")));
        break;
    case amenity_courthouse:
        Tags.append(qMakePair(QString("amenity"),QString("courthouse")));
        break;
    case amenity_prison:
        Tags.append(qMakePair(QString("amenity"),QString("prison")));
        break;
    case amenity_public_building:
        Tags.append(qMakePair(QString("amenity"),QString("public_building")));
        break;
    case amenity_townhall:
        Tags.append(qMakePair(QString("amenity"),QString("townhall")));
        break;
    case amenity_cinema:
        Tags.append(qMakePair(QString("amenity"),QString("cinema")));
        break;
    case amenity_arts_centre:
        Tags.append(qMakePair(QString("amenity"),QString("arts_centre")));
        break;
    case amenity_theatre:
        Tags.append(qMakePair(QString("amenity"),QString("theatre")));
        break;
    case tourism_hotel:
        Tags.append(qMakePair(QString("tourism"),QString("hotel")));
        break;
    case tourism_motel:
        Tags.append(qMakePair(QString("tourism"),QString("motel")));
        break;
    case tourism_guest_house:
        Tags.append(qMakePair(QString("tourism"),QString("guest_house")));
        break;
    case tourism_hostel:
        Tags.append(qMakePair(QString("tourism"),QString("hostel")));
        break;
    case tourism_chalet:
        Tags.append(qMakePair(QString("tourism"),QString("chalet")));
        break;
    case tourism_camp_site:
        Tags.append(qMakePair(QString("tourism"),QString("camp_site")));
        break;
    case tourism_caravan_site:
        Tags.append(qMakePair(QString("tourism"),QString("caravan_site")));
        break;
    case amenity_pharmacy:
        Tags.append(qMakePair(QString("amenity"),QString("pharmacy")));
        break;
    case amenity_dentist:
        Tags.append(qMakePair(QString("amenity"),QString("dentist")));
        break;
    case amenity_doctor:
        Tags.append(qMakePair(QString("amenity"),QString("doctor")));
        break;
    case amenity_hospital:
        Tags.append(qMakePair(QString("amenity"),QString("hospital")));
        break;
    case amenity_bank:
        Tags.append(qMakePair(QString("amenity"),QString("bank")));
        break;
    case amenity_bureau_de_change:
        Tags.append(qMakePair(QString("amenity"),QString("bureau_de_change")));
        break;
    case amenity_atm:
        Tags.append(qMakePair(QString("amenity"),QString("atm")));
        break;
    case amenity_drinking_water:
        Tags.append(qMakePair(QString("amenity"),QString("drinking_water")));
        break;
    case amenity_fountain:
        Tags.append(qMakePair(QString("amenity"),QString("fountain")));
        break;
    case natural_spring:
        Tags.append(qMakePair(QString("natural"),QString("spring")));
        break;
    case amenity_university:
        Tags.append(qMakePair(QString("amenity"),QString("university")));
        break;
    case amenity_college:
        Tags.append(qMakePair(QString("amenity"),QString("college")));
        break;
    case amenity_kindergarten:
        Tags.append(qMakePair(QString("amenity"),QString("kindergarten")));
        break;
    case highway_living_street:
        Tags.append(qMakePair(QString("highway"),QString("living_street")));
        break;
    case highway_motorway:
        Tags.append(qMakePair(QString("highway"),QString("motorway")));
        break;
    case highway_motorway_link:
        Tags.append(qMakePair(QString("highway"),QString("motorway_link")));
        break;
    case highway_trunk_link:
        Tags.append(qMakePair(QString("highway"),QString("trunk_link")));
        break;
    case highway_primary_link:
        Tags.append(qMakePair(QString("highway"),QString("primary_link")));
        break;
    case barrier_bollard:
        Tags.append(qMakePair(QString("barrier"),QString("bollard")));
        break;
    case barrier_gate:
        Tags.append(qMakePair(QString("barrier"),QString("gate")));
        break;
    case barrier_stile:
        Tags.append(qMakePair(QString("barrier"),QString("stile")));
        break;
    case barrier_cattle_grid:
        Tags.append(qMakePair(QString("barrier"),QString("cattle_grid")));
        break;
    case barrier_toll_booth:
        Tags.append(qMakePair(QString("barrier"),QString("toll_booth")));
        break;
    case man_made_beacon:
        Tags.append(qMakePair(QString("man_made"),QString("beacon")));
        break;
    case man_made_survey_point:
        Tags.append(qMakePair(QString("man_made"),QString("survey_point")));
        break;
    case man_made_tower:
        Tags.append(qMakePair(QString("man_made"),QString("tower")));
        break;
    case man_made_water_tower:
        Tags.append(qMakePair(QString("man_made"),QString("water_tower")));
        break;
    case man_made_gasometer:
        Tags.append(qMakePair(QString("man_made"),QString("gasometer")));
        break;
    case man_made_reservoir_covered:
        Tags.append(qMakePair(QString("man_made"),QString("reservoir_covered")));
        break;
    case man_made_lighthouse:
        Tags.append(qMakePair(QString("man_made"),QString("lighthouse")));
        break;
    case man_made_windmill:
        Tags.append(qMakePair(QString("man_made"),QString("windmill")));
        break;
    case man_made_pier:
        Tags.append(qMakePair(QString("man_made"),QString("pier")));
        break;
    case man_made_pipeline:
        Tags.append(qMakePair(QString("man_made"),QString("pipeline")));
        break;
    case man_made_wastewater_plant:
        Tags.append(qMakePair(QString("man_made"),QString("wastewater_plant")));
        break;
    case man_made_crane:
        Tags.append(qMakePair(QString("man_made"),QString("crane")));
        break;
    case building_yes:
        Tags.append(qMakePair(QString("building"),QString("yes")));
        break;
    case landuse_forest:
        Tags.append(qMakePair(QString("landuse"),QString("forest")));
        break;
    case landuse_residential:
        Tags.append(qMakePair(QString("landuse"),QString("residential")));
        break;
    case landuse_industrial:
        Tags.append(qMakePair(QString("landuse"),QString("industrial")));
        break;
    case landuse_retail:
        Tags.append(qMakePair(QString("landuse"),QString("retail")));
        break;
    case landuse_commercial:
        Tags.append(qMakePair(QString("landuse"),QString("commercial")));
        break;
    case landuse_construction:
        Tags.append(qMakePair(QString("landuse"),QString("construction")));
        break;
    case landuse_reservoir:
        Tags.append(qMakePair(QString("landuse"),QString("reservoir")));
        break;
    case natural_water:
        Tags.append(qMakePair(QString("natural"),QString("water")));
        break;
    case landuse_basin:
        Tags.append(qMakePair(QString("landuse"),QString("basin")));
        break;
    case landuse_landfill:
        Tags.append(qMakePair(QString("landuse"),QString("landfill")));
        break;
    case landuse_quarry:
        Tags.append(qMakePair(QString("landuse"),QString("quarry")));
        break;
    case landuse_cemetery:
        Tags.append(qMakePair(QString("landuse"),QString("cemetery")));
        break;
    case landuse_allotments:
        Tags.append(qMakePair(QString("landuse"),QString("allotments")));
        break;
    case landuse_farm:
        Tags.append(qMakePair(QString("landuse"),QString("farm")));
        break;
    case landuse_farmyard:
        Tags.append(qMakePair(QString("landuse"),QString("farmyard")));
        break;
    case landuse_military:
        Tags.append(qMakePair(QString("landuse"),QString("military")));
        break;
    case religion_bahai:
        Tags.append(qMakePair(QString("religion"),QString("bahai")));
        Tags.append(qMakePair(QString("amenity"),QString("place_of_worship")));
        break;
    case religion_buddhist:
        Tags.append(qMakePair(QString("religion"),QString("buddhist")));
        Tags.append(qMakePair(QString("amenity"),QString("place_of_worship")));
        break;
    case religion_hindu:
        Tags.append(qMakePair(QString("religion"),QString("hindu")));
        Tags.append(qMakePair(QString("amenity"),QString("place_of_worship")));
        break;
    case religion_jain:
        Tags.append(qMakePair(QString("religion"),QString("jain")));
        Tags.append(qMakePair(QString("amenity"),QString("place_of_worship")));
        break;
    case religion_sikh:
        Tags.append(qMakePair(QString("religion"),QString("sikh")));
        Tags.append(qMakePair(QString("amenity"),QString("place_of_worship")));
        break;
    case religion_shinto:
        Tags.append(qMakePair(QString("religion"),QString("shinto")));
        Tags.append(qMakePair(QString("amenity"),QString("place_of_worship")));
        break;
    case religion_taoist:
        Tags.append(qMakePair(QString("religion"),QString("taoist")));
        Tags.append(qMakePair(QString("amenity"),QString("place_of_worship")));
        break;
    case highway_road:
        Tags.append(qMakePair(QString("highway"),QString("road")));
        break;
    case restriction_no_right_turn:
        Tags.append(qMakePair(QString("restriction"),QString("no_right_turn")));
        break;
    case restriction_no_left_turn:
        Tags.append(qMakePair(QString("restriction"),QString("no_left_turn")));
        break;
    case restriction_no_u_turn:
        Tags.append(qMakePair(QString("restriction"),QString("no_u_turn")));
        break;
    case restriction_no_straight_on:
        Tags.append(qMakePair(QString("restriction"),QString("no_straight_on")));
        break;
    case restriction_only_right_turn:
        Tags.append(qMakePair(QString("restriction"),QString("only_right_turn")));
        break;
    case restriction_only_left_turn:
        Tags.append(qMakePair(QString("restriction"),QString("only_left_turn")));
        break;
    case restriction_only_straight_on:
        Tags.append(qMakePair(QString("restriction"),QString("only_straight_on")));
        break;
    }
}

int GosmoreFeature::tagSize() const
{
    return Tags.size();
}

QString GosmoreFeature::tagValue(int i) const
{
    return Tags[i].second;
}

QString GosmoreFeature::tagKey(int i) const
{
    return Tags[i].first;
}

int GosmoreFeature::findKey(const QString &k) const
{
    for (int i=0; i<Tags.size(); ++i)
        if (Tags[i].first == k)
            return i;
    return Tags.size();
}

QString GosmoreFeature::tagValue(const QString& k, const QString& Default) const
{
    for (int i=0; i<Tags.size(); ++i)
        if (Tags[i].first == k)
            return Tags[i].second;
    return Default;
}

