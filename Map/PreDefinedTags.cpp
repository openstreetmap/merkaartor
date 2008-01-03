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
	Box->addItem(QCoreApplication::translate("Amenities","Hospital"), "hospital");
	Box->addItem(QCoreApplication::translate("Amenities","Parking"), "parking");
	Box->addItem(QCoreApplication::translate("Amenities","School"), "school");
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
