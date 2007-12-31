#include "Map/ImportOSM.h"

#include "Command/Command.h"
#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Command/RelationCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Map/DownloadOSM.h"
#include "Map/MapDocument.h"
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

/*
 * Forward decls
 */
static void loadTags(const QDomElement& Root, MapFeature* W);
static void loadTags(const QDomElement& Root, MapFeature* W, CommandList* theList);
static void importOSM(QProgressDialog* dlg, const QDomElement& Root, MapDocument* theDocument, MapLayer* theLayer, MapLayer* conflictLayer, CommandList* theList, Downloader* theDownloader);

static void importNode(const QDomElement& Root, MapDocument* theDocument, MapLayer* theLayer, MapLayer* conflictLayer, CommandList* theList)
{
	double Lat = Root.attribute("lat").toDouble();
	double Lon = Root.attribute("lon").toDouble();
	QString id = "node_"+Root.attribute("id");
	TrackPoint* Pt = dynamic_cast<TrackPoint*>(theDocument->get(id));
	if (Pt)
	{
		if (Pt->lastUpdated() == MapFeature::User)
		{
			// conflict
			Pt->setLastUpdated(MapFeature::UserResolved);
			Pt = new TrackPoint(Coord(angToRad(Lat),angToRad(Lon)));
			loadTags(Root,Pt);
			Pt->setId("conflict_"+id);
			Pt->setLastUpdated(MapFeature::OSMServerConflict);
			theList->add(new AddFeatureCommand(conflictLayer, Pt, false));
		}
		else if (Pt->lastUpdated() != MapFeature::UserResolved)
		{
			theList->add(new MoveTrackPointCommand(Pt,Coord(angToRad(Lat),angToRad(Lon))));
			loadTags(Root, Pt, theList);
			if (Pt->lastUpdated() == MapFeature::NotYetDownloaded)
				Pt->setLastUpdated(MapFeature::OSMServer);
		}
	}
	else
	{
		Pt = new TrackPoint(Coord(angToRad(Lat),angToRad(Lon)));
		Pt->setId(id);
		loadTags(Root, Pt);
		Pt->setLastUpdated(MapFeature::OSMServer);
		theList->add(new AddFeatureCommand(theLayer,Pt, false));
	}
}

static void loadTags(const QDomElement& Root, MapFeature* W)
{
	for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement t = n.toElement();
		if (!t.isNull())
		{
			if (t.tagName() == "tag")
				W->setTag(t.attribute("k"),t.attribute("v"));
		}
	}
}

static void loadTags(const QDomElement& Root, MapFeature* W, CommandList* theList)
{
	theList->add(new ClearTagsCommand(W));
	for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement t = n.toElement();
		if (!t.isNull())
		{
			if (t.tagName() == "tag")
				theList->add(new SetTagCommand(W, t.attribute("k"),t.attribute("v")));
		}
	}
}

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

static void importWay(QProgressDialog* dlg, const QDomElement& Root, MapDocument* theDocument, MapLayer* theLayer, 
					  MapLayer* conflictLayer, CommandList* theList, Downloader* theDownloader)
{
	std::vector<TrackPoint*> Pts;
	for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement t = n.toElement();
		if (!t.isNull())
		{
			if (t.tagName() == "nd")
			{
				TrackPoint *Part = getTrackPointOrCreatePlaceHolder(theDocument, theLayer, theList, t.attribute("ref"));
				Pts.push_back(Part);
			}
		}
	}
	if (Pts.size())
	{
		QString id = "way_"+Root.attribute("id");
		Road* R = dynamic_cast<Road*>(theDocument->get(id));
		if (R)
		{
			if (R->lastUpdated() == MapFeature::User)
			{
				R->setLastUpdated(MapFeature::UserResolved);
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
				for (unsigned int i=0; i<Pts.size(); ++i)
					theList->add(new RoadAddTrackPointCommand(R,Pts[i]));
				loadTags(Root,R, theList);
			}
		}
		else
		{
			R = new Road;
			for (unsigned int i=0; i<Pts.size(); ++i)
				R->add(Pts[i]);
			R->setId(id);
			loadTags(Root,R);
			theList->add(new AddFeatureCommand(theLayer,R, false));
			R->setLastUpdated(MapFeature::OSMServer);
		}
	}
}


static void importRelation(QProgressDialog* dlg, const QDomElement& Root, MapDocument* theDocument, MapLayer* theLayer, 
					  MapLayer* conflictLayer, CommandList* theList, Downloader* theDownloader)
{
	std::vector<std::pair<QString,MapFeature*> > Fts;
	for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement t = n.toElement();
		if (!t.isNull())
		{
			if (t.tagName() == "member")
			{
				QString Type = t.attribute("type");
				MapFeature* F = 0;
				if (Type == "node")
					F = getTrackPointOrCreatePlaceHolder(theDocument, theLayer, theList, t.attribute("ref"));
				else if (Type == "way")
					F = getWayOrCreatePlaceHolder(theDocument, theLayer, theList, t.attribute("ref"));
				else if (Type == "relation")
					F = getRelationOrCreatePlaceHolder(theDocument, theLayer, theList, t.attribute("ref"));
				if (F)
					Fts.push_back(std::make_pair(t.attribute("role"),F));
			}
		}
	}
	if (Fts.size())
	{
		QString id = "rel_"+Root.attribute("id");
		Relation* R = dynamic_cast<Relation*>(theDocument->get(id));
		if (R)
		{
			if (R->lastUpdated() == MapFeature::User)
			{
				R->setLastUpdated(MapFeature::UserResolved);
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
				for (unsigned int i=0; i<Fts.size(); ++i)
					theList->add(new RelationAddFeatureCommand(R,Fts[i].first,Fts[i].second));
				loadTags(Root,R, theList);
			}
		}
		else
		{
			R = new Relation;
			for (unsigned int i=0; i<Fts.size(); ++i)
				R->add(Fts[i].first,Fts[i].second);
			R->setId(id);
			loadTags(Root,R);
			theList->add(new AddFeatureCommand(theLayer,R, false));
			R->setLastUpdated(MapFeature::OSMServer);
		}
	}
}


static void importOSM(QProgressDialog* dlg, const QDomElement& Root, MapDocument* theDocument, MapLayer* theLayer, MapLayer* conflictLayer, CommandList* theList, Downloader* theDownloader)
{
	unsigned int Count = 0;
	for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
		++Count;
	unsigned int Done = 0;
	if (dlg)
		dlg->setMaximum(Count);
	QEventLoop ev;
	for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement t = n.toElement();
		if (!t.isNull())
		{
			if (t.tagName() == "node")
				importNode(t,theDocument, theLayer, conflictLayer, theList);
			else if (t.tagName() == "way")
				importWay(dlg, t,theDocument, theLayer, conflictLayer, theList, theDownloader);
			else if (t.tagName() == "relation")
				importRelation(dlg, t,theDocument, theLayer, conflictLayer, theList, theDownloader);
		}
		++Done;
		if (dlg)
		{
			dlg->setValue(Done);
			ev.processEvents();
			if (dlg->wasCanceled()) return;
		}
	}
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
					if (root.tagName() == "osm")
						importOSM(0, root, theDocument, theLayer, 0, theList, 0);
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
			bool x = theLayer->get(i-1)->notEverythingDownloaded();
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
/*	QFile debug("c:\\temp\\debug.osm");
	debug.open(QIODevice::Truncate|QIODevice::WriteOnly);
	File.reset();
	debug.write(File.readAll());
	File.close(); */
	if (!DomDoc.setContent(&File, true, &ErrorStr, &ErrorLine,&ErrorColumn))
	{
		QMessageBox::warning(aParent,"Parse error",
			QString("Parse error at line %1, column %2:\n%3")
                                  .arg(ErrorLine)
                                  .arg(ErrorColumn)
                                  .arg(ErrorStr));
		return false;
	}


	QDomElement root = DomDoc.documentElement();
	if (root.tagName() != "osm")
	{
		QMessageBox::information(aParent, "Parse error","Root is not an osm node");
		return false;
	}
	CommandList* theList = new CommandList;
	theList->setIsUpdateFromOSM();
	theDocument->add(theLayer);
	MapLayer* conflictLayer = new MapLayer("Conflicts from "+theLayer->name());
	importOSM(dlg, root, theDocument, theLayer, conflictLayer, theList, theDownloader);
	bool WasCanceled = dlg->wasCanceled();
	if (!WasCanceled)
		WasCanceled = !resolveNotYetDownloaded(dlg,theDocument,theLayer,theList,theDownloader);
	delete dlg;
	if (WasCanceled)
	{
		theDocument->remove(theLayer);
		delete conflictLayer;
		delete theList;
		return false;
	}
	else
	{
		theDocument->history().add(theList);
		if (conflictLayer->size())
			theDocument->add(conflictLayer);
		else
			delete conflictLayer;
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
	return importOSM(aParent, File, theDocument, theLayer, theDownloader);
}



