#ifndef MERKATOR_IMPORTGPX_H_
#define MERKATOR_IMPORTGPX_H_

class MapDocument;
class MapLayer;

class QByteArray;
class QString;
class QWidget;

bool importGPX(QWidget* aParent, const QString& aFilename, MapDocument* theDocument, MapLayer* theLayer);
bool importGPX(QWidget* aParent, QByteArray& aFile, MapDocument* theDocument, MapLayer* theLayer, bool MakeSegment);

#endif


