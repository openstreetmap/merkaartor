#include "Map/PreDefinedTags.h"
#include "Command/FeatureCommands.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/MapLayer.h"

#include <QtCore/QCoreApplication>
#include <QtGui/QComboBox>

// HACKING HINT
// The first parameter of addItem is the string as displayed to the user
// Surround it with a call to QCoreApplication::translate(Context,UserVisibleString)
// to make sure it gets translated, Context is a disambiguating help for translators
// The second parameter of addItem is the value of the tag


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
		translateTag("DefaultTags","Not specified", DoIt),
		"__unspecified");
	Box->addItem(
		translateTag("DefaultTags","Unknown", DoIt),
		"__unknown");
}

void fillAmenities(QComboBox* Box)
{
	bool DoIt = M_PREFS->getTranslateTags();
	addDefaults(Box, DoIt);
	Box->addItem(translateTag("Amenities","Arts centre",DoIt), "arts_centre");
	Box->addItem(translateTag("Amenities","ATM or cash point",DoIt), "atm");
	Box->addItem(translateTag("Amenities","Bank",DoIt), "bank");
	Box->addItem(translateTag("Amenities","Bank with atm",DoIt), "bank;atm");
	Box->addItem(translateTag("Amenities","Beer garden",DoIt), "biergarten");
	Box->addItem(translateTag("Amenities","Parking for bicycles", DoIt), "bicycle_parking");
	Box->addItem(translateTag("Amenities","Bicycle Rental", DoIt), "bicycle_rental");
	Box->addItem(translateTag("Amenities","Bureau de change", DoIt), "bureau_de_change");
	Box->addItem(translateTag("Amenities","Bus station", DoIt), "bus_station");
	Box->addItem(translateTag("Amenities","Cafe", DoIt), "cafe");
	Box->addItem(translateTag("Amenities","Car Rental", DoIt), "car_rental");
	Box->addItem(translateTag("Amenities","Car Sharing", DoIt), "car_sharing");
	Box->addItem(translateTag("Amenities","Cinema", DoIt), "cinema");
	Box->addItem(translateTag("Amenities","College", DoIt), "college");
	Box->addItem(translateTag("Amenities","Court house", DoIt), "courthouse");
	Box->addItem(translateTag("Amenities","Crematorium", DoIt), "crematorium");
	Box->addItem(translateTag("Amenities","Source of drinking water", DoIt), "drinking_water");
	Box->addItem(translateTag("Amenities","Fast food", DoIt), "Fast food");
	Box->addItem(translateTag("Amenities","Fire Station", DoIt), "fire_station");
	Box->addItem(translateTag("Amenities","Fountain", DoIt), "fountain");
	Box->addItem(translateTag("Amenities","Fuel", DoIt), "fuel");
	Box->addItem(translateTag("Amenities","Small place of burial", DoIt), "grave_yard");
	Box->addItem(translateTag("Amenities","Hospital", DoIt), "hospital");
	Box->addItem(translateTag("Amenities","Library", DoIt), "library");
	Box->addItem(translateTag("Amenities","Nightclub", DoIt), "nightclub");
	Box->addItem(translateTag("Amenities","Parking", DoIt), "parking");
	Box->addItem(translateTag("Amenities","Pharmacy", DoIt), "pharmacy");
	Box->addItem(translateTag("Amenities","Place of Worship", DoIt), "place_of_worship");
	Box->addItem(translateTag("Amenities","Police Station", DoIt), "police");
	Box->addItem(translateTag("Amenities","Post Box", DoIt), "post_box");
	Box->addItem(translateTag("Amenities","Post Office", DoIt), "post_office");
	Box->addItem(translateTag("Amenities","Prison", DoIt), "prison");
	Box->addItem(translateTag("Amenities","Pub", DoIt), "pub");
	Box->addItem(translateTag("Amenities","Public building", DoIt), "public_building");
	Box->addItem(translateTag("Amenities","Public Telephone", DoIt), "telephone");
	Box->addItem(translateTag("Amenities","Recycling Facilities", DoIt), "recycling");
	Box->addItem(translateTag("Amenities","Restaurant", DoIt), "restaurant");
	Box->addItem(translateTag("Amenities","School", DoIt), "school");
	Box->addItem(translateTag("Amenities","Taxi", DoIt), "taxi");
	Box->addItem(translateTag("Amenities","Theatre or opera house", DoIt), "theatre");
	Box->addItem(translateTag("Amenities","Toilets", DoIt), "toilets");
	Box->addItem(translateTag("Amenities","Town hall", DoIt), "townhall");
	Box->addItem(translateTag("Amenities","University", DoIt), "university");
	Box->addItem(translateTag("Amenities","Waste Disposal", DoIt), "waste_disposal");
}

void fillHighway(QComboBox* Box)
{
	bool DoIt = M_PREFS->getTranslateTags();
	addDefaults(Box, DoIt);
	Box->addItem(translateTag("Highway","Motorway", DoIt), "motorway");
	Box->addItem(translateTag("Highway","Ramp to motorway", DoIt), "motorway_link");
	Box->addItem(translateTag("Highway","Trunk road", DoIt), "trunk");
	Box->addItem(translateTag("Highway","Ramp to trunk road", DoIt), "trunk_link");
	Box->addItem(translateTag("Highway","Primary road", DoIt), "primary");
	Box->addItem(translateTag("Highway","Ramp to primary road", DoIt), "primary_link");
	Box->addItem(translateTag("Highway","Secondary road", DoIt), "secondary");
	Box->addItem(translateTag("Highway","Tertiary road", DoIt), "tertiary");
	Box->addItem(translateTag("Highway","Unclassified road", DoIt), "unclassified");
	Box->addItem(translateTag("Highway","Residential road", DoIt), "residential");
	Box->addItem(translateTag("Highway","Service road", DoIt), "service");
	Box->addItem(translateTag("Highway","Track road", DoIt), "track");
	Box->addItem(translateTag("Highway","Pedestrian priority road", DoIt), "living_street");
	Box->addItem(translateTag("Highway","Pedestrian only road", DoIt), "pedestrian");
	Box->addItem(translateTag("Highway","Footway", DoIt), "footway");
	Box->addItem(translateTag("Highway","Cycleway", DoIt), "cycleway");
	Box->addItem(translateTag("Highway","Bridleway", DoIt), "bridleway");
	Box->addItem(translateTag("Highway","Steps", DoIt), "steps");
	Box->addItem(translateTag("Highway","Bus guideway (not a bus way)", DoIt), "bus_guideway");
	Box->addItem(translateTag("Highway","Unsurfaced road (old tag)", DoIt), "unsurfaced");
}

void fillLandUse(QComboBox* Box)
{
	bool DoIt = M_PREFS->getTranslateTags();
	addDefaults(Box, DoIt);
	Box->addItem(translateTag("landuse","Allotments", DoIt),"allotments");
	Box->addItem(translateTag("landuse","Basin", DoIt),"basin");
	Box->addItem(translateTag("landuse","Brownfield", DoIt),"brownfield");
	Box->addItem(translateTag("landuse","Cemetery", DoIt),"cemetery");
	Box->addItem(translateTag("landuse","Commercial zone", DoIt),"commercial");
	Box->addItem(translateTag("landuse","Construction zone", DoIt),"construction");
	Box->addItem(translateTag("landuse","Farm", DoIt),"farm");
	Box->addItem(translateTag("landuse","Forest", DoIt),"forest");
	Box->addItem(translateTag("landuse","Greenfield", DoIt),"greenfield");
	Box->addItem(translateTag("landuse","Industrial zone", DoIt),"industrial");
	Box->addItem(translateTag("landuse","Landfill", DoIt),"landfill");
	Box->addItem(translateTag("landuse","Military", DoIt),"military");
	Box->addItem(translateTag("landuse","Recreation ground", DoIt),"recreation_ground");
	Box->addItem(translateTag("landuse","Reservoir (water)", DoIt),"reservoir");
	Box->addItem(translateTag("landuse","Residential zone", DoIt),"residential");
	Box->addItem(translateTag("landuse","Retail zone", DoIt),"retail");
	Box->addItem(translateTag("landuse","Surface mineral extraction", DoIt),"quarry");
	Box->addItem(translateTag("landuse","Village green", DoIt),"village_green");
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
