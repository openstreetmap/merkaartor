#include "Command.h"
#include "Document.h"
#include "Layer.h"
#include "Feature.h"
#include "DocumentCommands.h"
#include "WayCommands.h"
#include "TrackSegmentCommands.h"
#include "RelationCommands.h"
#include "NodeCommands.h"
#include "FeatureCommands.h"

#include <QApplication>
#include <QAction>
#include <QListWidget>
#include <QUuid>
#include <QProgressDialog>

#include <algorithm>
#include <utility>
#include <QList>

Command::Command(Feature* aF)
    : mainFeature(aF), commandDirtyLevel(0), oldCreated(""), isUndone(false)
{
    description = QApplication::translate("Command", "No description");
}

Command::~Command()
{
}

void Command::setId(const QString& id)
{
    Id = id;
}

const QString& Command::id() const
{
    if (Id == "")
        Id = QUuid::createUuid().toString();
    return Id;
}

QString Command::getDescription()
{
    return description;
}

void Command::setDescription(QString desc)
{
    description = desc;
}

Feature* Command::getFeature()
{
    return mainFeature;
}

void Command::setFeature(Feature* feat)
{
    mainFeature = feat;
}

bool Command::buildUndoList(QListWidget* theListWidget)
{
    QListWidgetItem* it = new QListWidgetItem(getDescription(), theListWidget);
    if (getFeature())
        it->setData(Qt::UserRole, QVariant::fromValue(getFeature()->id()));

    return true;
}

int Command::incDirtyLevel(Layer* aLayer, Feature* F)
{
    F->incDirtyLevel();
    aLayer->incDirtyLevel();
    return ++commandDirtyLevel;
}

int Command::decDirtyLevel(Layer* aLayer, Feature* F)
{
    F->decDirtyLevel();
    aLayer->decDirtyLevel();
    return commandDirtyLevel;
}

int Command::getDirtyLevel()
{
    return commandDirtyLevel;
}

void Command::undo()
{
    if (mainFeature) {
        isUndone = true;
        mainFeature->notifyChanges();
    }
}

void Command::redo()
{
    if (mainFeature) {
        isUndone = false;
        mainFeature->notifyChanges();
    }
}

bool Command::toXML(QDomElement& xParent) const
{
    bool OK = true;

    if (mainFeature) {
        QDomElement e = xParent.ownerDocument().createElement("Command");
        xParent.appendChild(e);

        e.setAttribute("xml:id", id());
        e.setAttribute("feature", mainFeature->xmlId());
        e.setAttribute("oldCreated", oldCreated);
        if (isUndone)
            e.setAttribute("undone", "true");
    }

    return OK;
}

void Command::fromXML(Document* d, const QDomElement& e, Command* C)
{
    QDomElement c = e.firstChildElement();
    while(!c.isNull()) {
        if (c.tagName() == "Command") {
            Feature* F;
            if (!(F = d->getFeature(IFeature::FId(IFeature::Uninitialized, c.attribute("feature").toLongLong()))))
                return;

            C->setId(c.attribute("xml:id"));
            if (c.hasAttribute("oldCreated"))
                C->oldCreated = c.attribute("oldCreated");
            if (c.hasAttribute("undone"))
                C->isUndone = (c.attribute("undone") == "true" ? true : false);
            C->mainFeature = F;
        }
        c = c.nextSiblingElement();
    }
}

// COMMANDLIST

CommandList::CommandList()
: Command(0), Size(0), isReversed(false)
{
}

CommandList::CommandList(QString aDesc, Feature* aFeat)
: Command(0), Size(0), isReversed(false)
{
    description = aDesc;
    mainFeature = aFeat;
}

CommandList::~CommandList(void)
{
    for (int i=0; i<Subs.size(); ++i)
        delete Subs[i];
}

void CommandList::setReversed(bool val)
{
    isReversed = val;
}

void CommandList::add(Command* aCommand)
{
    Subs.push_back(aCommand);
    Size++;
}

bool CommandList::empty() const
{
    return Size == 0;
}

int CommandList::size()
{
    return Size;
}

void CommandList::redo()
{
    if (!isReversed) {
        for (int i=0; i<Size; ++i)
            Subs[i]->redo();
    } else {
        for (int i=Size; i; --i)
            Subs[i-1]->undo();
    }
}

void CommandList::undo()
{
    if (!isReversed) {
        for (int i=Size; i; --i)
            Subs[i-1]->undo();
    } else {
        for (int i=0; i<Size; ++i)
            Subs[i]->redo();
    }
}

bool CommandList::buildDirtyList(DirtyList& theList)
{
    for (int i=0; i<Size;)
    {
        if (Subs[i]->buildDirtyList(theList))
        {
            //delete Subs[i];
            //Subs.erase(Subs.begin()+i);
            std::rotate(Subs.begin()+i,Subs.begin()+i+1,Subs.end());
            --Size;
        }
        else
            ++i;
    }

    return Size == 0;
}

bool CommandList::toXML(QDomElement& xParent) const
{
    bool OK = true;

    QDomElement e = xParent.ownerDocument().createElement("CommandList");
    xParent.appendChild(e);

    e.setAttribute("xml:id", id());
    if (isReversed)
        e.setAttribute("reversed", "true");
    e.setAttribute("description", description);
    if (mainFeature) {
        e.setAttribute("feature", mainFeature->id().numId);
        e.setAttribute("featureclass", mainFeature->getClass());
    }

    for (int i=0; i<Size; ++i) {
        OK = Subs[i]->toXML(e);
    }

    return OK;
}

CommandList* CommandList::fromXML(Document* d, const QDomElement& e)
{
    CommandList* l = new CommandList();
    l->setId(e.attribute("xml:id"));
    l->isReversed = (e.attribute("reversed") == "true");
    if (e.hasAttribute("description"))
        l->description = e.attribute("description");
    if (e.hasAttribute("feature")) {
        if (e.attribute("featureclass") == "Node") {
            l->mainFeature = (Feature*) Feature::getTrackPointOrCreatePlaceHolder(d, (Layer *) d->getDirtyOrOriginLayer(), IFeature::FId(IFeature::Point, e.attribute("feature").toLongLong()));
        } else
        if (e.attribute("featureclass") == "Way") {
            l->mainFeature = (Feature*) Feature::getWayOrCreatePlaceHolder(d, (Layer *) d->getDirtyOrOriginLayer(), IFeature::FId(IFeature::LineString, e.attribute("feature").toLongLong()));
        } else
        if (e.attribute("featureclass") == "Relation") {
            l->mainFeature = (Feature*) Feature::getRelationOrCreatePlaceHolder(d, (Layer *) d->getDirtyOrOriginLayer(), IFeature::FId(IFeature::OsmRelation, e.attribute("feature").toLongLong()));
        }
    }

    QDomElement c = e.firstChildElement();
    while(!c.isNull()) {
        if (c.tagName() == "AddFeatureCommand") {
            AddFeatureCommand* C = AddFeatureCommand::fromXML(d, c);
            if (C)
                l->add(C);
        } else
        if (c.tagName() == "MoveTrackPointCommand") {
            MoveNodeCommand* C = MoveNodeCommand::fromXML(d, c);
            if (C)
                l->add(C);
        } else
        if (c.tagName() == "RelationAddFeatureCommand") {
            RelationAddFeatureCommand* C = RelationAddFeatureCommand::fromXML(d, c);
            if (C)
                l->add(C);
        } else
        if (c.tagName() == "RelationRemoveFeatureCommand") {
            RelationRemoveFeatureCommand* C = RelationRemoveFeatureCommand::fromXML(d, c);
            if (C)
                l->add(C);
        } else
        if (c.tagName() == "RemoveFeatureCommand") {
            RemoveFeatureCommand* C = RemoveFeatureCommand::fromXML(d, c);
            if (C)
                l->add(C);
        } else
        if (c.tagName() == "RoadAddTrackPointCommand") {
            WayAddNodeCommand* C = WayAddNodeCommand::fromXML(d, c);
            if (C)
                l->add(C);
        } else
        if (c.tagName() == "RoadRemoveTrackPointCommand") {
            WayRemoveNodeCommand* C = WayRemoveNodeCommand::fromXML(d, c);
            if (C)
                l->add(C);
        } else
        if (c.tagName() == "TrackSegmentAddTrackPointCommand") {
            TrackSegmentAddNodeCommand* C = TrackSegmentAddNodeCommand::fromXML(d, c);
            if (C)
                l->add(C);
        } else
        if (c.tagName() == "TrackSegmentRemoveTrackPointCommand") {
            TrackSegmentRemoveNodeCommand* C = TrackSegmentRemoveNodeCommand::fromXML(d, c);
            if (C)
                l->add(C);
        } else
        if (c.tagName() == "ClearTagCommand") {
            ClearTagCommand* C = ClearTagCommand::fromXML(d, c);
            if (C)
                l->add(C);
        } else
        if (c.tagName() == "ClearTagsCommand") {
            ClearTagsCommand* C = ClearTagsCommand::fromXML(d, c);
            if (C)
                l->add(C);
        } else
        if (c.tagName() == "SetTagCommand") {
            SetTagCommand* C = SetTagCommand::fromXML(d, c);
            if (C)
                l->add(C);
        } else
        if (c.tagName() == "CommandList") {
            l->add(CommandList::fromXML(d, c));
        }

        c = c.nextSiblingElement();
    }

    if (l->Size == 0) {
        qDebug() << "!! Corrupted (empty) command list. Deleting...";
        delete l;
        return NULL;
    }

    return l;
}


// COMMANDHISTORY

CommandHistory::CommandHistory()
: Index(0), Size(0), UndoAction(0), RedoAction(0), UploadAction(0)
{
}

CommandHistory::~CommandHistory()
{
    cleanup();
}

void CommandHistory::cleanup()
{
    //FIXME Is there a point to this?
    //for (int i=Index; i<Subs.size(); ++i)
    //	Subs[i]->redo();
    for (int i=0; i<Subs.size(); ++i)
        delete Subs[i];
    Subs.clear();
    Index = 0;
    Size = 0;
}

void CommandHistory::undo()
{
    if (Index)
    {
        Subs[--Index]->undo();
        updateActions();
    }
}

void CommandHistory::redo()
{
    if (Index < Size)
    {
        Subs[Index++]->redo();
        updateActions();
    }
}

void CommandHistory::add(Command* aCommand)
{
    //for (int i=Index; i<Subs.size(); ++i)
    //	delete Subs[i];
    //Subs.erase(Subs.begin()+Index,Subs.end());
    //Subs.push_back(aCommand);
    //Index = Subs.size();
    Subs.insert(Subs.begin()+Index, aCommand);
    Index++;
    Size = Index;
    updateActions();
}

void CommandHistory::setActions(QAction* anUndo, QAction* aRedo, QAction* anUploadAction)
{
    UndoAction = anUndo;
    RedoAction = aRedo;
    UploadAction = anUploadAction;
    updateActions();
}

void CommandHistory::updateActions()
{
    if (UndoAction)
        UndoAction->setEnabled(Index>0);
    if (RedoAction)
        RedoAction->setEnabled(Index<Size);
    if (UploadAction && !M_PREFS->getOfflineMode())
        UploadAction->setEnabled(Subs.size());
}

int CommandHistory::index() const
{
    return Index;
}

int CommandHistory::buildDirtyList(DirtyList& theList)
{
    for (int i=0; i<Subs.size();)
        if (Subs[i]->buildDirtyList(theList))
        {
            //delete Subs[i];
            //Subs.erase(Subs.begin()+i);
            std::rotate(Subs.begin()+i,Subs.begin()+i+1,Subs.end());
            --Index;
            --Size;
            if (!Size)
                cleanup();
        }
        else
            ++i;

    return Index;
}

int CommandHistory::buildUndoList(QListWidget* theList)
{
    for (int i=0; i<Index; ++i)
        if (!(i < Subs.size()))
            qDebug() << "!!! Error: Undo Index > list size";
        else
            Subs[i]->buildUndoList(theList);

    return Index;
}

bool CommandHistory::toXML(QDomElement& xParent, QProgressDialog * /*progress*/) const
{
    bool OK = true;

    QDomElement e = xParent.namedItem("CommandHistory").toElement();
    if (!e.isNull()) {
        xParent.removeChild(e);
    }

    e = xParent.ownerDocument().createElement("CommandHistory");
    xParent.appendChild(e);

    e.setAttribute("index", QString::number(Index));

    for (int i=0; i<Size; ++i) {
        OK = Subs[i]->toXML(e);
    }

    return OK;
}

CommandHistory* CommandHistory::fromXML(Document* d, QDomElement& e, QProgressDialog * progress)
{
    bool OK = true;
    CommandHistory* h = new CommandHistory();

    QDomElement c = e.firstChildElement();
    while(!c.isNull()) {
        if (c.tagName() == "CommandList") {
            CommandList* l = CommandList::fromXML(d, c);
            if (l)
                h->add(l);
            else
                OK = false;
        } else
        if (c.tagName() == "SetTagCommand") {
            SetTagCommand* C = SetTagCommand::fromXML(d, c);
            if (C)
                h->add(C);
            else
                OK = false;
        } else
        if (c.tagName() == "ClearTagCommand") {
            ClearTagCommand* C = ClearTagCommand::fromXML(d, c);
            if (C)
                h->add(C);
            else
                OK = false;
        } else
        if (c.tagName() == "MoveTrackPointCommand") {
            MoveNodeCommand* C = MoveNodeCommand::fromXML(d, c);
            if (C)
                h->add(C);
            else
                OK = false;
        } else {
            qDebug() << "!!! Error: Undefined tag in CommandHistory::fromXML: " << c.tagName();
            OK = false;
        }

        if (progress && progress->wasCanceled())
            break;

        c = c.nextSiblingElement();
    }
    h->Index = e.attribute("index").toUInt();

    if (!OK) {
        qDebug() << "!! File history is corrupted. Reseting...";
        qDebug() << "-- Size: " << h->Size;
        qDebug() << "-- Index: " << h->Index;
        delete h;
        h = new CommandHistory();
    }

    return h;
}


