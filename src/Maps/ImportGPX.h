#ifndef MERKATOR_IMPORTGPX_H_
#define MERKATOR_IMPORTGPX_H_

#include <QList>

class MapDocument;
class TrackMapLayer;

class QByteArray;
class QString;
class QWidget;

bool importGPX(QWidget* aParent, const QString& aFilename, MapDocument* theDocument, QList<TrackMapLayer*>& theTracklayers);
bool importGPX(QWidget* aParent, QByteArray& aFile, MapDocument* theDocument, QList<TrackMapLayer*>& theTracklayers, bool MakeSegment);

#endif


