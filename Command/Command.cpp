#include "Command/Command.h"
#include "Map/MapDocument.h"
#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Command/RelationCommands.h"
#include "Command/TrackPointCommands.h"
#include "Command/FeatureCommands.h"

#include <QtGui/QAction>
#include <QUuid>

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

// COMMANDLIST

CommandList::CommandList()
: IsUpdateFromOSM(false)
{
}

CommandList::~CommandList(void)
{
	for (unsigned int i=0; i<Subs.size(); ++i)
		delete Subs[i];
}

void CommandList::setIsUpdateFromOSM()
{
	IsUpdateFromOSM = true;
}

void CommandList::add(Command* aCommand)
{
	Subs.push_back(aCommand);
}

bool CommandList::empty() const
{
	return Subs.size() == 0;
}

void CommandList::redo()
{
	for (unsigned int i=0; i<Subs.size(); ++i)
		Subs[i]->redo();
}

void CommandList::undo()
{
	for (unsigned int i=Subs.size(); i; --i)
		Subs[i-1]->undo();
}

bool CommandList::buildDirtyList(DirtyList& theList)
{
	if (IsUpdateFromOSM)
		Subs.clear();
	for (unsigned int i=0; i<Subs.size();)
	{
		if (Subs[i]->buildDirtyList(theList))
		{
			delete Subs[i];
			Subs.erase(Subs.begin()+i);
		}
		else
			++i;
	}
	return Subs.size() == 0;
}

bool CommandList::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("CommandList");
	xParent.appendChild(e);

	for (unsigned int i=0; i<Subs.size(); ++i) {
		OK = Subs[i]->toXML(e);
	}

	return OK;
}

CommandList* CommandList::fromXML(MapDocument* d, QDomElement& e)
{
	CommandList* l = new CommandList();
	l->setId(e.attribute("xml:id"));

	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "AddFeatureCommand") {
			AddFeatureCommand* C = AddFeatureCommand::fromXML(d, c);
			l->add(C);
		} else
		if (c.tagName() == "MoveTrackPointCommand") {
			MoveTrackPointCommand* C = MoveTrackPointCommand::fromXML(d, c);
			l->add(C);
		} else
		if (c.tagName() == "RelationAddFeatureCommand") {
			RelationAddFeatureCommand* C = RelationAddFeatureCommand::fromXML(d, c);
			l->add(C);
		} else
		if (c.tagName() == "RelationRemoveFeatureCommand") {
			RelationRemoveFeatureCommand* C = RelationRemoveFeatureCommand::fromXML(d, c);
			l->add(C);
		} else
		if (c.tagName() == "RemoveFeatureCommand") {
			RemoveFeatureCommand* C = RemoveFeatureCommand::fromXML(d, c);
			l->add(C);
		} else
		if (c.tagName() == "RoadAddTrackPointCommand") {
			RoadAddTrackPointCommand* C = RoadAddTrackPointCommand::fromXML(d, c);
			l->add(C);
		} else
		if (c.tagName() == "RoadRemoveTrackPointCommand") {
			RoadRemoveTrackPointCommand* C = RoadRemoveTrackPointCommand::fromXML(d, c);
			l->add(C);
		} else
		if (c.tagName() == "ClearTagCommand") {
			ClearTagCommand* C = ClearTagCommand::fromXML(d, c);
			l->add(C);
		//} else
		//if (c.tagName() == "ClearTagsCommand") {
		} else
		if (c.tagName() == "SetTagCommand") {
			SetTagCommand* C = SetTagCommand::fromXML(d, c);
			l->add(C);
		} else
		if (c.tagName() == "CommandList") {
			l->add(CommandList::fromXML(d, c));
		}

		c = c.nextSiblingElement();
	}

	return l;
}


// COMMANDHISTORY

CommandHistory::CommandHistory()
: Index(0), UndoAction(0), RedoAction(0)
{
}

CommandHistory::~CommandHistory()
{
	cleanup();
}

void CommandHistory::cleanup()
{
	for (unsigned int i=Index; i<Subs.size(); ++i)
		Subs[i]->redo();
	for (unsigned int i=0; i<Subs.size(); ++i)
		delete Subs[i];
	Subs.clear();
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
	if (Index < Subs.size())
	{
		Subs[Index++]->redo();
		updateActions();
	}
}

void CommandHistory::add(Command* aCommand)
{
	for (unsigned int i=Index; i<Subs.size(); ++i)
		delete Subs[i];
	Subs.erase(Subs.begin()+Index,Subs.end());
	Subs.push_back(aCommand);
	Index = Subs.size();
	updateActions();
}

void CommandHistory::setActions(QAction* anUndo, QAction* aRedo)
{
	UndoAction = anUndo;
	RedoAction = aRedo;
	updateActions();
}

void CommandHistory::updateActions()
{
	if (UndoAction)
		UndoAction->setEnabled(Index>0);
	if (RedoAction)
		RedoAction->setEnabled(Index<Subs.size());
}

unsigned int CommandHistory::index() const
{
	return Index;
}

unsigned int CommandHistory::buildDirtyList(DirtyList& theList)
{
	for (unsigned int i=0; i<Index;)
		if (Subs[i]->buildDirtyList(theList))
		{
			delete Subs[i];
			Subs.erase(Subs.begin()+i);
			--Index;
		}
		else
			++i;
	return Index;
}

bool CommandHistory::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.namedItem("CommandHistory").toElement();
	if (!e.isNull()) {
		xParent.removeChild(e);
	}

	e = xParent.ownerDocument().createElement("CommandHistory");
	xParent.appendChild(e);

	//e.setAttribute("index", QString::number(Index));

	for (unsigned int i=0; i<Index; ++i) {
		OK = Subs[i]->toXML(e);
	}

	return OK;
}

CommandHistory* CommandHistory::fromXML(MapDocument* d, QDomElement& e)
{
	CommandHistory* h = new CommandHistory();

	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "CommandList") {
			CommandList* l = CommandList::fromXML(d, c);
			h->add(l);
		} else
		if (c.tagName() == "SetTagCommand") {
			SetTagCommand* C = SetTagCommand::fromXML(d, c);
			h->add(C);
		}
		c = c.nextSiblingElement();
	}
	//h->Index = e.attribute("index").toUInt();

	return h;
}


