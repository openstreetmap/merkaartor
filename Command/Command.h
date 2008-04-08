#ifndef MERKATOR_COMMAND_H_
#define MERKATOR_COMMAND_H_

#include <vector>
#include <QtXml>

class MapDocument;
class DirtyList;

class QAction;

class Command
{
	public:
		virtual ~Command(void) = 0;

		virtual void undo() = 0;
		virtual void redo() = 0;
		virtual bool buildDirtyList(DirtyList& theList) = 0;

		void setId(const QString& id);
		const QString& id() const;
		virtual bool toXML(QDomElement& xParent) const = 0;

	protected:
		mutable QString Id;
};

class CommandList : public Command
{
	public:
		CommandList();
		virtual ~CommandList();

		virtual void undo();
		virtual void redo();
		bool empty() const;
		void add(Command* aCommand);
		bool buildDirtyList(DirtyList& theList);
		void setIsUpdateFromOSM();

		virtual bool toXML(QDomElement& xParent) const;
		static CommandList* fromXML(MapDocument* d, QDomElement& e);

	private:
		std::vector<Command*> Subs;
		bool IsUpdateFromOSM;
};

class CommandHistory
{
	public:
		CommandHistory();
		virtual ~CommandHistory();

		void cleanup();
		void undo();
		void redo();
		void add(Command* aCommand);
		void setActions(QAction* anUndo, QAction* aRedo);
		unsigned int buildDirtyList(DirtyList& theList);
		unsigned int index() const;

		virtual bool toXML(QDomElement& xParent) const;
		static CommandHistory* fromXML(MapDocument* d, QDomElement& e);

	private:
		void updateActions();
		std::vector<Command*> Subs;
		unsigned int Index;
		QAction* UndoAction;
		QAction* RedoAction;
};

#endif


