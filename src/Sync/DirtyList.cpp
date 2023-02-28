#include "Global.h"

#include "DirtyList.h"
#include "Command.h"
#include "Coord.h"
#include "DownloadOSM.h"
#include "ExportOSM.h"
#include "Document.h"
#include "Layer.h"
#include "Features.h"
#include "Utils.h"

#include <QEventLoop>
#include <QDialog>
#include <QListWidget>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTcpSocket>
#include <QInputDialog>

#include <algorithm>

int glbAdded, glbUpdated, glbDeleted;
int glbNodesAdded, glbNodesUpdated, glbNodesDeleted;
int glbWaysAdded, glbWaysUpdated, glbWaysDeleted;
int glbRelationsAdded, glbRelationsUpdated, glbRelationsDeleted;

QString glbChangeSetComment;

QString stripToOSMId(const IFeature::FId& id)
{
    return QString::number(id.numId);
}

QString userName(const Feature* F)
{
    QString s(F->tagValue("name", QString()));
    if (!s.isEmpty())
        return " ("+s+")";
    return QString();
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
    for (int i=0; i<RelationsToDelete.keys().size(); i++) {
        if (!RelationsToDelete.keys()[i]->hasOSMId())
            continue;
        RelationsToDelete[RelationsToDelete.keys()[i]] = eraseRelation(RelationsToDelete.keys()[i]);
    }
    for (int i=0; i<RoadsToDelete.keys().size(); i++) {
        if (!RoadsToDelete.keys()[i]->hasOSMId())
            continue;
        RoadsToDelete[RoadsToDelete.keys()[i]] = eraseRoad(RoadsToDelete.keys()[i]);
    }
    for (int i=0; i<TrackPointsToDelete.keys().size(); i++) {
        if (!TrackPointsToDelete.keys()[i]->hasOSMId())
            continue;
        TrackPointsToDelete[TrackPointsToDelete.keys()[i]] = erasePoint(TrackPointsToDelete.keys()[i]);
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
    glbNodesAdded = glbNodesUpdated = glbNodesDeleted = 0;
    glbWaysAdded = glbWaysUpdated = glbWaysDeleted = 0;
    glbRelationsAdded = glbRelationsUpdated = glbRelationsDeleted = 0;
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

    Ui.lblNodesAdded->setText(QString::number(glbNodesAdded));
    Ui.lblNodesUpdated->setText(QString::number(glbNodesUpdated));
    Ui.lblNodesDeleted->setText(QString::number(glbNodesDeleted));
    Ui.lblWaysAdded->setText(QString::number(glbWaysAdded));
    Ui.lblWaysUpdated->setText(QString::number(glbWaysUpdated));
    Ui.lblWaysDeleted->setText(QString::number(glbWaysDeleted));
    Ui.lblRelationsAdded->setText(QString::number(glbRelationsAdded));
    Ui.lblRelationsUpdated->setText(QString::number(glbRelationsUpdated));
    Ui.lblRelationsDeleted->setText(QString::number(glbRelationsDeleted));
    Ui.lblAdded->setText(QString::number(glbAdded));
    Ui.lblUpdated->setText(QString::number(glbUpdated));
    Ui.lblDeleted->setText(QString::number(glbDeleted));

    //Ui.edChangesetComment->setText(glbChangeSetComment);
    //Ui.edChangesetComment->selectAll();

    bool ok = false;
    while (!ok) {
        if (dlg->exec() == QDialog::Accepted) {
            /* Dialog was accepted, check for non-empty comment */
            if (!Ui.edChangesetComment->text().isEmpty()) {
                glbChangeSetComment = Ui.edChangesetComment->text();
                ok = true;
            } else if (QMessageBox::question(NULL,
                        QApplication::tr("Use empty changeset comment?"),
                        QApplication::tr("The changeset comment is empty. It's considered a courtesy to your fellow mappers to provide a good comment, so everyone knows what your change does and with what intention.\n"
                            "Do you still wish to commit empty changeset comment?"), QMessageBox::Yes, QMessageBox::No )
                    == QMessageBox::Yes
               ) ok = true;
        } else break; /* Dialog was cancelled */
    }

    Task = Ui.ChangesList->count();
    SAFE_DELETE(dlg)
    return ok;
}


bool DirtyListDescriber::addRoad(Way* R)
{
    QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","ADD way %1").arg(R->id().numId) + userName(R), theListWidget);
    it->setData(Qt::UserRole, QVariant::fromValue(R->id()));
    ++glbAdded;
    ++glbWaysAdded;
    return false;
}

bool DirtyListDescriber::addPoint(Node* Pt)
{
    QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","ADD node %1").arg(Pt->id().numId) + userName(Pt), theListWidget);
    it->setData(Qt::UserRole, QVariant::fromValue(Pt->id()));
    ++glbAdded;
    ++glbNodesAdded;
    return false;
}

bool DirtyListDescriber::addRelation(Relation* R)
{
    QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","ADD relation %1").arg(R->id().numId) + userName(R), theListWidget);
    it->setData(Qt::UserRole, QVariant::fromValue(R->id()));
    ++glbAdded;
    ++glbRelationsAdded;
    return false;
}

bool DirtyListDescriber::updatePoint(Node* Pt)
{
    QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","UPDATE node %1").arg(Pt->id().numId) + userName(Pt), theListWidget);
    it->setData(Qt::UserRole, QVariant::fromValue(Pt->id()));
    ++glbUpdated;
    ++glbNodesUpdated;
    return false;
}

bool DirtyListDescriber::updateRelation(Relation* R)
{
    QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","UPDATE relation %1").arg(R->id().numId) + userName(R), theListWidget);
    it->setData(Qt::UserRole, QVariant::fromValue(R->id()));
    ++glbUpdated;
    ++glbRelationsUpdated;
    return false;
}

bool DirtyListDescriber::updateRoad(Way* R)
{
    QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","UPDATE way %1").arg(R->id().numId) + userName(R), theListWidget);
    it->setData(Qt::UserRole, QVariant::fromValue(R->id()));
    ++glbUpdated;
    ++glbWaysUpdated;
    return false;
}

bool DirtyListDescriber::erasePoint(Node* Pt)
{
    QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","REMOVE node %1").arg(Pt->id().numId) + userName(Pt), theListWidget);
    it->setData(Qt::UserRole, QVariant::fromValue(Pt->id()));
    ++glbDeleted;
    ++glbNodesDeleted;
    return false;
}

bool DirtyListDescriber::eraseRoad(Way* R)
{
    QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","REMOVE way %1").arg(R->id().numId) + userName(R), theListWidget);
    it->setData(Qt::UserRole, QVariant::fromValue(R->id()));
    ++glbDeleted;
    ++glbWaysDeleted;
    return false;
}

bool DirtyListDescriber::eraseRelation(Relation* R)
{
    QListWidgetItem* it = new QListWidgetItem(QApplication::translate("DirtyListExecutor","REMOVE relation %1").arg(R->id().numId) + userName(R), theListWidget);
    it->setData(Qt::UserRole, QVariant::fromValue(R->id()));
    ++glbDeleted;
    ++glbRelationsDeleted;
    return false;
}

