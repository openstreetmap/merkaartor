//***************************************************************
// CLass: DirtyListExecutorOSCOSC
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//******************************************************************

#ifndef DirtyListExecutorOSC_H
#define DirtyListExecutorOSC_H

#include "DirtyList.h"

#include <QXmlStreamWriter>
#include <QBuffer>
#include "Utils/OsmServer.h"

class Downloader;

class DirtyListExecutorOSC : public QObject, public DirtyListVisit
{
    Q_OBJECT

public:
    DirtyListExecutorOSC(Document* aDoc, const DirtyListBuild& aFuture);
    DirtyListExecutorOSC(Document* aDoc, const DirtyListBuild& aFuture, const ChangesetInfo& info, OsmServer server, int aTasks);
    virtual ~DirtyListExecutorOSC();

    void OscCreate(Feature* F);
    void OscModify(Feature* F);
    void OscDelete(Feature* F);

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
    QByteArray getChanges();

private:
    int sendRequest(const QString& Method, const QString& URL, const QString& Out, QString& Rcv);

    QXmlStreamWriter OscStream;
    QBuffer OscBuffer;

    Ui::SyncListDialog Ui;
    int Tasks, Done;
    QProgressDialog* Progress;
    OsmServer server;
    Downloader* theDownloader;
    QString ChangeSetId;
    QString LastAction;
    ChangesetInfo changesetInfo;
};


#endif // DirtyListExecutorOSC_H
