#ifndef MERKAARTOR_PREDEFINEDTAGS_H_
#define MERKAARTOR_PREDEFINEDTAGS_H_

class MapDocument;
class MapFeature;

class QComboBox;
class QString;

void fillAmenities(QComboBox* Box);
void fillHighway(QComboBox* Box);

void resetTagComboBox(QComboBox* Box, MapFeature* F, const QString& key);
void tagComboBoxActivated(QComboBox* Box, int idx, MapFeature* F, const QString& key, MapDocument* doc);

#endif
