#ifndef MERKATOR_DIRTYLIST_H_
#define MERKATOR_DIRTYLIST_H_

class Downloader;
class Document;
class Feature;
class Relation;
class Way;
class Node;
class Way;

class QProgressDialog;
class QWidget;

#include <ui_SyncListDialog.h>

#include <QtCore/QObject>
#include <QtCore/QString>

#include <utility>
#include <QList>

class DirtyList
{
	public:
		DirtyList() : errorAbort(false) {}
		virtual ~DirtyList() = 0;
		virtual bool add(Feature* F) = 0;
		virtual bool update(Feature* F) = 0;
		virtual bool erase(Feature* F) = 0;
		virtual bool noop(Feature* F) = 0;

		virtual bool inError() {return errorAbort; }

	protected:
		bool errorAbort;
};

class DirtyListBuild : public DirtyList
{
	public:
		virtual bool add(Feature* F);
		virtual bool update(Feature* F);
		virtual bool erase(Feature* F);
		virtual bool noop(Feature*) {return false;}

		virtual bool willBeAdded(Feature* F) const;
		virtual bool willBeErased(Feature* F) const;
		virtual bool updateNow(Feature* F) const;
		virtual void resetUpdates();

	protected:
		QList<Feature*> Added, Deleted;
		QList<Feature*> Updated;
		mutable QList<QPair<int, int> > UpdateCounter;
};

class DirtyListVisit : public DirtyList
{
	public:
		DirtyListVisit(Document* aDoc, const DirtyListBuild& aFuture, bool aEraseFromHistory);

		Document* document();
		bool runVisit();

		virtual bool add(Feature* F);
		virtual bool update(Feature* F);
		virtual bool erase(Feature* F);
		virtual bool noop(Feature* F);

		virtual bool addPoint(Node* Pt) = 0;
		virtual bool addRoad(Way* R) = 0;
		virtual bool addRelation(Relation* R) = 0;
		virtual bool updatePoint(Node* Pt) = 0;
		virtual bool updateRoad(Way* R) = 0;
		virtual bool updateRelation(Relation* R) = 0;
		virtual bool erasePoint(Node* Pt) = 0;
		virtual bool eraseRoad(Way* R) = 0;
		virtual bool eraseRelation(Relation* R) = 0;

	protected:
		bool notYetAdded(Feature* F);
		Document* theDocument;
		const DirtyListBuild& Future;
		bool EraseFromHistory;
		QList<Feature*> Updated;
		QList<Feature*> AlreadyAdded;
		QList<bool> EraseResponse;
		bool DeletePass;
		QMap<Node*, bool> TrackPointsToDelete;
		QMap<Way*, bool> RoadsToDelete;
		QMap<Relation*, bool> RelationsToDelete;
};

class DirtyListDescriber : public DirtyListVisit
{
	public:
		DirtyListDescriber(Document* aDoc, const DirtyListBuild& aFuture);

		virtual bool addPoint(Node* Pt);
		virtual bool addRoad(Way* R);
		virtual bool addRelation(Relation* R);
		virtual bool updatePoint(Node* Pt);
		virtual bool updateRoad(Way* R);
		virtual bool updateRelation(Relation *R);
		virtual bool erasePoint(Node* Pt);
		virtual bool eraseRoad(Way* R);
		virtual bool eraseRelation(Relation* R);

		bool showChanges(QWidget* Parent);
		int tasks() const;

	private:
		Ui::SyncListDialog Ui;

	protected:
		QListWidget* theListWidget;
		int Task;
};

class DirtyListExecutor : public QObject, public DirtyListVisit
{
	Q_OBJECT

	public:
		DirtyListExecutor(Document* aDoc, const DirtyListBuild& aFuture, const QString& aWeb, const QString& aUser, const QString& aPwd, int aTasks);
		virtual ~DirtyListExecutor();

		virtual bool start();
		virtual bool stop();
		virtual bool addPoint(Node* Pt);
		virtual bool addRoad(Way* R);
		virtual bool addRelation(Relation* R);
		virtual bool updatePoint(Node* Pt);
		virtual bool updateRoad(Way* R);
		virtual bool updateRelation(Relation* R);
		virtual bool erasePoint(Node* Pt);
		virtual bool eraseRoad(Way* R);
		virtual bool eraseRelation(Relation* R);

		bool executeChanges(QWidget* Parent);

	private:
		bool sendRequest(const QString& Method, const QString& URL, const QString& Out, QString& Rcv);

		Ui::SyncListDialog Ui;
		int Tasks, Done;
		QProgressDialog* Progress;
		QString Web,User,Pwd;
		Downloader* theDownloader;
		QString ChangeSetId;
};


#endif


