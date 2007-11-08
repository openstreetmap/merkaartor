#ifndef MERKAARTOR_IMPORTNGT_H_
#define MERKAARTOR_IMPORTNGT_H_

class MapLayer;
class MapDocument;
class QString;
class QWidget;

bool importNGT(QWidget* aParent, const QString& aFilename, MapDocument* theDocument, MapLayer* theLayer);

#endif