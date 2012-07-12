#ifndef MERKATOR_IMPORTGPX_H_
#define MERKATOR_IMPORTGPX_H_

#include <QList>

class Document;
class TrackLayer;

class QByteArray;
class QString;
class QWidget;

bool importGPX(const QString& aFilename, Document* theDocument, QList<TrackLayer*>& theTracklayers);
bool importGPX(QByteArray& aFile, Document* theDocument, QList<TrackLayer*>& theTracklayers, bool MakeSegment);

#endif


