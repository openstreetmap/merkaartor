#ifndef MERKATOR_IMPORTGPX_H_
#define MERKATOR_IMPORTGPX_H_

#include <QVector>

class MapDocument;
class TrackMapLayer;

class QByteArray;
class QString;
class QWidget;
class QProgressDialog;

bool importGPX(QWidget* aParent, const QString& aFilename, MapDocument* theDocument, QVector<TrackMapLayer*>& theTracklayers);
bool importGPX(QWidget* aParent, QByteArray& aFile, MapDocument* theDocument, QVector<TrackMapLayer*>& theTracklayers, bool MakeSegment);

#endif


