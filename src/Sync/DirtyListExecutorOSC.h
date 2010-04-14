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

#include <QDomDocument>

class Downloader;

class DirtyListExecutorOSC : public QObject, public DirtyListVisit
{
    Q_OBJECT

public:
    DirtyListExecutorOSC(Document* aDoc, const DirtyListBuild& aFuture);
    DirtyListExecutorOSC(Document* aDoc, const DirtyListBuild& aFuture, const QString& aWeb, const QString& aUser, const QString& aPwd, int aTasks);
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
    QString getChanges();

private:
    int sendRequest(const QString& Method, const QString& URL, const QString& Out, QString& Rcv);

    QDomDocument OscDoc;
    QDomElement OscRoot;
    QDomElement OscCurElem;

    Ui::SyncListDialog Ui;
    int Tasks, Done;
    QProgressDialog* Progress;
    QString Web,User,Pwd;
    Downloader* theDownloader;
    QString ChangeSetId;
};


#endif // DirtyListExecutorOSC_H
