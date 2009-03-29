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


OSMHandler::OSMHandler(MapDocument* aDoc, MapLayer* aLayer, MapLayer* aConflict)
: theDocument(aDoc), theLayer(aLayer), conflictLayer(aConflict), Current(0)
{
}

void OSMHandler::parseTag(const QXmlAttributes &atts)
{
	if (!Current) return;

	Current->setTag(atts.value("k"),atts.value("v"));
}

void parseStandardAttributes(const QXmlAttributes& atts, MapFeature* F)
{
	QString ts = atts.value("timestamp"); 
	QDateTime time = QDateTime::fromString(ts.left(19), Qt::ISODate);
	QString user = atts.value("user");
	F->setTime(time);
	F->setUser(user);
	QString version = atts.value("version");
	if (!version.isEmpty())
		F->setVersionNumber(version.toInt());
}

void OSMHandler::parseNode(const QXmlAttributes& atts)
{
	double Lat = atts.value("lat").toDouble();
	double Lon = atts.value("lon").toDouble();
	QString id = "node_"+atts.value("id");
	TrackPoint* Pt = dynamic_cast<TrackPoint*>(theDocument->getFeature(id));
	if (Pt)
	{
		if (Pt->lastUpdated() == MapFeature::User)
		{
			// conflict
			Pt->setLastUpdated(MapFeature::UserResolved);
			TrackPoint* userPt = Pt;
			Pt = new TrackPoint(Coord(angToInt(Lat),angToInt(Lon)));
			Pt->setId("conflict_"+id);
			Pt->setLastUpdated(MapFeature::OSMServerConflict);
			parseStandardAttributes(atts,Pt);
			if (Pt->time() > userPt->time()) {
				if (conflictLayer)
					conflictLayer->add(Pt);
				NewFeature = true;
			} else {
				delete Pt;
				Pt = userPt;
				NewFeature = false;
			}
		}
		else if (Pt->lastUpdated() != MapFeature::UserResolved)
		{
			Pt->layer()->remove(Pt);
			theLayer->add(Pt);
			Pt->setPosition(Coord(angToInt(Lat),angToInt(Lon)));
			NewFeature = false;
			if (Pt->lastUpdated() == MapFeature::NotYetDownloaded)
				Pt->setLastUpdated(MapFeature::OSMServer);
		}
	}
	else
	{
		Pt = new TrackPoint(Coord(angToInt(Lat),angToInt(Lon)));
		theLayer->add(Pt);
		NewFeature = true;
		Pt->setId(id);
		Pt->setLastUpdated(MapFeature::OSMServer);
	}
	if (NewFeature) {
		parseStandardAttributes(atts,Pt);
		Current = Pt;
	} else
		Current = NULL;
}

void OSMHandler::parseNd(const QXmlAttributes& atts)
{
	Road* R = dynamic_cast<Road*>(Current);
	if (!R) return;
	TrackPoint *Part = MapFeature::getTrackPointOrCreatePlaceHolder(theDocument, theLayer, atts.value("ref"));
	if (NewFeature)
		R->add(Part);
}

void OSMHandler::parseWay(const QXmlAttributes& atts)
{
	QString id = "way_"+atts.value("id");
        QString ts = atts.value("timestamp"); ts.truncate(19);
	Road* R = dynamic_cast<Road*>(theDocument->getFeature(id));
	if (R)
	{
		if (R->lastUpdated() == MapFeature::User)
		{
			R->setLastUpdated(MapFeature::UserResolved);
			NewFeature = false;
			// conflict
			Road* userRd = R;
			R = new Road();
			R->setId("conflict_"+id);
			R->setLastUpdated(MapFeature::OSMServerConflict);
			parseStandardAttributes(atts,R);
			if (R->time() > userRd->time()) {
				if (conflictLayer)
					conflictLayer->add(R);
				NewFeature = true;
			} else {
				delete R;
				R = userRd;
				NewFeature = false;
			}
		}
		else if (R->lastUpdated() != MapFeature::UserResolved)
		{
			R->layer()->remove(R);
			theLayer->add(R);
			while (R->size())
				R->remove((unsigned int)0);
			NewFeature = true;
			if (R->lastUpdated() == MapFeature::NotYetDownloaded)
				R->setLastUpdated(MapFeature::OSMServer);
		}
	}
	else
	{
		R = new Road;
		theLayer->add(R);
		NewFeature = true;
		R->setId(id);
		R->setLastUpdated(MapFeature::OSMServer);
	}
	if (NewFeature) {
		parseStandardAttributes(atts,R);
		Current = R;
	} else
		Current = NULL;
}

void OSMHandler::parseMember(const QXmlAttributes& atts)
{
	Relation* R = dynamic_cast<Relation*>(Current);
	if (!R)
		return;
	QString Type = atts.value("type");
	MapFeature* F = 0;
	if (Type == "node")
		F = MapFeature::getTrackPointOrCreatePlaceHolder(theDocument, theLayer, atts.value("ref"));
	else if (Type == "way")
		F = MapFeature::getWayOrCreatePlaceHolder(theDocument, theLayer, atts.value("ref"));
	else if (Type == "relation")
		F = MapFeature::getRelationOrCreatePlaceHolder(theDocument, theLayer, atts.value("ref"));

	if (F)
		R->add(atts.value("role"),F);
}

void OSMHandler::parseRelation(const QXmlAttributes& atts)
{
	QString id = "rel_"+atts.value("id");
        QString ts = atts.value("timestamp"); ts.truncate(19);
	Relation* R = dynamic_cast<Relation*>(theDocument->getFeature(id));
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
			R->layer()->remove(R);
			theLayer->add(R);
			while (R->size())
				R->remove((unsigned int)0);
			NewFeature = true;
			if (R->lastUpdated() == MapFeature::NotYetDownloaded)
				R->setLastUpdated(MapFeature::OSMServer);
		}
	}
	else
	{
		R = new Relation;
		NewFeature = true;
		R->setId(id);
		theLayer->add(R);
		R->setLastUpdated(MapFeature::OSMServer);
	}
	if (NewFeature) {
		parseStandardAttributes(atts,R);
		Current = R;
	} else
		Current = NULL;
}

bool OSMHandler::startElement ( const QString &, const QString & /* localName */, const QString & qName, const QXmlAttributes & atts )
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

bool OSMHandler::endElement ( const QString &, const QString & /* localName */, const QString & qName )
{
	if (qName == "node")
		Current = 0;
	return true;
}

static bool downloadToResolve(const std::vector<MapFeature*>& Resolution, QWidget* aParent, MapDocument* theDocument, MapLayer* theLayer, Downloader* theDownloader)
{
	IProgressWindow* aProgressWindow = dynamic_cast<IProgressWindow*>(aParent);
	if (!aProgressWindow)
		return false;

	QProgressDialog* dlg = aProgressWindow->getProgressDialog();
	QProgressBar* Bar = aProgressWindow->getProgressBar();
	QLabel* Lbl = aProgressWindow->getProgressLabel();

	for (unsigned int i=0; i<Resolution.size(); i++ )
	{
		QString URL = theDownloader->getURLToFetchFull(Resolution[i]);
		Lbl->setText(QApplication::translate("Downloader","Downloading unresolved %1 of %2").arg(i).arg(Resolution.size()));
		if (theDownloader->go(URL))
		{
			if (theDownloader->resultCode() == 410) {
				theLayer->remove(Resolution[i]);
				delete Resolution[i];
			}
			else
			{
				Lbl->setText(QApplication::translate("Downloader","Parsing unresolved %1 of %2").arg(i).arg(Resolution.size()));

				QByteArray ba(theDownloader->content());
				QBuffer  File(&ba);
				File.open(QIODevice::ReadOnly);

				OSMHandler theHandler(theDocument,theLayer,NULL);

				QXmlSimpleReader xmlReader;
				xmlReader.setContentHandler(&theHandler);
				QXmlInputSource source;
				QByteArray buf(File.read(10240));
				source.setData(buf);
				xmlReader.parse(&source,true);
				while (!File.atEnd())
				{
					QByteArray buf(File.read(20480));
					source.setData(buf);
					xmlReader.parseContinue();
					qApp->processEvents();
					if (dlg && dlg->wasCanceled())
						break;
				}
			}
			Resolution[i]->setLastUpdated(MapFeature::OSMServer);
		}
		else
			return false;
		Bar->setValue(i);
	}
	return true;
}

static void recurseDelete (MapFeature* F, QVector<MapFeature*>& MustDelete)
{
	for (unsigned int i=0; i<F->sizeParents(); i++) {
		recurseDelete(F->getParent(i), MustDelete);
	}
	if (!MustDelete.contains(F))
		MustDelete.push_back(F);
}

static bool resolveNotYetDownloaded(QWidget* aParent, MapDocument* theDocument, MapLayer* theLayer, Downloader* theDownloader)
{
	// resolving nodes and roads makes no sense since the OSM api guarantees that they will be all downloaded,
	//  so only resolve for relations if the ResolveRelations pref is set
	if (theDownloader && M_PREFS->getResolveRelations())
	{
		IProgressWindow* aProgressWindow = dynamic_cast<IProgressWindow*>(aParent);
		if (!aProgressWindow)
			return false;

		QProgressBar* Bar = aProgressWindow->getProgressBar();
		//QLabel* Lbl = aProgressWindow->getProgressLabel();

		std::vector<MapFeature*> MustResolve;
		MustResolve.clear();
		for (FeatureIterator it(theDocument); !it.isEnd(); ++it)
		{
			Relation* RR = CAST_RELATION(it.get());
			if (RR && RR->notEverythingDownloaded())
				MustResolve.push_back(RR);
		}
		if (MustResolve.size())
		{
			Bar->setMaximum(MustResolve.size());
			Bar->setValue(0);
			if (!downloadToResolve(MustResolve,aParent,theDocument,theLayer, theDownloader))
				return false;
		}
	}
	if (M_PREFS->getDeleteIncompleteRelations()) {
		QVector<MapFeature*> MustDelete;
		for (unsigned int i=0; i<theLayer->size(); i++) 
		{
			if (theLayer->get(i)->notEverythingDownloaded()) {
				recurseDelete(theLayer->get(i), MustDelete);
			}
		}
		for (int i=0; i<MustDelete.size(); i++) {
			MustDelete[i]->layer()->remove(MustDelete[i]);
			delete MustDelete[i];
		}
	}
	return true;
}

bool importOSM(QWidget* aParent, QIODevice& File, MapDocument* theDocument, MapLayer* theLayer, Downloader* theDownloader)
{
	QDomDocument DomDoc;
	QString ErrorStr;
	/* int ErrorLine; */
	/* int ErrorColumn; */

	IProgressWindow* aProgressWindow = dynamic_cast<IProgressWindow*>(aParent);
	if (!aProgressWindow)
		return false;

	QProgressDialog* dlg = aProgressWindow->getProgressDialog();

	QProgressBar* Bar = aProgressWindow->getProgressBar();
	Bar->setTextVisible(false);

	QLabel* Lbl = aProgressWindow->getProgressLabel();
	Lbl->setText(QApplication::translate("Downloader","Parsing XML"));

	if (dlg)
		dlg->show();

	if (theDownloader)
		theDownloader->setAnimator(dlg,Lbl,Bar,false);
	MapLayer* conflictLayer = new DrawingMapLayer(QApplication::translate("Downloader","Conflicts from %1").arg(theLayer->name()));
	theDocument->add(conflictLayer);

	OSMHandler theHandler(theDocument,theLayer,conflictLayer);

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
		if (dlg && dlg->wasCanceled())
			break;
	}

	bool WasCanceled = false;
	if (dlg)
		WasCanceled = dlg->wasCanceled();
	if (!WasCanceled)
		WasCanceled = !resolveNotYetDownloaded(aParent,theDocument,theLayer,theDownloader);
	if (WasCanceled)
	{
		theDocument->remove(conflictLayer);
		delete conflictLayer;
		return false;
	}
	else
	{
		if (!conflictLayer->size()) {
			theDocument->remove(conflictLayer);
			delete conflictLayer;
		} else {
			QMessageBox::warning(aParent,QApplication::translate("Downloader","Conflicts have been detected"), 
				QApplication::translate("Downloader",
				"This means that some of the feature you modified"
				" since your last download have since been modified by someone else on the server.\n"
				"The features have been duplicated as \"conflict_...\" on the \"Conflicts...\" layer.\n"
				"Before being able to upload your changes, you will have to manually merge the two versions"
				" and remove the one from the \"Conflicts...\" layer."
				));
		}

		// Check for empty Roads/Relations
		std::vector<MapFeature*> EmptyFeature;
		for (unsigned int i=0; i<theLayer->size(); ++i) {
			if (!theLayer->get(i)->size() && !theLayer->get(i)->notEverythingDownloaded() && !CAST_NODE(theLayer->get(i)))
					EmptyFeature.push_back(theLayer->get(i));
		}
		if (EmptyFeature.size()) {
			if (QMessageBox::warning(aParent,QApplication::translate("Downloader","Empty roads/relations detected"), 
					QApplication::translate("Downloader",
					"Empty roads/relations are probably errors.\n"
					"Do you want to mark them for deletion?"),
					QMessageBox::Ok | QMessageBox::Cancel,
					QMessageBox::Cancel
					) == QMessageBox::Ok) {
				for (unsigned int i=0; i<EmptyFeature.size(); i++ ) {
					CommandList* emptyFeatureList = new CommandList();
					emptyFeatureList->setDescription(QApplication::translate("Downloader","Remove empty feature %1").arg(EmptyFeature[i]->description()));
					emptyFeatureList->setFeature(EmptyFeature[i]);
					emptyFeatureList->add(new RemoveFeatureCommand(theDocument, EmptyFeature[i]));
					theDocument->addHistory(emptyFeatureList);
				}
			}
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



