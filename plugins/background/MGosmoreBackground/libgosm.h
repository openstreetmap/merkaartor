#ifndef LIBGOSM_H
#define LIBGOSM_H

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <limits.h>

#ifdef _WIN32_WCE
#include <windows.h>
#define lrint(x) int ((x) < 0 ? (x) - 0.5 : (x) + 0.5) 
typedef int intptr_t;
#define strncasecmp _strnicmp
#define stricmp _stricmp
#endif

#ifndef _WIN32
#include <libxml/xmlreader.h>
#include <inttypes.h>
#define stricmp strcasecmp
typedef long long __int64;
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846 // Not in math ??
#endif

#if __FreeBSD__ || __APPLE__  // Thanks to Ted Mielczarek & Dmitry
#define fopen64(x,y) fopen(x,y)
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// this definitions have been changes to prevent compiler warnings
// copied verbatim for /usr/include/limits.h
#ifndef INT_MAX
#define INT_MAX       2147483647
#endif

#ifndef INT_MIN
#define INT_MIN       (-INT_MAX - 1)
#endif

#define Sqr(x) ((x)*(x))
inline int isqrt (int x) { return lrint (sqrt (x)); } // Optimize this ?

#define TILEBITS (18)
#define TILESIZE (1<<TILEBITS)

#define RESTRICTIONS M (access) M (motorcar) M (bicycle) M (foot) M (goods) \
  M (hgv) M (horse) M (motorcycle) M (psv) M (moped) M (mofa) \
  M (motorboat) M (boat) \
  M (oneway) M (roundabout)

#define M(field) field ## R,
enum {
  STYLE_BITS = 10, RESTRICTIONS bicycleOneway, layerBit1, layerBit2, layerBit3
};
#undef M

// Below is a list of the tags that the user may add. It should also include
// any tags that gosmore.cpp may test for directly, e.g.
// StyleNr(w) == highway_traffic_signals .
// See http://etricceline.de/osm/Europe/En/tags.htm for the most popular tags
// The fields are k, v, short name and the additional tags.
#define STYLES \
 s (gosmore_note, yes,        "note"            , "") \
 s (highway, residential,     "residential"     , "") \
 s (highway, unclassified,    "unclassified"    , "") \
 s (highway, tertiary,        "tertiary"        , "") \
 s (highway, secondary,       "secondary"       , "") \
 s (highway, primary,         "primary"         , "") \
 s (highway, trunk,           "trunk"           , "") \
 s (highway, footway,         "footway"         , "") \
 s (highway, service,         "service"         , "") \
 s (highway, track,           "track"           , "") \
 s (highway, cycleway,        "cycleway"        , "") \
 s (highway, pedestrian,      "pedestrian"      , "") \
 s (highway, steps,           "steps"           , "") \
 s (highway, bridleway,       "bridleway"       , "") \
 s (railway, rail,            "railway"         , "") \
 s (railway, station,         "railway station" , "") \
 s (highway, mini_roundabout, "mini roundabout" , "") \
 s (highway, traffic_signals, "traffic signals" , "") \
 s (highway, bus_stop,        "bus stop"        , "") \
 s (amenity, parking,         "parking"         , "") \
 s (amenity, fuel,            "fuel"            , "") \
 s (amenity, school,          "school"          , "") \
 s (place,   village,         "village"         , "") \
 s (place,   suburb,          "suburb"          , "") \
 s (shop,    supermarket,     "supermarket"     , "") \
 s (religion, christian,      "church"          , \
               "  <tag k='amenity' v='place_of_worship' />\n") \
 s (religion, jewish,         "synagogue"       , \
               "  <tag k='amenity' v='place_of_worship' />\n") \
 s (religion, muslim,         "mosque"          , \
               "  <tag k='amenity' v='place_of_worship' />\n") \
 s (amenity, pub,             "pub"             , "") \
 s (amenity, restaurant,      "restaurant"      , "") \
 s (power,   tower,           "power tower"     , "") \
 s (waterway, stream,         "stream"          , "") \
 s (amenity, grave_yard,      "grave yard"      , "") \
 s (amenity, crematorium,     "crematorium"     , "") \
 s (amenity, shelter,         "shelter"         , "") \
 s (tourism, picnic_site,     "picnic site"     , "") \
 s (leisure, common,          "common area"     , "") \
 s (amenity, park_bench,      "park bench"      , "") \
 s (tourism, viewpoint,       "viewpoint"       , "") \
 s (tourism, artwork,         "artwork"         , "") \
 s (tourism, museum,          "museum"          , "") \
 s (tourism, theme_park,      "theme park"      , "") \
 s (tourism, zoo,             "zoo"             , "") \
 s (leisure, playground,      "playground"      , "") \
 s (leisure, park,            "park"            , "") \
 s (leisure, nature_reserve,  "nature reserve"  , "") \
 s (leisure, miniature_golf,  "miniature golf"  , "") \
 s (leisure, golf_course,     "golf course"     , "") \
 s (leisure, sports_centre,   "sports centre"   , "") \
 s (leisure, stadium,         "stadium"         , "") \
 s (leisure, pitch,           "pitch"           , "") \
 s (leisure, track,           "track"           , "") \
 s (sport,   athletics,       "athletics"       , "") \
 s (sport,   10pin,           "10 pin"          , "") \
 s (sport,   boules,          "boules"          , "") \
 s (sport,   bowls,           "bowls"           , "") \
 s (sport,   baseball,        "baseball"        , "") \
 s (sport,   basketball,      "basketball"      , "") \
 s (sport,   cricket,         "cricket"         , "") \
 s (sport,   cricket_nets,    "cricket nets"    , "") \
 s (sport,   croquet,         "croquet"         , "") \
 s (sport,   dog_racing,      "dog racing"      , "") \
 s (sport,   equestrian,      "equestrian"      , "") \
 s (sport,   football,        "football"        , "") \
 s (sport,   soccer,          "soccer"          , "") \
 s (sport,   climbing,        "climbing"        , "") \
 s (sport,   gymnastics,      "gymnastics"      , "") \
 s (sport,   hockey,          "hockey"          , "") \
 s (sport,   horse_racing,    "horse racing"    , "") \
 s (sport,   motor,           "motor sport"     , "") \
 s (sport,   pelota,          "pelota"          , "") \
 s (sport,   rugby,           "rugby"           , "") \
 s (sport,   australian_football, "australian football" , "") \
 s (sport,   skating,         "skating"         , "") \
 s (sport,   skateboard,      "skateboard"      , "") \
 s (sport,   handball,        "handball"        , "") \
 s (sport,   table_tennis,    "table tennis"    , "") \
 s (sport,   tennis,          "tennis"          , "") \
 s (sport,   racquet,         "racquet"         , "") \
 s (sport,   badminton,       "badminton"       , "") \
 s (sport,   paintball,       "paintball"       , "") \
 s (sport,   shooting,        "shooting"        , "") \
 s (sport,   volleyball,      "volleyball"      , "") \
 s (sport,   beachvolleyball, "beach volleyball" , "") \
 s (sport,   archery,         "archery"         , "") \
 s (sport,   skiing,          "skiing"          , "") \
 s (sport,   rowing,          "rowing"          , "") \
 s (sport,   sailing,         "sailing"         , "") \
 s (sport,   diving,          "diving"          , "") \
 s (sport,   swimming,        "swimming"        , "") \
 s (leisure, swimming_pool,   "swimming pool"   , "") \
 s (leisure, water_park,      "water park"      , "") \
 s (leisure, marina,          "marina"          , "") \
 s (leisure, slipway,         "slipway"         , "") \
 s (leisure, fishing,         "fishing"         , "") \
 s (shop,    bakery,          "bakery"          , "") \
 s (shop,    butcher,         "butcher"         , "") \
 s (shop,    florist,         "florist"         , "") \
 s (shop,    groceries,       "groceries"       , "") \
 s (shop,    beverages,       "liquor / wine"   , "") \
 s (shop,    clothes,         "clothing shop"   , "") \
 s (shop,    shoes,           "shoe shop"       , "") \
 s (shop,    jewelry,         "jewelry store"   , "") \
 s (shop,    books,           "bookshop"        , "") \
 s (shop,    newsagent,       "newsagent"       , "") \
 s (shop,    furniture,       "furniture store" , "") \
 s (shop,    hifi,            "Hi-Fi store"     , "") \
 s (shop,    electronics,     "electronics store" , "") \
 s (shop,    computer,        "computer shop"   , "") \
 s (shop,    video,           "video rental"    , "") \
 s (shop,    toys,            "toy shop"        , "") \
 s (shop,    motorcycle,      "motorcycle"      , "") \
 s (shop,    car_repair,      "car repair"      , "") \
 s (shop,    doityourself,    "doityourself"    , "") \
 s (shop,    garden_centre,   "garden centre"   , "") \
 s (shop,    outdoor,         "outdoor"         , "") \
 s (shop,    bicycle,         "bicycle shop"    , "") \
 s (shop,    dry_cleaning,    "dry cleaning"    , "") \
 s (shop,    laundry,         "laundry"         , "") \
 s (shop,    hairdresser,     "hairdresser"     , "") \
 s (shop,    travel_agency,   "travel_agency"   , "") \
 s (shop,    convenience,     "convenience"     , "") \
 s (shop,    mall,            "mall"            , "") \
 s (shop,    department_store, "department store" , "") \
 s (amenity, biergarten,      "biergarten"      , "") \
 s (amenity, nightclub,       "nightclub"       , "") \
 s (amenity, bar,             "bar"             , "") \
 s (amenity, cafe,            "cafe"            , "") \
 s (amenity, fast_food,       "fast food"       , "") \
 s (amenity, ice_cream,       "icecream"        , "") \
 s (amenity, bicycle_rental,  "bicycle rental"  , "") \
 s (amenity, car_rental,      "car rental"      , "") \
 s (amenity, car_sharing,     "car sharing"     , "") \
 s (amenity, car_wash,        "car wash"        , "") \
 s (amenity, taxi,            "taxi"            , "") \
 s (amenity, telephone,       "telephone"       , "") \
 s (amenity, post_office,     "post office"     , "") \
 s (amenity, post_box,        "post box"        , "") \
 s (tourism, information,     "tourist info"    , "") \
 s (amenity, toilets,         "toilets"         , "") \
 s (amenity, recycling,       "recycling"       , "") \
 s (amenity, fire_station,    "fire station"    , "") \
 s (amenity, police,          "police"          , "") \
 s (amenity, courthouse,      "courthouse"      , "") \
 s (amenity, prison,          "prison"          , "") \
 s (amenity, public_building, "public building" , "") \
 s (amenity, townhall,        "townhall"        , "") \
 s (amenity, cinema,          "cinema"          , "") \
 s (amenity, arts_centre,     "arts centre"     , "") \
 s (amenity, theatre,         "theatre"         , "") \
 s (tourism, hotel,           "hotel"           , "") \
 s (tourism, motel,           "motel"           , "") \
 s (tourism, guest_house,     "guest house"     , "") \
 s (tourism, hostel,          "hostel"          , "") \
 s (tourism, chalet,          "chalet"          , "") \
 s (tourism, camp_site,       "camp site"       , "") \
 s (tourism, caravan_site,    "caravan site"    , "") \
 s (amenity, pharmacy,        "pharmacy"        , "") \
 s (amenity, dentist,         "dentist"         , "") \
 s (amenity, doctor,          "doctor"          , "") \
 s (amenity, hospital,        "hospital"        , "") \
 s (amenity, bank,            "bank"            , "") \
 s (amenity, bureau_de_change, "bureau de change" , "") \
 s (amenity, atm,             "atm"            , "") \
 s (amenity, drinking_water,  "drinking water"  , "") \
 s (amenity, fountain,        "fountain"        , "") \
 s (natural, spring,          "spring"          , "") \
 s (amenity, university,      "university"      , "") \
 s (amenity, college,         "college"         , "") \
 s (amenity, kindergarten,    "kindergarten"    , "") \
 s (highway, living_street,   "living street"   , "") \
 s (highway, motorway,        "motorway"        , "") \
 s (highway, motorway_link,   "mway link"       , "") \
 s (highway, trunk_link,      "trunk link"      , "") \
 s (highway, primary_link,    "primary_link"    , "") \
 s (barrier, bollard,         "bollard"         , "") /* First barrier ! */ \
 s (barrier, gate,            "gate"            , "") \
 s (barrier, stile,           "stile"           , "") \
 s (barrier, cattle_grid,     "cattle_grid"     , "") \
 s (barrier, toll_booth,      "toll_booth"      , "") /* Last barrier ! */ \
 s (man_made, beacon,         "beacon"          , "" ) \
 s (man_made, survey_point,   "survey point"    , "" ) \
 s (man_made, tower,          "tower"           , "" ) \
 s (man_made, water_tower,    "water tower"     , "" ) \
 s (man_made, gasometer,      "gasometer"       , "" ) \
 s (man_made, reservoir_covered, "covered reservoir", "" ) \
 s (man_made, lighthouse,     "lighthouse"      , "" ) \
 s (man_made, windmill,       "windmill"        , "" ) \
 s (man_made, pier,           "pier"            , "" ) \
 s (man_made, pipeline,       "pipeline"        , "" ) \
 s (man_made, wastewater_plant, "wastewater plant" , "" ) \
 s (man_made, crane,          "crane"           , "" ) \
 s (building, yes,            "building"        , "") \
 s (landuse, forest,          "forest"          , "") \
 s (landuse, residential,     "residential area", "") \
 s (landuse, industrial,      "industrial area" , "") \
 s (landuse, retail,          "retail area"     , "") \
 s (landuse, commercial,      "commercial area" , "") \
 s (landuse, construction,    "construction area" , "") \
 s (landuse, reservoir,       "reservoir"       , "") \
 s (natural, water,           "lake / dam"      , "") \
 s (landuse, basin,           "basin"           , "") \
 s (landuse, landfill,        "landfill"        , "") \
 s (landuse, quarry,          "quarry"          , "") \
 s (landuse, cemetery,        "cemetery"        , "") \
 s (landuse, allotments,      "allotments"      , "") \
 s (landuse, farm,            "farmland"        , "") \
 s (landuse, farmyard,        "farmyard"        , "") \
 s (landuse, military,        "military area"   , "") \
 s (religion, bahai,          "bahai"           , \
               "  <tag k='amenity' v='place_of_worship' />\n") \
 s (religion, buddhist,       "buddhist"        , \
               "  <tag k='amenity' v='place_of_worship' />\n") \
 s (religion, hindu,          "hindu"           , \
               "  <tag k='amenity' v='place_of_worship' />\n") \
 s (religion, jain,           "jainism"         , \
               "  <tag k='amenity' v='place_of_worship' />\n") \
 s (religion, sikh,           "sikhism"         , \
               "  <tag k='amenity' v='place_of_worship' />\n") \
 s (religion, shinto,         "shinto"          , \
               "  <tag k='amenity' v='place_of_worship' />\n") \
 s (religion, taoist,         "taoism"          , \
               "  <tag k='amenity' v='place_of_worship' />\n") \
 s (highway,         road,     "hway=road"      , "") \
 /* relations must be last and restriction_no_right_turn must be first */ \
 s (restriction, no_right_turn, ""              , "") \
 s (restriction, no_left_turn, ""               , "") \
 s (restriction, no_u_turn, ""                  , "") \
 s (restriction, no_straight_on, ""             , "") \
 /* restriction_no_straight_on must be the last "no_" restriction */ \
 s (restriction, only_right_turn, ""            , "") \
 s (restriction, only_left_turn, ""             , "") \
 s (restriction, only_straight_on, ""           , "") \
 /* restriction_only_straight_on must be the last restriction */

#define XXXFSDFSF \
 s (sport,   orienteering,    "orienteering"    , "") \
 s (sport,   gym,             "gym"             , "") \
 /* sport=golf isn't a golf course, so what is it ? */ \

#define s(k,v,shortname,extraTags) k ## _ ## v,
enum { STYLES firstElemStyle }; // highway_residential, ...
#undef s

struct styleStruct {
  int  x[16], lineWidth, lineRWidth, lineColour, lineColourBg, dashed;
  int  scaleMax, areaColour, dummy /* pad to 8 for 64 bit compatibility */;
  double aveSpeed[layerBit1], invSpeed[layerBit1];
};

struct ndType {
  int wayPtr, lat, lon, other[2];
};
/* This struct takes up a lot of space, but compressing is possible: If
other is encoded as byte offset from the current position, it should typically
be close to 0. So it can be Huffman encoded. Then the struct will no longer
have a fixed size, and the final position of an nd will not be known during
the first pass. So we over estimate the space needed between 2 nds and we can
do multiple passes until all the excess has been removed.

The difference between lat and wayPtr->clat will normally also be quite
small and can also be Huffman coded. The same applies to lon.

C code can be generated for these non dynamic Huffman decoders and the C
compiler can then optimize it for efficient decompression. Not only will this
save (flash) storage, but it will also increase the size of maps that can be
loaded and reduce the paging that is so slow on mobile devices.
*/

struct wayType {
  int bits, destination;
  int clat, clon, dlat, dlon; /* Centre coordinates and (half)diameter */
};

inline int Layer (wayType *w) { return w->bits >> 29; }

inline int Latitude (double lat)
{ /* Mercator projection onto a square means we have to clip
     everything beyond N85.05 and S85.05 */
  return lat > 85.051128779 ? 2147483647 : lat < -85.051128779 ? -2147483647 :
    lrint (log (tan (M_PI / 4 + lat * M_PI / 360)) / M_PI * 2147483648.0);
}

inline int Longitude (double lon)
{
  return lrint (lon / 180 * 2147483648.0);
}

inline double LatInverse (int lat)
{
  return (atan (exp (lat / 2147483648.0 * M_PI)) - M_PI / 4) / M_PI * 360;
}

inline double LonInverse (int lon)
{
  return lon / 2147483648.0 * 180;
}

/*---------- Global variables -----------*/
#define searchCnt 40
extern wayType *gosmSway[searchCnt];
extern int *hashTable, bucketsMin1, pakHead;
extern char *gosmData, *gosmSstr[searchCnt];
/* gosmSstr is no longer nul terminated. If it's a problem, use
string s (gosmSstr[i], strcspn (gosmSstr[i], "\n")); ... s.c_str() ... */

extern ndType *ndBase;
extern styleStruct *style;
extern int stylecount;

inline wayType *Way (ndType *nd) { return (wayType *)(nd->wayPtr+gosmData); }

inline int StyleNr (wayType *w) { return w->bits & ((2 << STYLE_BITS) - 1); }

inline styleStruct *Style (wayType *w) { return &style[StyleNr (w)]; }

unsigned inline ZEnc (int lon, int lat)
{ // Input as bits : lon15,lon14,...lon0 and lat15,lat14,...,lat0
  int t = (lon << 16) | lat;
  t = (t & 0xff0000ff) | ((t & 0x00ff0000) >> 8) | ((t & 0x0000ff00) << 8);
  t = (t & 0xf00ff00f) | ((t & 0x0f000f00) >> 4) | ((t & 0x00f000f0) << 4);
  t = (t & 0xc3c3c3c3) | ((t & 0x30303030) >> 2) | ((t & 0x0c0c0c0c) << 2);
  return (t & 0x99999999) | ((t & 0x44444444) >> 1) | ((t & 0x22222222) << 1);
} // Output as bits : lon15,lat15,lon14,lat14,...,lon0,lat0

inline int Hash (int lon, int lat, int lowz = 0)
{ /* All the normal tiles (that make up a super tile) are mapped to sequential
     buckets thereby improving caching and reducing the number of disk tracks
     required to render / route through a super tile sized area. 
     The map to sequential buckets is a 2-D Hilbert curve. */
  if (lowz) {
    lon >>= 7;
    lat >>= 7;
  }
  
  int t = ZEnc (lon >> TILEBITS, ((unsigned) lat) >> TILEBITS);
  int s = ((((unsigned)t & 0xaaaaaaaa) >> 1) | ((t & 0x55555555) << 1)) ^ ~t;
  // s=ZEnc(lon,lat)^ZEnc(lat,lon), so it can be used to swap lat and lon.
  #define SUPERTILEBITS (TILEBITS + 8)
  for (int lead = 1 << (SUPERTILEBITS * 2 - TILEBITS * 2); lead; lead >>= 2) {
    if (!(t & lead)) t ^= ((t & (lead << 1)) ? s : ~s) & (lead - 1);
  }

  return (((((t & 0xaaaaaaaa) >> 1) ^ t) + (lon >> SUPERTILEBITS) * 0x00d20381
    + (lat >> SUPERTILEBITS) * 0x75d087d9) &
    (lowz ? bucketsMin1 >> 7 : bucketsMin1)) + (lowz ? bucketsMin1 + 1 : 0);
}

// int TagCmp (const char *a, const char *b); // Only used in libgosm

struct OsmItr { // Iterate over all the objects in a square
  ndType *nd[1]; /* Readonly. Either can be 'from' or 'to', but you */
  /* can be guaranteed that nodes will be in hs[0] */
  
  int slat, slon, left, right, top, bottom, tsize; /* Private */
  ndType *end;
  
  OsmItr (int l, int t, int r, int b)
  {
    tsize = r - l > 1500000 ? TILESIZE << 7 : TILESIZE;
    left = l & (~(tsize - 1));
    right = (r + tsize - 1) & (~(tsize-1));
    top = t & (~(tsize - 1));
    bottom = (b + tsize - 1) & (~(tsize-1));
    
    slat = top;
    slon = left - tsize;
    nd[0] = end = NULL;
  }
};

int Next (OsmItr &itr); /* Friend of osmItr */

struct routeNodeType {
  ndType *nd;
  routeNodeType *shortest;
  int heapIdx, dir, remain; // Dir is 0 or 1
  // if heapIdx is negative, the node is not in the heap and best = -heapIdx.
};
/* The data is split over two structures (routeNodeType and routeHeapType).
  Some of these fields may cause fewer cache misses if their are in
  routeHeapType */
struct routeHeapType {
  routeNodeType *r;
  int best;
};

extern routeNodeType *route, *shortest;
extern routeHeapType *routeHeap;
extern int routeHeapSize, tlat, tlon, flat, flon, rlat, rlon, routeSuccess;

void Route (int recalculate, int plon, int plat, int Vehicle, int fast);
int RouteLoop (void);
void GosmFreeRoute (void);

int JunctionType (ndType *nd);

int *GosmIdxSearch (const char *key, unsigned z);
// GosmIdxSearch is only exported in order to find unique object, like cities.
void GosmSearch (int clon, int clat, const char *key);

int GosmInit (void *d, long size);

// *** EVERYTHING AFTER THIS POINT IS NOT IN THE WINDOWS BUILDS ***

#ifndef _WIN32

void GosmLoadAltStyle(const char* elemstylefile, const char* iconscsvfile);

// struct to hold mappings between elemstyles.xml and stylesrec
// these are needed when the osm file is converted to a pak file
typedef struct {
  char style_k[80];
  char style_v[80];
  int ruleNr;
  int defaultRestrict;
} elemstyleMapping;

// reads the elemstyles.xml file into srec, with the mapping between
// srec and elemstyles.xml stored in map, and the list of maximum
// speeds for each vehicle type in maxspeeds. styleCnt representing
// the location of the first elemstyle. Returns the final styleCnt.
int LoadElemstyles(/* in */ const char *elemstylesfname, 
		   const char *iconsfname,
		   /* out */ styleStruct *srec, elemstyleMapping *map);

// creates a new pakfile from an osmdata file read from standard in
int RebuildPak(const char* pakfile, const char* elemstylefile, 
	       const char* iconscsvfile, const char* masterpakfile, 
	       const int bbox[4]);
int SortRelations (void);

#endif // #ifndef _WIN32

#endif
