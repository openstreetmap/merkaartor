#ifndef MERKATOR_IMPORTOSM_H_
#define MERKATOR_IMPORTOSM_H_

class CommandList;
class Downloader;
class Document;
class Feature;
class Layer;
class Way;
class Relation;

class QByteArray;
class QString;
class QWidget;

#include <QXmlDefaultHandler>
#include <QSet>

class OSMHandler : public QXmlDefaultHandler
{
public:
    OSMHandler(Document* aDoc, Layer* aLayer, Layer* aConflict);

    virtual bool startElement ( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts );
    virtual bool endElement ( const QString & namespaceURI, const QString & localName, const QString & qName );

private:
    void parseNode(const QXmlAttributes & atts);
    void parseTag(const QXmlAttributes & atts);
    void parseWay(const QXmlAttributes & atts);
    void parseNd(const QXmlAttributes & atts);
    void parseMember(const QXmlAttributes & atts);
    void parseRelation(const QXmlAttributes& atts);

    Document* theDocument;
    Layer* theLayer;
    Layer* conflictLayer;
    Feature* Current;
    bool NewFeature;

public:
        QSet<Way*> touchedWays;
        QSet<Relation*> touchedRelations;
};

bool importOSM(QWidget* aParent, const QString& aFilename, Document* theDocument, Layer* theLayer);
bool importOSM(QWidget* aParent, QByteArray& Content, Document* theDocument, Layer* theLayer, Downloader* theDownloader);

#endif
