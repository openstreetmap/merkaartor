#ifndef MERKATOR_COMMAND_H_
#define MERKATOR_COMMAND_H_

#include <vector>

class DirtyList;

class QAction;

class Command
{
	public:
		virtual ~Command(void) = 0;

		virtual void undo() = 0;
		virtual void redo() = 0;
		virtual bool buildDirtyList(DirtyList& theList) = 0;
};

class CommandList : public Command
{
	public:
		virtual ~CommandList();

		virtual void undo();
		virtual void redo();
		bool empty() const;
		void add(Command* aCommand);
		bool buildDirtyList(DirtyList& theList);

	private:
		std::vector<Command*> Subs;
};

class CommandHistory
{
	public:
		CommandHistory();
		~CommandHistory();

		void cleanup();
		void undo();
		void redo();
		void add(Command* aCommand);
		void setActions(QAction* anUndo, QAction* aRedo);
		unsigned int buildDirtyList(DirtyList& theList);
		unsigned int index() const;

	private:
		void updateActions();
		std::vector<Command*> Subs;
		unsigned int Index;
		QAction* UndoAction;
		QAction* RedoAction;
};

#endif


