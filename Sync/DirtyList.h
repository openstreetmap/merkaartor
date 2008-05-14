#ifndef MERKATOR_DIRTYLIST_H_
#define MERKATOR_DIRTYLIST_H_

class Downloader;
class MapDocument;
class MapFeature;
class Relation;
class Road;
class TrackPoint;
class Way;

class QProgressDialog;
class QWidget;

#include <ui_SyncListDialog.h>

#include <QtCore/QObject>
#include <QtCore/QString>

#include <utility>
#include <vector>

class DirtyList
{
	public:
		virtual ~DirtyList() = 0;
		virtual bool add(MapFeature* F) = 0;
		virtual bool update(MapFeature* F) = 0;
		virtual bool erase(MapFeature* F) = 0;
};

class DirtyListBuild : public DirtyList
{
	public:
		virtual bool add(MapFeature* F);
		virtual bool update(MapFeature* F);
		virtual bool erase(MapFeature* F);

		bool willBeAdded(MapFeature* F) const;
		bool willBeErased(MapFeature* F) const;
		bool updateNow(MapFeature* F) const;
		void resetUpdates();

	private:
		std::vector<MapFeature*> Added, Deleted;
		std::vector<MapFeature*> Updated;
		mutable std::vector<std::pair<unsigned int, unsigned int> > UpdateCounter;
};

class DirtyListVisit : public DirtyList
{
	public:
		DirtyListVisit(MapDocument* aDoc, const DirtyListBuild& aFuture, bool aEraseFromHistory);

		MapDocument* document();
		virtual bool add(MapFeature* F);
		virtual bool update(MapFeature* F);
		virtual bool erase(MapFeature* F);

		virtual bool addPoint(TrackPoint* Pt) = 0;
		virtual bool addRoad(Road* R) = 0;
		virtual bool addRelation(Relation* R) = 0;
		virtual bool updatePoint(TrackPoint* Pt) = 0;
		virtual bool updateRoad(Road* R) = 0;
		virtual bool updateRelation(Relation* R) = 0;
		virtual bool erasePoint(TrackPoint* Pt) = 0;
		virtual bool eraseRoad(Road* R) = 0;
		virtual bool eraseRelation(Relation* R) = 0;

	private:
		bool notYetAdded(MapFeature* F);
		MapDocument* theDocument;
		const DirtyListBuild& Future;
		bool EraseFromHistory;
		std::vector<MapFeature*> Updated;
		std::vector<MapFeature*> AlreadyAdded;
		std::vector<bool> EraseResponse;
};

class DirtyListDescriber : public DirtyListVisit
{
	public:
		DirtyListDescriber(MapDocument* aDoc, const DirtyListBuild& aFuture);

		virtual bool addPoint(TrackPoint* Pt);
		virtual bool addRoad(Road* R);
		virtual bool addRelation(Relation* R);
		virtual bool updatePoint(TrackPoint* Pt);
		virtual bool updateRoad(Road* R);
		virtual bool updateRelation(Relation *R);
		virtual bool erasePoint(TrackPoint* Pt);
		virtual bool eraseRoad(Road* R);
		virtual bool eraseRelation(Relation* R);

		bool showChanges(QWidget* Parent);
		unsigned int tasks() const;

	private:
		Ui::SyncListDialog Ui;
		unsigned int Task;
};


class DirtyListExecutor : public QObject, public DirtyListVisit
{
	Q_OBJECT

	public:
		DirtyListExecutor(MapDocument* aDoc, const DirtyListBuild& aFuture, const QString& aWeb, const QString& aUser, const QString& aPwd, bool UseProxy, const QString& ProxyHost, int ProxyPort, unsigned int aTasks);
		virtual ~DirtyListExecutor();

		virtual bool start();
		virtual bool stop();
		virtual bool addPoint(TrackPoint* Pt);
		virtual bool addRoad(Road* R);
		virtual bool addRelation(Relation* R);
		virtual bool updatePoint(TrackPoint* Pt);
		virtual bool updateRoad(Road* R);
		virtual bool updateRelation(Relation* R);
		virtual bool erasePoint(TrackPoint* Pt);
		virtual bool eraseRoad(Road* R);
		virtual bool eraseRelation(Relation* R);

		bool executeChanges(QWidget* Parent);

	private:
		bool sendRequest(const QString& Method, const QString& URL, const QString& Out, QString& Rcv);

		Ui::SyncListDialog Ui;
		unsigned int Tasks, Done;
		QProgressDialog* Progress;
		QString Web,User,Pwd;
		bool UseProxy;
		QString ProxyHost;
		int ProxyPort;
		Downloader* theDownloader;
		QString ChangeSetId;
};


#endif


