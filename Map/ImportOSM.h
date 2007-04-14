#ifndef MERKATOR_IMPORTOSM_H_
#define MERKATOR_IMPORTOSM_H_

class Downloader;
class MapDocument;
class MapLayer;

class QByteArray;
class QString;
class QWidget;

bool importOSM(QWidget* aParent, const QString& aFilename, MapDocument* theDocument, MapLayer* theLayer);
bool importOSM(QWidget* aParent, QByteArray& aFile, MapDocument* theDocument, MapLayer* theLayer, Downloader* theDownloader);

#endif


