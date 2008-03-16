#ifndef MERKATOR_IMPORTOSM_H_
#define MERKATOR_IMPORTOSM_H_

class CommandList;
class Downloader;
class MapDocument;
class MapFeature;
class MapLayer;

class QByteArray;
class QString;
class QWidget;

#include <QtXML/QXmlDefaultHandler>

class OSMHandler : public QXmlDefaultHandler
{
	public:
		OSMHandler(MapDocument* aDoc, MapLayer* aLayer, MapLayer* aConflict, CommandList* aList);

		virtual bool startElement ( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts );
		virtual bool endElement ( const QString & namespaceURI, const QString & localName, const QString & qName );

	private:
		void parseNode(const QXmlAttributes & atts);
		void parseTag(const QXmlAttributes & atts);
		void parseWay(const QXmlAttributes & atts);
		void parseNd(const QXmlAttributes & atts);
		void parseMember(const QXmlAttributes & atts);
		void parseRelation(const QXmlAttributes& atts);

		MapDocument* theDocument;
		MapLayer* theLayer;
		MapLayer* conflictLayer;
		CommandList* theList;
		MapFeature* Current;
		bool NewFeature;
};

bool importOSM(QWidget* aParent, const QString& aFilename, MapDocument* theDocument, MapLayer* theLayer);
bool importOSM(QWidget* aParent, QByteArray& Content, MapDocument* theDocument, MapLayer* theLayer, Downloader* theDownloader);

#endif