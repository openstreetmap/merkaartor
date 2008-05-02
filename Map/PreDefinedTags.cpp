#include "Map/PreDefinedTags.h"
#include "Command/FeatureCommands.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"

#include <QtCore/QCoreApplication>
#include <QtGui/QComboBox>

// HACKING HINT
// The first parameter of addItem is the string as displayed to the user
// Surround it with a call to QCoreApplication::translate(Context,UserVisibleString)
// to make sure it gets translated, Context is a disambiguating help for translators
// The second parameter of addItem is the value of the tag

void addDefaults(QComboBox* Box)
{
	Box->addItem(
		QCoreApplication::translate("DefaultTags","Not specified"),
		"__unspecified");
	Box->addItem(
		QCoreApplication::translate("DefaultTags","Unknown"),
		"__unknown");
}

void fillAmenities(QComboBox* Box)
{
	addDefaults(Box);
	Box->addItem(QCoreApplication::translate("Amenities","College"), "college");
	Box->addItem(QCoreApplication::translate("Amenities","Fire Station"), "fire_station");
	Box->addItem(QCoreApplication::translate("Amenities","Fuel"), "fuel");
	Box->addItem(QCoreApplication::translate("Amenities","Hospital"), "hospital");
	Box->addItem(QCoreApplication::translate("Amenities","Library"), "library");
	Box->addItem(QCoreApplication::translate("Amenities","Parking"), "parking");
	Box->addItem(QCoreApplication::translate("Amenities","Place of Worship"), "place_of_worship");
	Box->addItem(QCoreApplication::translate("Amenities","Police Station"), "police");
	Box->addItem(QCoreApplication::translate("Amenities","Post Box"), "post_box");
	Box->addItem(QCoreApplication::translate("Amenities","Post Office"), "post_office");
	Box->addItem(QCoreApplication::translate("Amenities","Pub"), "pub");
	Box->addItem(QCoreApplication::translate("Amenities","Public Telephone"), "telephone");
	Box->addItem(QCoreApplication::translate("Amenities","Recycling Facilities"), "recycling");
	Box->addItem(QCoreApplication::translate("Amenities","Restaurant"), "restaurant");
	Box->addItem(QCoreApplication::translate("Amenities","School"), "school");
	Box->addItem(QCoreApplication::translate("Amenities","University"), "university");
}

void fillHighway(QComboBox* Box)
{
	addDefaults(Box);
	Box->addItem(QCoreApplication::translate("Highway","Motorway"), "motorway");
	Box->addItem(QCoreApplication::translate("Highway","Ramp to motorway"), "motorway_link");
	Box->addItem(QCoreApplication::translate("Highway","Trunk road"), "trunk");
	Box->addItem(QCoreApplication::translate("Highway","Ramp to trunk road"), "trunk_link");
	Box->addItem(QCoreApplication::translate("Highway","Primary road"), "primary");
	Box->addItem(QCoreApplication::translate("Highway","Ramp to primary road"), "primary_link");
	Box->addItem(QCoreApplication::translate("Highway","Secondary road"), "secondary");
	Box->addItem(QCoreApplication::translate("Highway","Tertiary road"), "tertiary");
	Box->addItem(QCoreApplication::translate("Highway","Residential road"), "residential");
	Box->addItem(QCoreApplication::translate("Highway","Pedestrian priority road"), "living_street");
	Box->addItem(QCoreApplication::translate("Highway","Service road"), "service");
	Box->addItem(QCoreApplication::translate("Highway","Unsurfaced road"), "unsurfaced");
	Box->addItem(QCoreApplication::translate("Highway","Track road"), "track");
	Box->addItem(QCoreApplication::translate("Highway","Cycleway"), "cycleway");
	Box->addItem(QCoreApplication::translate("Highway","Footway"), "footway");
	Box->addItem(QCoreApplication::translate("Highway","Bridleway"), "bridleway");
	Box->addItem(QCoreApplication::translate("Highway","Steps"), "steps");
}

void fillLandUse(QComboBox* Box)
{
	addDefaults(Box);
	Box->addItem(QCoreApplication::translate("landuse","Forest"),"forest");
	Box->addItem(QCoreApplication::translate("landuse","Farm"),"farm");
	Box->addItem(QCoreApplication::translate("landuse","Recreation ground"),"recreation_ground");
	Box->addItem(QCoreApplication::translate("landuse","Village green"),"village_green");
	Box->addItem(QCoreApplication::translate("landuse","Residential zone"),"residential");
	Box->addItem(QCoreApplication::translate("landuse","Industrial zone"),"industrial");
	Box->addItem(QCoreApplication::translate("landuse","Retail zone"),"retail");
	Box->addItem(QCoreApplication::translate("landuse","Commercial zone"),"commercial");
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
		doc->history().add(new ClearTagCommand(F,key));
	else if (Box->itemData(idx) != "__unknown")
		doc->history().add(new SetTagCommand(F,key,Box->itemData(idx).toString()));
}
