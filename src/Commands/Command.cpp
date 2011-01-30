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

bool Command::toXML(QXmlStreamWriter& stream) const
{
    bool OK = true;

    if (mainFeature) {
        stream.writeStartElement("Command");
        stream.writeAttribute("xml:id", id());
        stream.writeAttribute("feature", mainFeature->xmlId());
        stream.writeAttribute("oldCreated", oldCreated);
        if (isUndone)
            stream.writeAttribute("undone", "true");
        stream.writeAttribute("description", description);
        stream.writeEndElement();
    }

    return OK;
}

void Command::fromXML(Document* d, QXmlStreamReader& stream, Command* C)
{
    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "Command") {
            Feature* F;
            if (!(F = d->getFeature(IFeature::FId(IFeature::All, stream.attributes().value("feature").toString().toLongLong()))))
                return;

            C->setId(stream.attributes().value("xml:id").toString());
            if (stream.attributes().hasAttribute("oldCreated"))
                C->oldCreated = stream.attributes().value("oldCreated").toString();
            if (stream.attributes().hasAttribute("undone"))
                C->isUndone = (stream.attributes().value("undone") == "true" ? true : false);
            if (stream.attributes().hasAttribute("description"))
                C->description = stream.attributes().value("description").toString();
            C->mainFeature = F;
            stream.readNext();
        }
        stream.readNext();
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

bool CommandList::toXML(QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement("CommandList");
    stream.writeAttribute("xml:id", id());
    if (isReversed)
        stream.writeAttribute("reversed", "true");
    if (mainFeature) {
        stream.writeAttribute("feature",QString::number(mainFeature->id().numId));
        stream.writeAttribute("featureclass", mainFeature->getClass());
    }

    for (int i=0; i<Size; ++i) {
        OK = Subs[i]->toXML(stream);
    }
    stream.writeEndElement();

    return OK;
}

CommandList* CommandList::fromXML(Document* d, QXmlStreamReader& stream)
{
    CommandList* l = new CommandList();
    l->setId(stream.attributes().value("xml:id").toString());
    l->isReversed = (stream.attributes().value("reversed") == "true");
    if (stream.attributes().hasAttribute("feature")) {
        if (stream.attributes().value("featureclass") == "Node") {
            l->mainFeature = (Feature*) Feature::getTrackPointOrCreatePlaceHolder(d, (Layer *) d->getDirtyOrOriginLayer(), IFeature::FId(IFeature::Point, stream.attributes().value("feature").toString().toLongLong()));
        } else
        if (stream.attributes().value("featureclass") == "Way") {
            l->mainFeature = (Feature*) Feature::getWayOrCreatePlaceHolder(d, (Layer *) d->getDirtyOrOriginLayer(), IFeature::FId(IFeature::LineString, stream.attributes().value("feature").toString().toLongLong()));
        } else
        if (stream.attributes().value("featureclass") == "Relation") {
            l->mainFeature = (Feature*) Feature::getRelationOrCreatePlaceHolder(d, (Layer *) d->getDirtyOrOriginLayer(), IFeature::FId(IFeature::OsmRelation, stream.attributes().value("feature").toString().toLongLong()));
        }
    }

    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "AddFeatureCommand") {
            AddFeatureCommand* C = AddFeatureCommand::fromXML(d, stream);
            if (C)
                l->add(C);
        } else if (stream.name() == "MoveTrackPointCommand") {
            MoveNodeCommand* C = MoveNodeCommand::fromXML(d, stream);
            if (C)
                l->add(C);
        } else if (stream.name() == "RelationAddFeatureCommand") {
            RelationAddFeatureCommand* C = RelationAddFeatureCommand::fromXML(d, stream);
            if (C)
                l->add(C);
        } else if (stream.name() == "RelationRemoveFeatureCommand") {
            RelationRemoveFeatureCommand* C = RelationRemoveFeatureCommand::fromXML(d, stream);
            if (C)
                l->add(C);
        } else if (stream.name() == "RemoveFeatureCommand") {
            RemoveFeatureCommand* C = RemoveFeatureCommand::fromXML(d, stream);
            if (C)
                l->add(C);
        } else if (stream.name() == "RoadAddTrackPointCommand") {
            WayAddNodeCommand* C = WayAddNodeCommand::fromXML(d, stream);
            if (C)
                l->add(C);
        } else if (stream.name() == "RoadRemoveTrackPointCommand") {
            WayRemoveNodeCommand* C = WayRemoveNodeCommand::fromXML(d, stream);
            if (C)
                l->add(C);
        } else if (stream.name() == "TrackSegmentAddTrackPointCommand") {
            TrackSegmentAddNodeCommand* C = TrackSegmentAddNodeCommand::fromXML(d, stream);
            if (C)
                l->add(C);
        } else if (stream.name() == "TrackSegmentRemoveTrackPointCommand") {
            TrackSegmentRemoveNodeCommand* C = TrackSegmentRemoveNodeCommand::fromXML(d, stream);
            if (C)
                l->add(C);
        } else if (stream.name() == "ClearTagCommand") {
            ClearTagCommand* C = ClearTagCommand::fromXML(d, stream);
            if (C)
                l->add(C);
        } else if (stream.name() == "ClearTagsCommand") {
            ClearTagsCommand* C = ClearTagsCommand::fromXML(d, stream);
            if (C)
                l->add(C);
        } else if (stream.name() == "SetTagCommand") {
            SetTagCommand* C = SetTagCommand::fromXML(d, stream);
            if (C)
                l->add(C);
        } else if (stream.name() == "CommandList") {
            l->add(CommandList::fromXML(d, stream));
        } else if (!stream.isWhitespace()) {
                qDebug() << "CList: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
                QString el = stream.readElementText(QXmlStreamReader::IncludeChildElements);
        }
        stream.readNext();
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

int CommandHistory::size() const
{
    return Size;
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

bool CommandHistory::toXML(QXmlStreamWriter& stream, QProgressDialog * /*progress*/) const
{
    bool OK = true;

    stream.writeStartElement("CommandHistory");

    stream.writeAttribute("index", QString::number(Index));

    for (int i=0; i<Size; ++i) {
        OK = Subs[i]->toXML(stream);
    }
    stream.writeEndElement();

    return OK;
}

CommandHistory* CommandHistory::fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * progress)
{
    bool OK = true;
    CommandHistory* h = new CommandHistory();
    int index = stream.attributes().value("index").toString().toUInt();

    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "CommandList") {
            CommandList* l = CommandList::fromXML(d, stream);
            if (l)
                h->add(l);
            else
                OK = false;
        } else if (stream.name() == "AddFeatureCommand") {
            AddFeatureCommand* C = AddFeatureCommand::fromXML(d, stream);
            if (C)
                h->add(C);
            else
                OK = false;
        } else if (stream.name() == "MoveTrackPointCommand") {
            MoveNodeCommand* C = MoveNodeCommand::fromXML(d, stream);
            if (C)
                h->add(C);
            else
                OK = false;
        } else if (stream.name() == "RelationAddFeatureCommand") {
            RelationAddFeatureCommand* C = RelationAddFeatureCommand::fromXML(d, stream);
            if (C)
                h->add(C);
            else
                OK = false;
        } else if (stream.name() == "RelationRemoveFeatureCommand") {
            RelationRemoveFeatureCommand* C = RelationRemoveFeatureCommand::fromXML(d, stream);
            if (C)
                h->add(C);
            else
                OK = false;
        } else if (stream.name() == "RemoveFeatureCommand") {
            RemoveFeatureCommand* C = RemoveFeatureCommand::fromXML(d, stream);
            if (C)
                h->add(C);
            else
                OK = false;
        } else if (stream.name() == "RoadAddTrackPointCommand") {
            WayAddNodeCommand* C = WayAddNodeCommand::fromXML(d, stream);
            if (C)
                h->add(C);
            else
                OK = false;
        } else if (stream.name() == "RoadRemoveTrackPointCommand") {
            WayRemoveNodeCommand* C = WayRemoveNodeCommand::fromXML(d, stream);
            if (C)
                h->add(C);
            else
                OK = false;
        } else if (stream.name() == "TrackSegmentAddTrackPointCommand") {
            TrackSegmentAddNodeCommand* C = TrackSegmentAddNodeCommand::fromXML(d, stream);
            if (C)
                h->add(C);
            else
                OK = false;
        } else if (stream.name() == "TrackSegmentRemoveTrackPointCommand") {
            TrackSegmentRemoveNodeCommand* C = TrackSegmentRemoveNodeCommand::fromXML(d, stream);
            if (C)
                h->add(C);
            else
                OK = false;
        } else if (stream.name() == "ClearTagCommand") {
            ClearTagCommand* C = ClearTagCommand::fromXML(d, stream);
            if (C)
                h->add(C);
            else
                OK = false;
        } else if (stream.name() == "ClearTagsCommand") {
            ClearTagsCommand* C = ClearTagsCommand::fromXML(d, stream);
            if (C)
                h->add(C);
            else
                OK = false;
        } else if (stream.name() == "SetTagCommand") {
            SetTagCommand* C = SetTagCommand::fromXML(d, stream);
            if (C)
                h->add(C);
            else
                OK = false;
        } else if (!stream.isWhitespace()) {
            qDebug() << "CHist: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
            QString el = stream.readElementText(QXmlStreamReader::IncludeChildElements);
        }

        progress->setValue(stream.characterOffset());
        if (progress && progress->wasCanceled())
            break;

        stream.readNext();
    }

    if (!OK) {
        qDebug() << "!! File history is corrupted. Reseting...";
        qDebug() << "-- Size: " << h->Size;
        qDebug() << "-- Index: " << h->Index;
        delete h;
        h = new CommandHistory();
    } else
        h->Index = index;

    return h;
}


