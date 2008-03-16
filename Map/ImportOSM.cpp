#include "Map/ImportOSM.h"

#include "Command/Command.h"
#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Command/RelationCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Map/DownloadOSM.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "Map/Relation.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Map/TrackSegment.h"

#include <QtCore/QBuffer>
#include <QtCore/QDateTime>
#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressBar>
#include <QtGui/QProgressDialog>
#include <QtXml/QDomDocument>
#include <QtXml/QXmlAttributes>


static TrackPoint* getTrackPointOrCreatePlaceHolder(MapDocument *theDocument, MapLayer *theLayer, CommandList *theList, const QString& Id)
{
	TrackPoint* Part = dynamic_cast<TrackPoint*>(theDocument->get("node_"+Id));
	if (!Part)
	{
		Part = new TrackPoint(Coord(0,0));
		Part->setId("node_"+Id);
		Part->setLastUpdated(MapFeature::NotYetDownloaded);
		theList->add(new AddFeatureCommand(theLayer, Part, false));
	}
	return Part;
}

static Road* getWayOrCreatePlaceHolder(MapDocument *theDocument, MapLayer *theLayer, CommandList *theList, const QString& Id)
{
	Road* Part = dynamic_cast<Road*>(theDocument->get("way_"+Id));
	if (!Part)
	{
		Part = new Road;
		Part->setId("way_"+Id);
		Part->setLastUpdated(MapFeature::NotYetDownloaded);
		theList->add(new AddFeatureCommand(theLayer, Part, false));
	}
	return Part;
}

static Relation* getRelationOrCreatePlaceHolder(MapDocument *theDocument, MapLayer *theLayer, CommandList *theList, const QString& Id)
{
	Relation* Part = dynamic_cast<Relation*>(theDocument->get("rel_"+Id));
	if (!Part)
	{
		Part = new Relation;
		Part->setId("rel_"+Id);
		Part->setLastUpdated(MapFeature::NotYetDownloaded);
		theList->add(new AddFeatureCommand(theLayer, Part, false));
	}
	return Part;
}


OSMHandler::OSMHandler(MapDocument* aDoc, MapLayer* aLayer, MapLayer* aConflict, CommandList* aList)
: theDocument(aDoc), theLayer(aLayer), conflictLayer(aConflict), theList(aList), Current(0)
{
}

void OSMHandler::parseTag(const QXmlAttributes &atts)
{
	if (!Current) return;
	if (NewFeature)
		Current->setTag(atts.value("k"),atts.value("v"));
	else
		theList->add(new SetTagCommand(Current, atts.value("k"),atts.value("v")));
}

void OSMHandler::parseNode(const QXmlAttributes& atts)
{
	double Lat = atts.value("lat").toDouble();
	double Lon = atts.value("lon").toDouble();
	QString id = "node_"+atts.value("id");
	TrackPoint* Pt = dynamic_cast<TrackPoint*>(theDocument->get(id));
	if (Pt)
	{
		if (Pt->lastUpdated() == MapFeature::User)
		{
			// conflict
			Pt->setLastUpdated(MapFeature::UserResolved);
			Pt = new TrackPoint(Coord(angToRad(Lat),angToRad(Lon)));
			theList->add(new AddFeatureCommand(conflictLayer, Pt, false));
			NewFeature = true;
			Pt->setId("conflict_"+id);
			Pt->setLastUpdated(MapFeature::OSMServerConflict);
		}
		else if (Pt->lastUpdated() != MapFeature::UserResolved)
		{
			theList->add(new MoveTrackPointCommand(Pt,Coord(angToRad(Lat),angToRad(Lon))));
			NewFeature = false;
			if (Pt->lastUpdated() == MapFeature::NotYetDownloaded)
				Pt->setLastUpdated(MapFeature::OSMServer);
		}
	}
	else
	{
		Pt = new TrackPoint(Coord(angToRad(Lat),angToRad(Lon)));
		theList->add(new AddFeatureCommand(theLayer,Pt, false));
		NewFeature = true;
		Pt->setId(id);
		Pt->setLastUpdated(MapFeature::OSMServer);
	}
	Current = Pt;
}

void OSMHandler::parseNd(const QXmlAttributes& atts)
{
	Road* R = dynamic_cast<Road*>(Current);
	if (!R) return;
	TrackPoint *Part = getTrackPointOrCreatePlaceHolder(theDocument, theLayer, theList, atts.value("ref"));
	if (NewFeature)
		R->add(Part);
	else
		theList->add(new RoadAddTrackPointCommand(R,Part));
}

void OSMHandler::parseWay(const QXmlAttributes& atts)
{
	QString id = "way_"+atts.value("id");
	Road* R = dynamic_cast<Road*>(theDocument->get(id));
	if (R)
	{
		if (R->lastUpdated() == MapFeature::User)
		{
			R->setLastUpdated(MapFeature::UserResolved);
			NewFeature = false;
			// conflict
/*				TrackPoint* Conflict = dynamic_cast<TrackPoint*>(theDocument->get("conflict_node_"+Root.attribute("from")));
			if (Conflict) From = Conflict;
			Conflict = dynamic_cast<TrackPoint*>(theDocument->get("conflict_node_"+Root.attribute("to")));
			if (Conflict) To = Conflict;
			Way* W = new Way(From,To);
			W->setId("conflict_"+id);
			loadSegmentTags(Root,W);
			theList->add(new AddFeatureCommand(conflictLayer,W, false));
			W->setLastUpdated(MapFeature::OSMServerConflict); */
		}
		else if (R->lastUpdated() != MapFeature::UserResolved)
		{
			while (R->size())
				theList->add(new RoadRemoveTrackPointCommand(R,R->get(0)));
			NewFeature = false;
		}
	}
	else
	{
		R = new Road;
		theList->add(new AddFeatureCommand(theLayer,R, false));
		NewFeature = true;
		R->setId(id);
		R->setLastUpdated(MapFeature::OSMServer);
	}
	Current = R;
}

void OSMHandler::parseMember(const QXmlAttributes& atts)
{
	Relation* R = dynamic_cast<Relation*>(Current);
	if (!R)
		return;
	QString Type = atts.value("type");
	MapFeature* F = 0;
	if (Type == "node")
		F = getTrackPointOrCreatePlaceHolder(theDocument, theLayer, theList, atts.value("ref"));
	else if (Type == "way")
		F = getWayOrCreatePlaceHolder(theDocument, theLayer, theList, atts.value("ref"));
	else if (Type == "relation")
		F = getRelationOrCreatePlaceHolder(theDocument, theLayer, theList, atts.value("ref"));
	if (F)
	{
		if (NewFeature)
			R->add(atts.value("role"),F);
		else
			theList->add(new RelationAddFeatureCommand(R,atts.value("role"),F));
	}
}

void OSMHandler::parseRelation(const QXmlAttributes& atts)
{
	QString id = "rel_"+atts.value("id");
	Relation* R = dynamic_cast<Relation*>(theDocument->get(id));
	if (R)
	{
		if (R->lastUpdated() == MapFeature::User)
		{
			R->setLastUpdated(MapFeature::UserResolved);
			NewFeature = true;
			// conflict
/*				TrackPoint* Conflict = dynamic_cast<TrackPoint*>(theDocument->get("conflict_node_"+Root.attribute("from")));
			if (Conflict) From = Conflict;
			Conflict = dynamic_cast<TrackPoint*>(theDocument->get("conflict_node_"+Root.attribute("to")));
			if (Conflict) To = Conflict;
			Way* W = new Way(From,To);
			W->setId("conflict_"+id);
			loadSegmentTags(Root,W);
			theList->add(new AddFeatureCommand(conflictLayer,W, false));
			W->setLastUpdated(MapFeature::OSMServerConflict); */
		}
		else if (R->lastUpdated() != MapFeature::UserResolved)
		{
			while (R->size())
				theList->add(new RelationRemoveFeatureCommand(R,R->get(0)));
			NewFeature = false;
		}
	}
	else
	{
		R = new Relation;
		NewFeature = true;
		R->setId(id);
		theList->add(new AddFeatureCommand(theLayer,R, false));
		R->setLastUpdated(MapFeature::OSMServer);
	}
	Current = R;
}

bool OSMHandler::startElement ( const QString &, const QString & localName, const QString & qName, const QXmlAttributes & atts )
{
	if (qName == "tag")
		parseTag(atts);
	else if (qName == "node")
		parseNode(atts);
	else if (qName == "nd")
		parseNd(atts);
	else if (qName == "way")
		parseWay(atts);
	else if (qName == "member")
		parseMember(atts);
	else if (qName == "relation")
		parseRelation(atts);
	return true;
}

bool OSMHandler::endElement ( const QString &, const QString & localName, const QString & qName )
{
	if (qName == "node")
		Current = 0;
	return true;
}

static bool downloadToResolve(const QString& What, const std::vector<MapFeature*>& Resolution, QProgressDialog* dlg, MapDocument* theDocument, MapLayer* theLayer, CommandList* theList, Downloader* theDownloader)
{
	for (unsigned int i=0; i<Resolution.size(); i+=10 )
	{
		QString URL = theDownloader->getURLToFetch(What);
		URL+=Resolution[i]->id().right(Resolution[i]->id().length()-Resolution[i]->id().lastIndexOf('_')-1);
		for (unsigned int j=1; j<10; ++j)
			if (i+j < Resolution.size())
			{
				URL+=",";
				URL+=Resolution[i+j]->id().right(Resolution[i+j]->id().length()-Resolution[i+j]->id().lastIndexOf('_')-1);
			}
		dlg->setLabelText(QString("downloading segment %1 of %2").arg(i).arg(Resolution.size()));
		if (theDownloader->go(URL))
		{
			if (theDownloader->resultCode() == 410)
				theList->add(new RemoveFeatureCommand(theDocument, Resolution[i], std::vector<MapFeature*>()));
			else
			{
				QByteArray ba(theDownloader->content());
				QBuffer  File(&ba);
				dlg->setLabelText(QString("parsing segment %1 of %2").arg(i).arg(Resolution.size()));

				QDomDocument DomDoc;
				QString ErrorStr;
				int ErrorLine;
				int ErrorColumn;
				if (DomDoc.setContent(&File, true, &ErrorStr, &ErrorLine,&ErrorColumn))
				{
					QDomElement root = DomDoc.documentElement();
//					if (root.tagName() == "osm")
//						importOSM(0, root, theDocument, theLayer, 0, theList, 0);
				}
			}
		}
		else
			return false;
		dlg->setValue(i);
	}
	return true;
}

static bool resolveNotYetDownloaded(QProgressDialog* dlg, MapDocument* theDocument, MapLayer* theLayer, CommandList* theList, Downloader* theDownloader)
{
	if (theDownloader)
	{
		// resolve nodes FIXME make this for nodes only or templatize?
		std::vector<MapFeature*> MustResolve;
		MustResolve.clear();
		for (unsigned int i=0; i<theLayer->size(); ++i)
		{
			TrackPoint* P = dynamic_cast<TrackPoint*>(theLayer->get(i));
			if (P && P->lastUpdated() == MapFeature::NotYetDownloaded)
				MustResolve.push_back(P);
		}
		if (MustResolve.size())
		{
			dlg->setMaximum(MustResolve.size());
			dlg->setValue(0);
			dlg->show();
			if (!downloadToResolve("nodes",MustResolve,dlg,theDocument,theLayer, theList,theDownloader))
				return false;
		}
	}
	for (unsigned int i=theLayer->size(); i; --i)
	{
		if (theLayer->get(i-1)->notEverythingDownloaded())
		{
//			bool x = theLayer->get(i-1)->notEverythingDownloaded();
			theList->add(new RemoveFeatureCommand(theDocument,theLayer->get(i-1)));
		}
	}
	return true;
}

bool importOSM(QWidget* aParent, QIODevice& File, MapDocument* theDocument, MapLayer* theLayer, Downloader* theDownloader)
{
	QDomDocument DomDoc;
	QString ErrorStr;
	int ErrorLine;
	int ErrorColumn;
	QProgressDialog* dlg = new QProgressDialog(aParent);
	QProgressBar* Bar = new QProgressBar(dlg);
	dlg->setBar(Bar);
	dlg->setWindowModality(Qt::ApplicationModal);
	dlg->setMinimumDuration(0);
	dlg->setLabelText("Parsing XML");
	dlg->show();
	if (theDownloader)
		theDownloader->setAnimator(dlg,Bar,false);
	CommandList* theList = new CommandList;
	theList->setIsUpdateFromOSM();
	MapLayer* conflictLayer = new DrawingMapLayer("Conflicts from "+theLayer->name());
	theDocument->add(conflictLayer);

	OSMHandler theHandler(theDocument,theLayer,conflictLayer,theList);

	QXmlSimpleReader xmlReader;
	xmlReader.setContentHandler(&theHandler);
	QXmlInputSource source;
	QByteArray buf(File.read(10240));
	source.setData(buf);
	xmlReader.parse(&source,true);
	Bar->setMaximum(File.size());
	Bar->setValue(Bar->value()+buf.size());
	while (!File.atEnd())
	{
		QByteArray buf(File.read(20480));
		source.setData(buf);
		xmlReader.parseContinue();
		Bar->setValue(Bar->value()+buf.size());
		qApp->processEvents();
		if (dlg->wasCanceled())
			break;
	}
	bool ok = xmlReader.parse(source);

	bool WasCanceled = dlg->wasCanceled();
	if (!WasCanceled)
		WasCanceled = !resolveNotYetDownloaded(dlg,theDocument,theLayer,theList,theDownloader);
	delete dlg;
	if (WasCanceled)
	{
		theDocument->remove(conflictLayer);
		delete conflictLayer;
		delete theList;
		return false;
	}
	else
	{
		//FIXME Do we have to add a download to the undo list? + dirties the document
		//theDocument->history().add(theList);

               // If we decide not to, we should at least delete the object. Better not to
               // create theList at all.
               delete theList;

		if (!conflictLayer->size()) {
			theDocument->remove(conflictLayer);
			delete conflictLayer;
		}
	}
	return true;
}

bool importOSM(QWidget* aParent, const QString& aFilename, MapDocument* theDocument, MapLayer* theLayer)
{
	QFile File(aFilename);
	if (!File.open(QIODevice::ReadOnly))
		 return false;
	return importOSM(aParent, File, theDocument, theLayer, 0 );
}

bool importOSM(QWidget* aParent, QByteArray& Content, MapDocument* theDocument, MapLayer* theLayer, Downloader* theDownloader)
{
	QBuffer File(&Content);
	File.open(QIODevice::ReadOnly);
	return importOSM(aParent, File, theDocument, theLayer, theDownloader);
}



