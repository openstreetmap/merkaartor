#include "Map/PreDefinedTags.h"
#include "Command/FeatureCommands.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/MapLayer.h"

#include <QtCore/QCoreApplication>
#include <QtGui/QComboBox>

// HACKING HINT
// The first parameter of addItem is the string as displayed to the user
// Surround it with a call to QT_TRANSLATE_NOOP(Context,UserVisibleString)
// to make sure it gets translated, Context is a disambiguating help for translators
// However we need the disambiguating context too so we redefine QT_TRANSLATE_NOOP
// The second parameter of addItem is the value of the tag

#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(A,B) A,B

static QString translateTag(const char* Context, const char* src, bool DoIt)
{
	if (DoIt)
		return QCoreApplication::translate(Context,src);
	else
		return QString::fromLatin1(src);
}

void addDefaults(QComboBox* Box, bool DoIt)
{
	Box->addItem(
		translateTag(QT_TRANSLATE_NOOP("DefaultTags","Not specified"), DoIt),
		"__unspecified");
	Box->addItem(
		translateTag(QT_TRANSLATE_NOOP("DefaultTags","Unknown"), DoIt),
		"__unknown");
}

void fillAmenities(QComboBox* Box)
{
	bool DoIt = M_PREFS->getTranslateTags();
	addDefaults(Box, DoIt);
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Arts centre"),DoIt), "arts_centre");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","ATM or cash point"),DoIt), "atm");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Bank"),DoIt), "bank");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Bank with atm"),DoIt), "bank;atm");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Beer garden"),DoIt), "biergarten");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Parking for bicycles"), DoIt), "bicycle_parking");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Bicycle Rental"), DoIt), "bicycle_rental");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Bureau de change"), DoIt), "bureau_de_change");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Bus station"), DoIt), "bus_station");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Cafe"), DoIt), "cafe");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Car Rental"), DoIt), "car_rental");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Car Sharing"), DoIt), "car_sharing");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Cinema"), DoIt), "cinema");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","College"), DoIt), "college");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Court house"), DoIt), "courthouse");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Crematorium"), DoIt), "crematorium");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Source of drinking water"), DoIt), "drinking_water");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Fast food"), DoIt), "Fast food");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Fire Station"), DoIt), "fire_station");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Fountain"), DoIt), "fountain");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Fuel"), DoIt), "fuel");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Small place of burial"), DoIt), "grave_yard");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Hospital"), DoIt), "hospital");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Library"), DoIt), "library");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Nightclub"), DoIt), "nightclub");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Parking"), DoIt), "parking");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Pharmacy"), DoIt), "pharmacy");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Place of Worship"), DoIt), "place_of_worship");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Police Station"), DoIt), "police");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Post Box"), DoIt), "post_box");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Post Office"), DoIt), "post_office");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Prison"), DoIt), "prison");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Pub"), DoIt), "pub");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Public building"), DoIt), "public_building");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Public Telephone"), DoIt), "telephone");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Recycling Facilities"), DoIt), "recycling");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Restaurant"), DoIt), "restaurant");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","School"), DoIt), "school");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Taxi"), DoIt), "taxi");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Theatre or opera house"), DoIt), "theatre");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Toilets"), DoIt), "toilets");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Town hall"), DoIt), "townhall");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","University"), DoIt), "university");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Amenities","Waste Disposal"), DoIt), "waste_disposal");
}

void fillHighway(QComboBox* Box)
{
	bool DoIt = M_PREFS->getTranslateTags();
	addDefaults(Box, DoIt);
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Motorway"), DoIt), "motorway");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Ramp to motorway"), DoIt), "motorway_link");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Trunk road"), DoIt), "trunk");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Ramp to trunk road"), DoIt), "trunk_link");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Primary road"), DoIt), "primary");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Ramp to primary road"), DoIt), "primary_link");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Secondary road"), DoIt), "secondary");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Tertiary road"), DoIt), "tertiary");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Unclassified road"), DoIt), "unclassified");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Residential road"), DoIt), "residential");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Service road"), DoIt), "service");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Track road"), DoIt), "track");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Pedestrian priority road"), DoIt), "living_street");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Pedestrian only road"), DoIt), "pedestrian");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Footway"), DoIt), "footway");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Cycleway"), DoIt), "cycleway");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Bridleway"), DoIt), "bridleway");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Steps"), DoIt), "steps");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Bus guideway (not a bus way)"), DoIt), "bus_guideway");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("Highway","Unsurfaced road (old tag)"), DoIt), "unsurfaced");
}

void fillLandUse(QComboBox* Box)
{
	bool DoIt = M_PREFS->getTranslateTags();
	addDefaults(Box, DoIt);
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Allotments"), DoIt),"allotments");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Basin"), DoIt),"basin");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Brownfield"), DoIt),"brownfield");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Cemetery"), DoIt),"cemetery");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Commercial zone"), DoIt),"commercial");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Construction zone"), DoIt),"construction");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Farm"), DoIt),"farm");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Forest"), DoIt),"forest");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Greenfield"), DoIt),"greenfield");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Industrial zone"), DoIt),"industrial");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Landfill"), DoIt),"landfill");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Military"), DoIt),"military");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Recreation ground"), DoIt),"recreation_ground");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Reservoir (water)"), DoIt),"reservoir");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Residential zone"), DoIt),"residential");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Retail zone"), DoIt),"retail");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Surface mineral extraction"), DoIt),"quarry");
	Box->addItem(translateTag(QT_TRANSLATE_NOOP("landuse","Village green"), DoIt),"village_green");
}

void setTagComboBoxTo(QComboBox* Box, const QString& userData)
{
	for (int i=0; i<Box->count(); ++i)
		if (Box->itemData(i) == userData)
		{
			Box->setCurrentIndex(i);
			return;
		}
		else if (Box->itemData(i) == "__unknown")
			Box->setCurrentIndex(i);
}

void resetTagComboBox(QComboBox* Box, MapFeature* F, const QString& key)
{
	unsigned int idx = F->findKey(key);
	if (idx<F->tagSize())
		setTagComboBoxTo(Box,F->tagValue(idx));
	else
		setTagComboBoxTo(Box,"__unspecified");
}

void tagComboBoxActivated(QComboBox* Box, int idx, MapFeature* F, const QString& key, MapDocument* doc)
{
	if (Box->itemData(idx) == "__unspecified")
		doc->addHistory(new ClearTagCommand(F,key,doc->getDirtyOrOriginLayer(F->layer())));
	else if (Box->itemData(idx) != "__unknown")
		doc->addHistory(new SetTagCommand(F,key,Box->itemData(idx).toString(),doc->getDirtyOrOriginLayer(F->layer())));
}
