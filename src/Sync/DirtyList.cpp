#include "DirtyList.h"
#include "Command.h"
#include "Maps/Coord.h"
#include "DownloadOSM.h"
#include "ExportOSM.h"
#include "Document.h"
#include "Layer.h"
#include "Features.h"

#include <QtCore/QEventLoop>
#include <QtGui/QDialog>
#include <QtGui/QListWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>
#include <QtNetwork/QHttp>
#include <QtNetwork/QTcpSocket>
#include <QInputDialog>

#include <algorithm>

int glbAdded, glbUpdated, glbDeleted;
QString glbChangeSetComment;

static QString stripToOSMId(const QString& id)
{
	int f = id.lastIndexOf("_");
	if (f>0)
		return id.right(id.length()-(f+1));
	return id;
}

static QString userName(const Feature* F)
{
	QString s(F->tagValue("name",""));
	if (!s.isEmpty())
		return " ("+s+")";
	return "";
}

DirtyList::~DirtyList()
{
}

bool DirtyListBuild::add(Feature* F)
{
	if (!F->isDirty()) return false;
	//if (F->hasOSMId()) return false;

	Added.push_back(F);
	return false;
}

bool DirtyListBuild::update(Feature* F)
{
	if (!F->isDirty()) return false;

	for (int i=0; i<Updated.size(); ++i)
		if (Updated[i] == F)
		{
			UpdateCounter[i].first++;
			return false;
		}
	Updated.push_back(F);
	UpdateCounter.push_back(qMakePair((int) 1, (int)0));
	return false;
}

bool DirtyListBuild::erase(Feature* F)
{
	if (!F->isDirty()) return false;

	Deleted.push_back(F);
	return false;
}

bool DirtyListBuild::willBeAdded(Feature* F) const
{
	return std::find(Added.begin(),Added.end(),F) != Added.end();
}

bool DirtyListBuild::willBeErased(Feature* F) const
{
	return std::find(Deleted.begin(),Deleted.end(),F) != Deleted.end();
}

bool DirtyListBuild::updateNow(Feature* F) const
{
	for (int i=0; i<Updated.size(); ++i)
		if (Updated[i] == F)
		{
			UpdateCounter[i].second++;
			return UpdateCounter[i].first == UpdateCounter[i].second;
		}
	return false;
}

void DirtyListBuild::resetUpdates()
{
	for (int i=0; i<UpdateCounter.size(); ++i)
		UpdateCounter[i].second = 0;
}

/* DIRTYLISTVISIT */

DirtyListVisit::DirtyListVisit(Document* aDoc, const DirtyListBuild &aBuilder, bool b)
: theDocument(aDoc), Future(aBuilder), EraseFromHistory(b)
{
}

bool DirtyListVisit::runVisit()
{
	DeletePass = false;
	document()->history().buildDirtyList(*this);
	DeletePass = true;
	for (int i=0; i<RelationsToDelete.uniqueKeys().size(); i++) {
		if (!RelationsToDelete.uniqueKeys()[i]->hasOSMId())
			continue;
		RelationsToDelete[RelationsToDelete.uniqueKeys()[i]] = eraseRelation(RelationsToDelete.uniqueKeys()[i]);
	}
	for (int i=0; i<RoadsToDelete.uniqueKeys().size(); i++) {
		if (!RoadsToDelete.uniqueKeys()[i]->hasOSMId())
			continue;
		RoadsToDelete[RoadsToDelete.uniqueKeys()[i]] = eraseRoad(RoadsToDelete.uniqueKeys()[i]);
	}
	for (int i=0; i<TrackPointsToDelete.uniqueKeys().size(); i++) {
		if (!TrackPointsToDelete.uniqueKeys()[i]->hasOSMId())
			continue;
		TrackPointsToDelete[TrackPointsToDelete.uniqueKeys()[i]] = erasePoint(TrackPointsToDelete.uniqueKeys()[i]);
	}
	return document()->history().buildDirtyList(*this);
}

Document* DirtyListVisit::document()
{
	return theDocument;
}

bool DirtyListVisit::notYetAdded(Feature* F)
{
	return std::find(AlreadyAdded.begin(),AlreadyAdded.end(),F) == AlreadyAdded.end();
}

bool DirtyListVisit::add(Feature* F)
{
	if (DeletePass) return false;
	if (F->isDeleted()) return false;
	// TODO Needed to add children of updated imported features. Sure there is no advert cases? 
	//if (!F->isDirty()) return false;

	// Allow "Force Upload" of OSM objects
	//if (F->hasOSMId()) return false;

	if (Future.willBeErased(F))
		return EraseFromHistory;
	for (int i=0; i<AlreadyAdded.size(); ++i)
		if (AlreadyAdded[i] == F)
			return EraseResponse[i];

	bool x;
    if (Node* Pt = CAST_NODE(F))
	{
		if (Pt->isInteresting())
		{
			if (F->hasOSMId())
				x = updatePoint(Pt);
			else
				x = addPoint(Pt);
			AlreadyAdded.push_back(F);
			EraseResponse.push_back(x);
			return x;
		}
		else
			return EraseFromHistory;
	}
	else if (Way* R = dynamic_cast<Way*>(F))
	{
		for (int i=0; i<R->size(); ++i)
            if (!R->getNode(i)->isVirtual())
                if (!(R->get(i)->hasOSMId()) && notYetAdded(R->get(i)))
                    add(R->get(i));
		if (F->hasOSMId())
			x = updateRoad(R);
		else
			x = addRoad(R);
		AlreadyAdded.push_back(F);
		EraseResponse.push_back(x);
		return x;
	}
	else if (Relation* Rel = dynamic_cast<Relation*>(F))
	{
		for (int i=0; i<Rel->size(); ++i)
			if (!(Rel->get(i)->hasOSMId()) && notYetAdded(Rel->get(i)))
				add(Rel->get(i));
		if (F->hasOSMId())
			x = updateRelation(Rel);
		else
			x = addRelation(Rel);
		AlreadyAdded.push_back(F);
		EraseResponse.push_back(x);
		return x;
	}
	return EraseFromHistory;
}

bool DirtyListVisit::update(Feature* F)
{
	if (DeletePass) return false;
	if (!F->isDirty()) return false;
	if (F->isDeleted()) return false;

	if (Future.willBeErased(F) || Future.willBeAdded(F))
		return EraseFromHistory;
	if (!Future.updateNow(F))
		return EraseFromHistory;
	if (Node* Pt = dynamic_cast<Node*>(F))
	{
		if (Pt->isInteresting()) {
			if (!(Pt->hasOSMId()) && notYetAdded(Pt)) 
				return addPoint(Pt);
			else
				return updatePoint(Pt);
		} else
			return EraseFromHistory;
	}
	else if (Way* R = dynamic_cast<Way*>(F)) {
		for (int i=0; i<R->size(); ++i)
			if (!(R->get(i)->hasOSMId()) && notYetAdded(R->get(i)))
				add(R->get(i));
		return updateRoad(R);
	} else if (Relation* Rel = dynamic_cast<Relation*>(F)) {
		for (int i=0; i<Rel->size(); ++i)
			if (!(Rel->get(i)->hasOSMId()) && notYetAdded(Rel->get(i)))
				add(Rel->get(i));
		return updateRelation(Rel);
	}
	return EraseFromHistory;
}

bool DirtyListVisit::erase(Feature* F)
{
	if (!F->isDirty()) return false;

	if (DeletePass) {
		if (Node* Pt = dynamic_cast<Node*>(F))
			return TrackPointsToDelete[Pt];
		else if (Way* R = dynamic_cast<Way*>(F)) 
			return RoadsToDelete[R];
		else if (Relation* S = dynamic_cast<Relation*>(F)) 
			return RelationsToDelete[S];
	}
	if (Future.willBeAdded(F))
		return EraseFromHistory;
	if (Node* Pt = dynamic_cast<Node*>(F))
	{
		if (Pt->isInteresting()) 
			TrackPointsToDelete[Pt] = false;
	}
	else if (Way* R = dynamic_cast<Way*>(F)) 
		RoadsToDelete[R] = false;
	else if (Relation* S = dynamic_cast<Relation*>(F)) 
		RelationsToDelete[S] = false;

	return false;
}

bool DirtyListVisit::noop(Feature* F)
{
	if (!F->isDirty()) return false;

	if (!F->isDeleted() && !F->isUploadable())
		return false;

	return EraseFromHistory;
}

/* DIRTYLISTDESCRIBER */


DirtyListDescriber::DirtyListDescriber(Document* aDoc, const DirtyListBuild& aFuture)
: DirtyListVisit(aDoc, aFuture, false), Task(0)
{
	glbAdded = glbUpdated = glbDeleted = 0;
}

int DirtyListDescriber::tasks() const
{
	return Task;
}

bool DirtyListDescriber::showChanges(QWidget* aParent)
{
	QDialog* dlg = new QDialog(aParent);
	Ui.setupUi(dlg);
	dlg->setWindowFlags(dlg->windowFlags() & ~Qt::WindowContextHelpButtonHint);
	theListWidget = Ui.ChangesList;

	runVisit();

	if (M_PREFS->apiVersionNum() < 0.6) {
		Ui.lblChangesetComment->setVisible(false);
		Ui.edChangesetComment->setVisible(false);
	} else {
		CoordBox bbox = theDocument->getDirtyOrOriginLayer()->boundingBox();
		QString bboxComment = QString("BBOX:%1,%2,%3,%4")
			.arg(QString::number(intToAng(bbox.bottomLeft().lon()), 'f', 2))
			.arg(QString::number(intToAng(bbox.bottomLeft().lat()), 'f', 2))
			.arg(QString::number(intToAng(bbox.topRight().lon()), 'f', 2))
			.arg(QString::number(intToAng(bbox.topRight().lat()), 'f', 2));

		QString statComment = QString("ADD:%1 UPD:%2 DEL:%3").arg(glbAdded).arg(glbUpdated).arg(glbDeleted);

		glbChangeSetComment = bboxComment + " " + statComment;
		Ui.edChangesetComment->setText(glbChangeSetComment);
	}

	bool ok = (dlg->exec() == QDialog::Accepted);

	if (M_PREFS->apiVersionNum() > 0.5) {
		if (!Ui.edChangesetComment->text().isEmpty())
			glbChangeSetComment = Ui.edChangesetComment->text();
		else
			glbChangeSetComment = "-";
	}
	
	Task = Ui.ChangesList->count();
	SAFE_DELETE(dlg);
	return ok;
}


bool DirtyListDescriber::addRoad(Way* R)
{
	QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","ADD road %1").arg(R->id()) + userName(R), theListWidget);
	it->setData(Qt::UserRole, R->id());
	++glbAdded;
	return false;
}

bool DirtyListDescriber::addPoint(Node* Pt)
{
	QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","ADD trackpoint %1").arg(Pt->id()) + userName(Pt), theListWidget);
	it->setData(Qt::UserRole, Pt->id());
	++glbAdded;
	return false;
}

bool DirtyListDescriber::addRelation(Relation* R)
{
	QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","ADD relation %1").arg(R->id()) + userName(R), theListWidget);
	it->setData(Qt::UserRole, R->id());
	++glbAdded;
	return false;
}

bool DirtyListDescriber::updatePoint(Node* Pt)
{
	QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","UPDATE trackpoint %1").arg(Pt->id()) + userName(Pt), theListWidget);
	it->setData(Qt::UserRole, Pt->id());
	++glbUpdated;
	return false;
}

bool DirtyListDescriber::updateRelation(Relation* R)
{
	QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","UPDATE relation %1").arg(R->id()) + userName(R), theListWidget);
	it->setData(Qt::UserRole, R->id());
	++glbUpdated;
	return false;
}

bool DirtyListDescriber::updateRoad(Way* R)
{
	QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","UPDATE road %1").arg(R->id()) + userName(R), theListWidget);
	it->setData(Qt::UserRole, R->id());
	++glbUpdated;
	return false;
}

bool DirtyListDescriber::erasePoint(Node* Pt)
{
	QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","REMOVE trackpoint %1").arg(Pt->id()) + userName(Pt), theListWidget);
	it->setData(Qt::UserRole, Pt->id());
	++glbDeleted;
	return false;
}

bool DirtyListDescriber::eraseRoad(Way* R)
{
	QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","REMOVE road %1").arg(R->id()) + userName(R), theListWidget);
	it->setData(Qt::UserRole, R->id());
	++glbDeleted;
	return false;
}

bool DirtyListDescriber::eraseRelation(Relation* R)
{
	QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","REMOVE relation %1").arg(R->id()) + userName(R), theListWidget);
	it->setData(Qt::UserRole, R->id());
	++glbDeleted;
	return false;
}

/* DIRTYLIST */


DirtyListExecutor::DirtyListExecutor(Document* aDoc, const DirtyListBuild& aFuture, const QString& aWeb, const QString& aUser, const QString& aPwd, int aTasks)
: DirtyListVisit(aDoc, aFuture, false), Tasks(aTasks), Done(0), Web(aWeb), User(aUser), Pwd(aPwd), theDownloader(0)
{
	theDownloader = new Downloader(Web, User, Pwd);
}

DirtyListExecutor::~DirtyListExecutor()
{
	delete theDownloader;
}


bool DirtyListExecutor::sendRequest(const QString& Method, const QString& URL, const QString& Data, QString& Rcv)
{
	if (inError())
		return false;

	QMessageBox::StandardButton theChoice = QMessageBox::Retry;
	while (theChoice == QMessageBox::Retry) {
		if (!theDownloader->request(Method,URL,Data))
		{
			qDebug() << QString("Upload error: request (%1); Server message is '%2'").arg(theDownloader->resultCode()).arg(theDownloader->resultText());
			if (theDownloader->resultCode() == 401) {
				QMessageBox::warning(Progress,tr("Error uploading request"),
					tr("Please check your username and password in the Preferences menu"));
				theChoice = QMessageBox::Abort;
			} else {
				QString msg = tr("There was an error uploading this request (%1)\nServer message is '%2'").arg(theDownloader->resultCode()).arg(theDownloader->resultText());
				if (!theDownloader->errorText().isEmpty())
					msg += tr("\nAPI message is '%1'").arg(theDownloader->errorText());
				theChoice = QMessageBox::warning(Progress,tr("Error uploading request"), msg, QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
				continue;
			}
		}

		QByteArray Content = theDownloader->content();
		int x = theDownloader->resultCode();

		if (x==200)
		{
			Rcv = QString::fromUtf8(Content.data());
			break;
		}
		else
		{
			qDebug() << QString("Upload error: request (%1); Server message is '%2'").arg(theDownloader->resultCode()).arg(theDownloader->resultText());
			theChoice = QMessageBox::warning(Progress,tr("Error uploading request"),
							tr("There was an error uploading this request (%1)\nServer message is '%2'").arg(x).arg(theDownloader->resultText()),
							QMessageBox::Abort | QMessageBox::Retry | QMessageBox::Ignore);
			continue;
		}
	}
	if (theChoice == QMessageBox::Abort) {
		errorAbort = true;
		return false;
	}
	return true;
}

bool DirtyListExecutor::executeChanges(QWidget* aParent)
{
	Progress = new QProgressDialog(aParent);
	Progress->setWindowTitle(tr("Uploading changes..."));
	Progress->setWindowFlags(Progress->windowFlags() & ~Qt::WindowContextHelpButtonHint);
	Progress->setWindowFlags(Progress->windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
	Progress->setWindowModality(Qt::WindowModal);
	Progress->setMinimumDuration(0);
	Progress->setMaximum(Tasks+2);
	Progress->show();

	if (start())
	{
		if (runVisit())
			stop();
	}
	SAFE_DELETE(Progress);
	return true;
}

bool DirtyListExecutor::start()
{
	ChangeSetId = "";
	Progress->setValue(++Done);
	if (!(M_PREFS->apiVersionNum() > 0.5)) return true;

	qDebug() << QString("OPEN changeset");

	Progress->setLabelText(tr("OPEN changeset"));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

	QString DataIn(
		"<osm>"
		"<changeset>"
		"<tag k=\"created_by\" v=\"Merkaartor %1\"/>"
		"<tag k=\"comment\" v=\"%2\"/>"
		"</changeset>"
		"</osm>");
	DataIn = DataIn.arg(VERSION).arg(encodeAttributes(glbChangeSetComment));
	QString DataOut;
	QString URL = theDownloader->getURLToOpenChangeSet();
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
	if (!(M_PREFS->apiVersionNum() > 0.5)) return true;

	qDebug() << QString("CLOSE changeset");

	Progress->setLabelText(tr("CLOSE changeset"));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

	QString URL = theDownloader->getURLToCloseChangeSet(ChangeSetId);
	QString DataIn, DataOut;
	if (sendRequest("PUT",URL,DataIn,DataOut))
	{
		ChangeSetId = "";
		return true;
	}

	return true;
}

bool DirtyListExecutor::addRelation(Relation *R)
{
	Progress->setValue(++Done);

	qDebug() << QString("ADD relation %1").arg(R->id());

	Progress->setLabelText(tr("ADD relation %1").arg(R->id()) + userName(R));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

	QString DataIn, DataOut, OldId;
	OldId = R->id();
	R->setId("0");
	DataIn = wrapOSM(exportOSM(*R, ChangeSetId), ChangeSetId);
	R->setId(OldId);
	QString URL=theDownloader->getURLToCreate("relation");
	if (sendRequest("PUT",URL,DataIn,DataOut))
	{
		// chop off extra spaces, newlines etc
		R->setId("rel_"+QString::number(DataOut.toInt()));
		R->setLastUpdated(Feature::OSMServer);
		R->setVersionNumber(1);
		R->layer()->remove(R);
		document()->getUploadedLayer()->add(R);
		R->setUploaded(true);
		return EraseFromHistory;
	}
	return false;
}

bool DirtyListExecutor::addRoad(Way *R)
{
	Progress->setValue(++Done);

	qDebug() << QString("ADD road %1").arg(R->id());

	Progress->setLabelText(tr("ADD road %1").arg(R->id()) + userName(R));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

	QString DataIn, DataOut, OldId;
	OldId = R->id();
	R->setId("0");
	DataIn = wrapOSM(exportOSM(*R, ChangeSetId), ChangeSetId);
	R->setId(OldId);
	QString URL=theDownloader->getURLToCreate("way");
	if (sendRequest("PUT",URL,DataIn,DataOut))
	{
		// chop off extra spaces, newlines etc
		R->setId("way_"+QString::number(DataOut.toInt()));
		R->setLastUpdated(Feature::OSMServer);
		R->setVersionNumber(1);
		R->layer()->remove(R);
		document()->getUploadedLayer()->add(R);
		R->setUploaded(true);
		return EraseFromHistory;
	}
	return false;
}


bool DirtyListExecutor::addPoint(Node* Pt)
{
	Progress->setValue(++Done);

	qDebug() << QString("ADD trackpoint %1").arg(Pt->id());

	Progress->setLabelText(tr("ADD trackpoint %1").arg(Pt->id()) + userName(Pt));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

	QString DataIn, DataOut, OldId;
	OldId = Pt->id();
	Pt->setId("0");
	DataIn = wrapOSM(exportOSM(*Pt, ChangeSetId), ChangeSetId);
	Pt->setId(OldId);
	QString URL=theDownloader->getURLToCreate("node");
	if (sendRequest("PUT",URL,DataIn,DataOut))
	{
		// chop off extra spaces, newlines etc
		Pt->setId("node_"+QString::number(DataOut.toInt()));
		Pt->setLastUpdated(Feature::OSMServer);
		Pt->setVersionNumber(1);
		Pt->layer()->remove(Pt);
		document()->getUploadedLayer()->add(Pt);
		Pt->setUploaded(true);
		return EraseFromHistory;
	}
	return false;
}



bool DirtyListExecutor::updateRelation(Relation* R)
{
	Progress->setValue(++Done);

	qDebug() << QString("UPDATE relation %1").arg(R->id());

	Progress->setLabelText(tr("UPDATE relation %1").arg(R->id()) + userName(R));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);
	QString URL = theDownloader->getURLToUpdate("relation",stripToOSMId(R->id()));
	QString DataIn, DataOut;
	DataIn = wrapOSM(exportOSM(*R, ChangeSetId), ChangeSetId);
	if (sendRequest("PUT",URL,DataIn,DataOut))
	{
		R->setLastUpdated(Feature::OSMServer);
		if (M_PREFS->apiVersionNum() > 0.5)
		{
			int NewVersion = DataOut.toInt();
			if (NewVersion <= R->versionNumber())
				NewVersion = R->versionNumber()+1;
			R->setVersionNumber(NewVersion);
		}
		R->layer()->remove(R);
		document()->getUploadedLayer()->add(R);
		R->setUploaded(true);
		return EraseFromHistory;
	}
	return true;
}


bool DirtyListExecutor::updateRoad(Way* R)
{
	Progress->setValue(++Done);

	qDebug() << QString("UPDATE road %1").arg(R->id());

	Progress->setLabelText(tr("UPDATE road %1").arg(R->id()) + userName(R));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);
	QString URL = theDownloader->getURLToUpdate("way",stripToOSMId(R->id()));
	QString DataIn, DataOut;
	DataIn = wrapOSM(exportOSM(*R, ChangeSetId), ChangeSetId);
	if (sendRequest("PUT",URL,DataIn,DataOut))
	{
		R->setLastUpdated(Feature::OSMServer);
		if (M_PREFS->apiVersionNum() > 0.5)
		{
			int NewVersion = DataOut.toInt();
			if (NewVersion <= R->versionNumber())
				NewVersion = R->versionNumber()+1;
			R->setVersionNumber(NewVersion);
		}
		R->layer()->remove(R);
		document()->getUploadedLayer()->add(R);
		R->setUploaded(true);
		return EraseFromHistory;
	}
	return true;
}

bool DirtyListExecutor::updatePoint(Node* Pt)
{
	Progress->setValue(++Done);

	qDebug() << QString("UPDATE trackpoint %1").arg(Pt->id());

	Progress->setLabelText(tr("UPDATE trackpoint %1").arg(Pt->id()) + userName(Pt));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);
//	QString URL("/api/0.3/node/%1");
//	URL = URL.arg(stripToOSMId(Pt->id()));
	QString URL = theDownloader->getURLToUpdate("node",stripToOSMId(Pt->id()));
	QString DataIn, DataOut;
	DataIn = wrapOSM(exportOSM(*Pt, ChangeSetId), ChangeSetId);
	if (sendRequest("PUT",URL,DataIn,DataOut))
	{
		Pt->setLastUpdated(Feature::OSMServer);
		if (M_PREFS->apiVersionNum() > 0.5)
		{
			int NewVersion = DataOut.toInt();
			if (NewVersion <= Pt->versionNumber())
				NewVersion = Pt->versionNumber()+1;
			Pt->setVersionNumber(NewVersion);
		}
		Pt->layer()->remove(Pt);
		document()->getUploadedLayer()->add(Pt);
		Pt->setUploaded(true);
		return EraseFromHistory;
	}
	return false;
}

bool DirtyListExecutor::erasePoint(Node *Pt)
{
	Progress->setValue(++Done);

	qDebug() << QString("REMOVE trackpoint %1").arg(Pt->id());

	Progress->setLabelText(tr("REMOVE trackpoint %1").arg(Pt->id()) + userName(Pt));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);
//	QString URL("/api/0.3/node/%1");
//	URL = URL.arg(stripToOSMId(Pt->id()));
	QString URL = theDownloader->getURLToDelete("node",stripToOSMId(Pt->id()));
	QString DataIn, DataOut;
	if (M_PREFS->apiVersionNum() > 0.5)
		DataIn = wrapOSM(exportOSM(*Pt, ChangeSetId), ChangeSetId);
	if (sendRequest("DELETE",URL,DataIn,DataOut))
	{
		Pt->setLastUpdated(Feature::OSMServer);
		Pt->layer()->remove(Pt);
		document()->getUploadedLayer()->add(Pt);
		Pt->setUploaded(true);
		return EraseFromHistory;
	}
	return false;
}

bool DirtyListExecutor::eraseRoad(Way *R)
{
	Progress->setValue(++Done);

	qDebug() << QString("REMOVE road %1").arg(R->id());

	Progress->setLabelText(tr("REMOVE road %1").arg(R->id()) + userName(R));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);
//	QString URL("/api/0.3/way/%1");
//	URL = URL.arg(stripToOSMId(R->id()));
	QString URL = theDownloader->getURLToDelete("way",stripToOSMId(R->id()));
	QString DataIn, DataOut;
	if (M_PREFS->apiVersionNum() > 0.5)
		DataIn = wrapOSM(exportOSM(*R, ChangeSetId), ChangeSetId);
	if (sendRequest("DELETE",URL,DataIn,DataOut))
	{
		R->setLastUpdated(Feature::OSMServer);
		R->layer()->remove(R);
		document()->getUploadedLayer()->add(R);
		R->setUploaded(true);
		return EraseFromHistory;
	}
	return false;
}

bool DirtyListExecutor::eraseRelation(Relation *R)
{
	Progress->setValue(++Done);

	qDebug() << QString("REMOVE relation %1").arg(R->id());

	Progress->setLabelText(tr("REMOVE relation %1").arg(R->id()) + userName(R));
	QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);
	QString URL = theDownloader->getURLToDelete("relation",stripToOSMId(R->id()));
	QString DataIn, DataOut;
	if (M_PREFS->apiVersionNum() > 0.5)
		DataIn = wrapOSM(exportOSM(*R, ChangeSetId), ChangeSetId);
	if (sendRequest("DELETE",URL,DataIn,DataOut))
	{
		R->setLastUpdated(Feature::OSMServer);
		R->layer()->remove(R);
		document()->getUploadedLayer()->add(R);
		R->setUploaded(true);
		return EraseFromHistory;
	}
	return false;
}
