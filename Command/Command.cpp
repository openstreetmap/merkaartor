#include "Command/Command.h"

#include <QtGui/QAction>

Command::~Command()
{
}

CommandList::~CommandList(void)
{
	for (unsigned int i=0; i<Subs.size(); ++i)
		delete Subs[i];
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
		delete  Subs[i];
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




