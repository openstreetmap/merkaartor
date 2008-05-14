#include "Sync/DirtyList.h"
#include "Command/Command.h"
#include "Map/Coord.h"
#include "Map/DownloadOSM.h"
#include "Map/ExportOSM.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "Map/Relation.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"

#include <QtCore/QEventLoop>
#include <QtGui/QDialog>
#include <QtGui/QListWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>
#include <QtNetwork/QHttp>
#include <QtNetwork/QTcpSocket>

#include <algorithm>

static QString stripToOSMId(const QString& id)
{
	int f = id.lastIndexOf("_");
	if (f>0)
		return id.right(id.length()-(f+1));
	return id;
}

static QString userName(const MapFeature* F)
{
	QString s(F->tagValue("name",""));
	if (!s.isEmpty())
		return " ("+s+")";
	return "";
}

static bool isInterestingPoint(MapDocument* theDocument, TrackPoint* Pt)
{
	// does its id look like one from osm
	if (Pt->id().left(5) == "node_")
		return true;
	// if the user has added special tags, that's fine also
	for (unsigned int i=0; i<Pt->tagSize(); ++i)
		if (Pt->tagKey(i) != "created_by")
			return true;
	// if it is part of a road, then too
	for (unsigned int j=0; j<theDocument->layerSize(); ++j)
	{
		MapLayer* theLayer = theDocument->getLayer(j);
		for (unsigned i=0; i<theLayer->size(); ++i)
		{
			Road* R = dynamic_cast<Road*>(theLayer->get(i));
			if (R)
			{
				for (unsigned int i=0; i<R->size(); ++i)
					if (R->get(i) == Pt)
						return true;
			}
		}
	}
	return false;
}

DirtyList::~DirtyList()
{
}

bool DirtyListBuild::add(MapFeature* F)
{
	Added.push_back(F);
	return false;
}

bool DirtyListBuild::update(MapFeature* F)
{
	for (unsigned int i=0; i<Updated.size(); ++i)
		if (Updated[i] == F)
		{
			UpdateCounter[i].first++;
			return false;
		}
	Updated.push_back(F);
	UpdateCounter.push_back(std::make_pair(1,0));
	return false;
}

bool DirtyListBuild::erase(MapFeature* F)
{
	Deleted.push_back(F);
	return false;
}

bool DirtyListBuild::willBeAdded(MapFeature* F) const
{
	return std::find(Added.begin(),Added.end(),F) != Added.end();
}

bool DirtyListBuild::willBeErased(MapFeature* F) const
{
	return std::find(Deleted.begin(),Deleted.end(),F) != Deleted.end();
}

bool DirtyListBuild::updateNow(MapFeature* F) const
{
	for (unsigned int i=0; i<Updated.size(); ++i)
		if (Updated[i] == F)
		{
			UpdateCounter[i].second++;
			return UpdateCounter[i].first == UpdateCounter[i].second;
		}
	return false;
}

void DirtyListBuild::resetUpdates()
{
	for (unsigned int i=0; i<UpdateCounter.size(); ++i)
		UpdateCounter[i].second = 0;
}

/* DIRTYLISTVISIT */

DirtyListVisit::DirtyListVisit(MapDocument* aDoc, const DirtyListBuild &aBuilder, bool b)
: theDocument(aDoc), Future(aBuilder), EraseFromHistory(b)
{
}

MapDocument* DirtyListVisit::document()
{
	return theDocument;
}

bool DirtyListVisit::notYetAdded(MapFeature* F)
{
	return std::find(AlreadyAdded.begin(),AlreadyAdded.end(),F) == AlreadyAdded.end();
}

bool DirtyListVisit::add(MapFeature* F)
{
	if (Future.willBeErased(F))
		return EraseFromHistory;
	for (unsigned int i=0; i<AlreadyAdded.size(); ++i)
		if (AlreadyAdded[i] == F)
			return EraseResponse[i];
	if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(F))
	{
		if (isInterestingPoint(theDocument,Pt))
		{
			bool x = addPoint(Pt);
			AlreadyAdded.push_back(F);
			EraseResponse.push_back(x);
			return x;
		}
		else
			return EraseFromHistory;
	}
	else if (Road* R = dynamic_cast<Road*>(F))
	{
		for (unsigned int i=0; i<R->size(); ++i)
			if (!hasOSMId(R->get(i)) && notYetAdded(R->get(i)))
				add(R->get(i));
		bool x = addRoad(R);
		AlreadyAdded.push_back(F);
		EraseResponse.push_back(x);
		return x;
	}
	else if (Relation* Rel = dynamic_cast<Relation*>(F))
	{
		for (unsigned int i=0; i<Rel->size(); ++i)
			if (!hasOSMId(R->get(i)) && notYetAdded(Rel->get(i)))
				add(Rel->get(i));
		bool x = addRelation(Rel);
		AlreadyAdded.push_back(F);
		EraseResponse.push_back(x);
		return x;
	}
	return EraseFromHistory;
}

bool DirtyListVisit::update(MapFeature* F)
{
	if (Future.willBeErased(F) || Future.willBeAdded(F))
		return EraseFromHistory;
	if (!Future.updateNow(F))
		return EraseFromHistory;
	if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(F))
	{
		if (isInterestingPoint(theDocument,Pt))
			return updatePoint(Pt);
		else
			return EraseFromHistory;
	}
	else if (Road* R = dynamic_cast<Road*>(F))
		return updateRoad(R);
	else if (Relation* S = dynamic_cast<Relation*>(F))
		return updateRelation(S);
	return EraseFromHistory;
}

bool DirtyListVisit::erase(MapFeature* F)
{
	if (Future.willBeAdded(F))
		return EraseFromHistory;
	if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(F))
	{
		if (isInterestingPoint(theDocument,Pt))
			return erasePoint(Pt);
		else
			return EraseFromHistory;
	}
	else if (Road* R = dynamic_cast<Road*>(F))
		return eraseRoad(R);
	else if (Relation* S = dynamic_cast<Relation*>(F))
		return eraseRelation(S);
	return EraseFromHistory;
}

/* DIRTYLISTDESCRIBER */


DirtyListDescriber::DirtyListDescriber(MapDocument* aDoc, const DirtyListBuild& aFuture)
: DirtyListVisit(aDoc, aFuture, false), Task(0)
{
}

unsigned int DirtyListDescriber::tasks() const
{
	return Task;
}

bool DirtyListDescriber::showChanges(QWidget* aParent)
{
	QDialog* dlg = new QDialog(aParent);
	Ui.setupUi(dlg);

	document()->history().buildDirtyList(*this);

	bool ok = (dlg->exec() == QDialog::Accepted);

	Task = Ui.ChangesList->count();
	delete dlg;
	return ok;
}


bool DirtyListDescriber::addRoad(Road* R)
{
	Ui.ChangesList->addItem(QApplication::translate("DirtyListExecutor","ADD road %1").arg(R->id()) + userName(R));
	return false;
}

bool DirtyListDescriber::addPoint(TrackPoint* Pt)
{
	Ui.ChangesList->addItem(QApplication::translate("DirtyListExecutor","ADD trackpoint %1").arg(Pt->id()) + userName(Pt));
	return false;
}

bool DirtyListDescriber::addRelation(Relation* R)
{
	Ui.ChangesList->addItem(QApplication::translate("DirtyListExecutor","ADD relation %1").arg(R->id()) + userName(R));
	return false;
}

bool DirtyListDescriber::updatePoint(TrackPoint* Pt)
{
	Ui.ChangesList->addItem(QApplication::translate("DirtyListExecutor","UPDATE trackpoint %1").arg(Pt->id()) + userName(Pt));
	return false;
}

bool DirtyListDescriber::updateRelation(Relation* R)
{
	Ui.ChangesList->addItem(QApplication::translate("DirtyListExecutor","UPDATE relation %1").arg(R->id()) + userName(R));
	return false;
}

bool DirtyListDescriber::updateRoad(Road* R)
{
	Ui.ChangesList->addItem(QApplication::translate("DirtyListExecutor","UPDATE road %1").arg(R->id()) + userName(R));
	return false;
}

bool DirtyListDescriber::erasePoint(TrackPoint* Pt)
{
	Ui.ChangesList->addItem(QApplication::translate("DirtyListExecutor","REMOVE trackpoint %1").arg(Pt->id()) + userName(Pt));
	return false;
}

bool DirtyListDescriber::eraseRoad(Road* R)
{
	Ui.ChangesList->addItem(QApplication::translate("DirtyListExecutor","REMOVE road %1").arg(R->id()) + userName(R));
	return false;
}

bool DirtyListDescriber::eraseRelation(Relation* R)
{
	Ui.ChangesList->addItem(QApplication::translate("DirtyListExecutor","REMOVE relation %1").arg(R->id()) + userName(R));
	return false;
}



/* DIRTYLIST */


DirtyListExecutor::DirtyListExecutor(MapDocument* aDoc, const DirtyListBuild& aFuture, const QString& aWeb, const QString& aUser, const QString& aPwd,
									 bool aUseProxy, const QString& aProxyHost, int aProxyPort, unsigned int aTasks)
: DirtyListVisit(aDoc, aFuture, true), Tasks(aTasks), Done(0), Web(aWeb), User(aUser), Pwd(aPwd),
  UseProxy(aUseProxy), ProxyHost(aProxyHost), ProxyPort(aProxyPort), theDownloader(0)
{
	theDownloader = new Downloader(Web, User, Pwd, UseProxy, ProxyHost, ProxyPort);
}

DirtyListExecutor::~DirtyListExecutor()
{
	delete theDownloader;
}


bool DirtyListExecutor::sendRequest(const QString& Method, const QString& URL, const QString& Data, QString& Rcv)
{
	if (!theDownloader->request(Method,URL,Data))
		return false;

	QByteArray Content = theDownloader->content();
	int x = theDownloader->resultCode();

	if (x==200)
	{
		Rcv = QString::fromUtf8(Content.data());
		return true;
	}
	else
	{
		QMessageBox::warning(Progress,tr("Error uploading request"),tr("There was an error uploading this request (%1)\nServer message is '%2'").arg(x).arg(theDownloader->resultText()));
	}
	return false;

}

bool DirtyListExecutor::executeChanges(QWidget* aParent)
{
	Progress = new QProgressDialog(aParent);
	Progress->setMinimumDuration(0);
	Progress->setMaximum(Tasks+2);
	Progress->show();
	if (!start())
		return false;
	document()->history().buildDirtyList(*this);
	stop();
	delete Progress;
	return true;
}

bool DirtyListExecutor::start()
{
	ChangeSetId = "";
	Progress->setValue(++Done);
	if (!MerkaartorPreferences::instance()->use06Api()) return true;
	Progress->setLabelText(tr("OPEN changeset"));
	QString DataIn(
		"<osm>"
		"<changeset>"
		"<tag k=\"created_by\" v=\"Merkaartor %1\"/>"
		"<tag k=\"comment\" v=\"-\"/>"
		"</changeset>"
		"</osm>");
	DataIn = DataIn.arg(VERSION);
	QString DataOut;
	QString URL = theDownloader->getURLToOpenChangeSet();
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);
	if (sendRequest("PUT",URL,DataIn, DataOut))
	{
		ChangeSetId = DataOut;
		return true;
	}
	return false;
}

bool DirtyListExecutor::stop()
{
	Progress->setValue(++Done);
	if (!MerkaartorPreferences::instance()->use06Api()) return true;
	Progress->setLabelText(tr("CLOSE changeset"));
	QString URL = theDownloader->getURLToCloseChangeSet(ChangeSetId);
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);
	if (sendRequest("PUT",URL,QString(), QString()))
	{
		ChangeSetId = "";
		return true;
	}

	return true;
}

bool DirtyListExecutor::addRelation(Relation *R)
{
	Progress->setValue(++Done);
	Progress->setLabelText(tr("ADD relation %1").arg(R->id()) + userName(R));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

	QString DataIn, DataOut, OldId;
	OldId = R->id();
	R->setId("0");
	DataIn = wrapOSM(exportOSM(*R), ChangeSetId);
	R->setId(OldId);
	QString URL=theDownloader->getURLToCreate("relation");
	if (sendRequest("PUT",URL,DataIn,DataOut))
	{
		// chop off extra spaces, newlines etc
		R->setId("rel_"+QString::number(DataOut.toInt()));
		R->setLastUpdated(MapFeature::OSMServer);
		R->setVersionNumber(0);
		return true;
	}
	return false;
}

bool DirtyListExecutor::addRoad(Road *R)
{
	Progress->setValue(++Done);
	Progress->setLabelText(tr("ADD road %1").arg(R->id()) + userName(R));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

	QString DataIn, DataOut, OldId;
	OldId = R->id();
	R->setId("0");
	DataIn = wrapOSM(exportOSM(*R), ChangeSetId);
	R->setId(OldId);
	QString URL=theDownloader->getURLToCreate("way");
	if (sendRequest("PUT",URL,DataIn,DataOut))
	{
		// chop off extra spaces, newlines etc
		R->setId("way_"+QString::number(DataOut.toInt()));
		R->setLastUpdated(MapFeature::OSMServer);
		R->setVersionNumber(0);
		return true;
	}
	return false;
}


bool DirtyListExecutor::addPoint(TrackPoint* Pt)
{
	Progress->setValue(++Done);
	Progress->setLabelText(tr("ADD trackpoint %1").arg(Pt->id()) + userName(Pt));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

	QString DataIn, DataOut, OldId;
	OldId = Pt->id();
	Pt->setId("0");
	DataIn = wrapOSM(exportOSM(*Pt), ChangeSetId);
	Pt->setId(OldId);
	QString URL=theDownloader->getURLToCreate("node");
	if (sendRequest("PUT",URL,DataIn,DataOut))
	{
		// chop off extra spaces, newlines etc
		Pt->setId("node_"+QString::number(DataOut.toInt()));
		Pt->setLastUpdated(MapFeature::OSMServer);
		Pt->setVersionNumber(0);
		return true;
	}
	return false;
}



bool DirtyListExecutor::updateRelation(Relation* R)
{
	Progress->setValue(++Done);
	Progress->setLabelText(tr("UPDATE relation %1").arg(R->id()) + userName(R));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);
	QString URL = theDownloader->getURLToUpdate("relation",stripToOSMId(R->id()));
	QString DataIn, DataOut;
	DataIn = wrapOSM(exportOSM(*R), ChangeSetId);
	if (sendRequest("PUT",URL,DataIn,DataOut))
	{
		R->setLastUpdated(MapFeature::OSMServer);
		return true;
	}
	return true;
}


bool DirtyListExecutor::updateRoad(Road* R)
{
	Progress->setValue(++Done);
	Progress->setLabelText(tr("UPDATE road %1").arg(R->id()) + userName(R));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);
//	QString URL("/api/0.3/way/%1");
//	URL = URL.arg(stripToOSMId(R->id()));
	QString URL = theDownloader->getURLToUpdate("way",stripToOSMId(R->id()));
	QString DataIn, DataOut;
	DataIn = wrapOSM(exportOSM(*R), ChangeSetId);
	if (sendRequest("PUT",URL,DataIn,DataOut))
	{
		R->setLastUpdated(MapFeature::OSMServer);
		return true;
	}
	return true;
}

bool DirtyListExecutor::updatePoint(TrackPoint* Pt)
{
	Progress->setValue(++Done);
	Progress->setLabelText(tr("UPDATE trackpoint %1").arg(Pt->id()) + userName(Pt));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);
//	QString URL("/api/0.3/node/%1");
//	URL = URL.arg(stripToOSMId(Pt->id()));
	QString URL = theDownloader->getURLToUpdate("node",stripToOSMId(Pt->id()));
	QString DataIn, DataOut;
	DataIn = wrapOSM(exportOSM(*Pt), ChangeSetId);
	if (sendRequest("PUT",URL,DataIn,DataOut))
	{
		Pt->setLastUpdated(MapFeature::OSMServer);
		return true;
	}
	return false;
}

bool DirtyListExecutor::erasePoint(TrackPoint *Pt)
{
	Progress->setValue(++Done);
	Progress->setLabelText(tr("REMOVE trackpoint %1").arg(Pt->id()) + userName(Pt));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);
//	QString URL("/api/0.3/node/%1");
//	URL = URL.arg(stripToOSMId(Pt->id()));
	QString URL = theDownloader->getURLToDelete("node",stripToOSMId(Pt->id()));
	QString DataIn, DataOut;
	if (sendRequest("DELETE",URL,DataIn,DataOut))
	{
		Pt->setLastUpdated(MapFeature::OSMServer);
		return true;
	}
	return false;
}

bool DirtyListExecutor::eraseRoad(Road *R)
{
	Progress->setValue(++Done);
	Progress->setLabelText(tr("REMOVE road %1").arg(R->id()) + userName(R));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);
//	QString URL("/api/0.3/way/%1");
//	URL = URL.arg(stripToOSMId(R->id()));
	QString URL = theDownloader->getURLToDelete("way",stripToOSMId(R->id()));
	QString DataIn, DataOut;
	if (sendRequest("DELETE",URL,DataIn,DataOut))
	{
		R->setLastUpdated(MapFeature::OSMServer);
		return true;
	}
	return false;
}

bool DirtyListExecutor::eraseRelation(Relation *R)
{
	Progress->setValue(++Done);
	Progress->setLabelText(tr("REMOVE relation %1").arg(R->id()) + userName(R));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);
	QString URL = theDownloader->getURLToDelete("relation",stripToOSMId(R->id()));
	QString DataIn, DataOut;
	if (sendRequest("DELETE",URL,DataIn,DataOut))
	{
		R->setLastUpdated(MapFeature::OSMServer);
		return true;
	}
	return false;
}
