#ifndef MERKATOR_COMMAND_H_
#define MERKATOR_COMMAND_H_

#include <QList>
#include <QtXml>

#define KEY_UNDEF_VALUE "%%%%%"
#define TAG_UNDEF_VALUE "%%%%%"

class MapDocument;
class MapLayer;
class MapFeature;
class DirtyList;
class CommandList;

class QAction;
class QListWidget;
class QProgressDialog;

class Command
{
	public:
		Command(MapFeature* aF);
		virtual ~Command(void) = 0;

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList) = 0;
		virtual bool buildUndoList(QListWidget* theList);

		void setId(const QString& id);
		const QString& id() const;
		virtual bool toXML(QDomElement& xParent) const;
		static void fromXML(MapDocument* d, const QDomElement& e, Command* C);

		virtual QString getDescription();
		virtual void setDescription(QString desc);
		virtual MapFeature* getFeature();
		virtual void setFeature(MapFeature* feat);

		int incDirtyLevel(MapLayer* aLayer);
		int decDirtyLevel(MapLayer* aLayer);
		int getDirtyLevel();

	protected:
		mutable QString Id;
		QString description;
		MapFeature* mainFeature;
		int commandDirtyLevel;
		QString oldCreated;
		bool isUndone;
		Command* postUploadCommand;
		bool wasUploaded;
};

class CommandList : public Command
{
	public:
		CommandList();
		CommandList(QString aDesc, MapFeature* aFeat=NULL);
		virtual ~CommandList();

		virtual void undo();
		virtual void redo();
		bool empty() const;
		int size();
		void add(Command* aCommand);
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static CommandList* fromXML(MapDocument* d, const QDomElement& e);

	private:
		QList<Command*> Subs;
		int Size;
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
		void setActions(QAction* anUndo, QAction* aRedo, QAction* anUploadAction);
		void updateActions();
		int buildDirtyList(DirtyList& theList);
		int buildUndoList(QListWidget* theList);
		int index() const;

		virtual bool toXML(QDomElement& xParent, QProgressDialog & progress) const;
		static CommandHistory* fromXML(MapDocument* d, QDomElement& e, QProgressDialog & progress);

	private:
		QList<Command*> Subs;
		int Index;
		int Size;
		QAction* UndoAction;
		QAction* RedoAction;
		QAction* UploadAction;
};

#endif


