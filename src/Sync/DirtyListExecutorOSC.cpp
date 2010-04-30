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

#include "DirtyListExecutorOSC.h"

#include "Features.h"
#include "ExportOSM.h"
#include "DownloadOSM.h"
#include "MerkaartorPreferences.h"
#include "Command.h"

#include <QMessageBox>
#include <QDebug>
#include <QProgressDialog>
#include <QRegExp>

extern int glbAdded, glbUpdated, glbDeleted;
extern QString glbChangeSetComment;

DirtyListExecutorOSC::DirtyListExecutorOSC(Document* aDoc, const DirtyListBuild& aFuture)
    : DirtyListVisit(aDoc, aFuture, false)
    , Done(0)
    , theDownloader(0)
{
}

DirtyListExecutorOSC::DirtyListExecutorOSC(Document* aDoc, const DirtyListBuild& aFuture, const QString& aWeb, const QString& aUser, const QString& aPwd, int aTasks)
: DirtyListVisit(aDoc, aFuture, false), Tasks(aTasks), Done(0), Web(aWeb), User(aUser), Pwd(aPwd), theDownloader(0)
{
    theDownloader = new Downloader(Web, User, Pwd);
}

DirtyListExecutorOSC::~DirtyListExecutorOSC()
{
    delete theDownloader;
}


int DirtyListExecutorOSC::sendRequest(const QString& Method, const QString& URL, const QString& Data, QString& Rcv)
{
    if (inError())
        return false;

    int rCode;

    QMessageBox::StandardButton theChoice = QMessageBox::Retry;
    while (theChoice == QMessageBox::Retry) {
        if (!theDownloader->request(Method,URL,Data))
        {
            qDebug() << QString("Upload error: request (%1); Server message is '%2'").arg(theDownloader->resultCode()).arg(theDownloader->resultText());
            if (theDownloader->resultCode() == 401) {
                QMessageBox::warning(Progress,tr("Error uploading request"),
                    tr("Please check your username and password in the Preferences menu"));
                theChoice = QMessageBox::Abort;
            } else {
                QString msg = tr("There was an error uploading this request (%1)\nServer message is '%2'").arg(theDownloader->resultCode()).arg(theDownloader->resultText());
                if (!theDownloader->errorText().isEmpty())
                    msg += tr("\nAPI message is '%1'").arg(theDownloader->errorText());
                theChoice = QMessageBox::warning(Progress,tr("Error uploading request"), msg, QMessageBox::Abort | QMessageBox::Retry);
                continue;
            }
        }

        QByteArray Content = theDownloader->content();
        Rcv = QString::fromUtf8(Content.data());
        rCode = theDownloader->resultCode();

        qDebug() << QString("Upload: rCode (%1); Msg (%2)").arg(rCode).arg(Rcv);

        if (rCode == 200)
            break;

        theChoice = QMessageBox::warning(Progress,tr("Error uploading request"),
                                         tr(
                                                 "There was an error uploading this request (%1)\n\"%2\"\n"
                                                 "Please redownload the problematic feature to handle the conflict."
                                                 ).arg(rCode).arg(Rcv),
                                         QMessageBox::Abort | QMessageBox::Retry);
    }
    if (theChoice == QMessageBox::Abort) {
        errorAbort = true;
        return rCode;
    }
    return rCode;
}

QString DirtyListExecutorOSC::getChanges()
{
    Progress = new QProgressDialog(0);
    Progress->setWindowTitle(tr("Checking changes..."));
    Progress->setWindowFlags(Progress->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    Progress->setWindowFlags(Progress->windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
    Progress->setWindowModality(Qt::WindowModal);
    Progress->setMinimumDuration(0);
    Progress->setMaximum(Tasks+2);
    Progress->show();

    OscDoc.appendChild(OscDoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));

    OscRoot = OscDoc.createElement("osmChange ");
    OscDoc.appendChild(OscRoot);
    OscRoot.setAttribute("version", "0.3");
    OscRoot.setAttribute("generator", QString("Merkaartor %1").arg(STRINGIFY(VERSION)));

    runVisit();

    SAFE_DELETE(Progress)

    return OscDoc.toString();
}

bool DirtyListExecutorOSC::executeChanges(QWidget* aParent)
{
    bool ok;

    MainWindow* main = dynamic_cast<MainWindow*>(aParent);
    main->createProgressDialog();

    Progress = main->getProgressDialog();
    if (Progress)
        Progress->setWindowTitle(QApplication::translate("Downloader", "Uploading..."));

    QProgressBar* Bar = main->getProgressBar();
    Bar->setTextVisible(false);

    QLabel* Lbl = main->getProgressLabel();

    if (Progress)
        Progress->show();

    if (theDownloader)
        theDownloader->setAnimator(Progress,Lbl,Bar,false);

    if ((ok = start()))
    {
        OscDoc.appendChild(OscDoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
        OscRoot = OscDoc.createElement("osmChange ");
        OscDoc.appendChild(OscRoot);
        OscRoot.setAttribute("version", "0.3");
        OscRoot.setAttribute("generator", QString("Merkaartor %1").arg(STRINGIFY(VERSION)));

        Lbl->setText(QApplication::translate("Downloader","Preparing changes"));
        if ((ok = runVisit())) {
            Lbl->setText(QApplication::translate("Downloader","Waiting for server response"));
            ok = stop();
        }
    }
    main->deleteProgressDialog();

    return ok;
}

bool DirtyListExecutorOSC::start()
{
    ChangeSetId = "";
    Progress->setValue(++Done);
    qDebug() << QString("OPEN changeset");

    Progress->setLabelText(tr("OPEN changeset"));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    QString DataIn(
        "<osm>"
        "<changeset>"
        "<tag k=\"created_by\" v=\"Merkaartor %1 (%2)\"/>"
        "<tag k=\"comment\" v=\"%3\"/>"
        "</changeset>"
        "</osm>");
    DataIn = DataIn.arg(STRINGIFY(VERSION)).arg(QLocale::system().name().split("_")[0]).arg(encodeAttributes(glbChangeSetComment));
    QString DataOut;
    QString URL = theDownloader->getURLToOpenChangeSet();
    if (sendRequest("PUT",URL,DataIn, DataOut) != 200)
        return false;

    ChangeSetId = DataOut;

    return true;
}

bool DirtyListExecutorOSC::stop()
{
    QString DataIn, DataOut;
    QString errFeat;

    QString URL = theDownloader->getURLToUploadDiff(ChangeSetId);
    switch (sendRequest("POST", URL, OscDoc.toString(), DataOut)) {
    case 200: {
        QDomDocument resDoc;
        if (resDoc.setContent(DataOut)) {

            QDomNodeList nl = resDoc.elementsByTagName("diffResult");
            if (nl.size()) {
                QDomElement resRoot = nl.at(0).toElement();
                QDomElement c = resRoot.firstChildElement();
                while (!c.isNull()) {
                    Feature* F = theDocument->getFeature(c.attribute("old_id"), false);
                    if (F) {
                        QString idPrefix;
                        switch (F->getType()) {
                        case Feature::Nodes:
                            idPrefix = "node_";
                            break;
                        case Feature::Ways:
                            idPrefix = "way_";
                            break;
                        case Feature::Relations:
                            idPrefix = "rel_";
                            break;
                        default:
                            break;
                        }

                        F->setId(idPrefix + c.attribute("new_id"));
                        F->setVersionNumber(c.attribute("new_version").toInt());
                        F->setLastUpdated(Feature::OSMServer);

                        F->layer()->remove(F);
                        document()->getUploadedLayer()->add(F);
                        F->setUploaded(true);

                    } else
                        qDebug() << "Feature not found in diff upload result: " << c.attribute("old_id");

                    c = c.nextSiblingElement();
                }
                theDocument->history().cleanup();
            }
        }
        break;
    }

    case 409: {    // Confilct
//            QRegExp rx(".*node.*(\\d+)", Qt::CaseInsensitive);
//            if (rx.indexIn(DataOut) > -1) {
//                errFeat = "node_" + rx.cap(1);
//            } else {
//                QRegExp rx(".*way.*(\\d+)", Qt::CaseInsensitive);
//                if (rx.indexIn(DataOut) > -1) {
//                    errFeat = "way_" + rx.cap(1);
//                } else {
//                    QRegExp rx(".*relation.*(\\d+)", Qt::CaseInsensitive);
//                    if (rx.indexIn(DataOut) > -1) {
//                        errFeat = "relation_" + rx.cap(1);
//                    }
//                }
//            }
//            qDebug() << errFeat;
            break;
        }
    case 410: {    // Gone
//            QRegExp rx(".*node.*(\\d+)", Qt::CaseInsensitive);
//            if (rx.indexIn(DataOut) > -1) {
//                errFeat = "node_" + rx.cap(1);
//            } else {
//                QRegExp rx(".*way.*(\\d+)", Qt::CaseInsensitive);
//                if (rx.indexIn(DataOut) > -1) {
//                    errFeat = "way_" + rx.cap(1);
//                } else {
//                    QRegExp rx(".*relation.*(\\d+)", Qt::CaseInsensitive);
//                    if (rx.indexIn(DataOut) > -1) {
//                        errFeat = "relation_" + rx.cap(1);
//                    }
//                }
//            }
//            qDebug() << errFeat;
            break;
        }

    default:
        break;
    }

    qDebug() << QString("CLOSE changeset");

    Progress->setLabelText(tr("CLOSE changeset"));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    URL = theDownloader->getURLToCloseChangeSet(ChangeSetId);
    if (sendRequest("PUT",URL,DataIn,DataOut) != 200)
    {
        ChangeSetId = "";
        return false;
    }
    return true;
}

void DirtyListExecutorOSC::OscCreate(Feature* F)
{
    if (OscCurElem.tagName() != "create") {
        OscCurElem = OscDoc.createElement("create");
        OscRoot.appendChild(OscCurElem);
    }

    F->toXML(OscCurElem, *Progress, true);
    if (!ChangeSetId.isEmpty())
        OscCurElem.lastChildElement().setAttribute("changeset", ChangeSetId);
}

void DirtyListExecutorOSC::OscModify(Feature* F)
{
    if (OscCurElem.tagName() != "modify") {
        OscCurElem = OscDoc.createElement("modify");
        OscRoot.appendChild(OscCurElem);
    }

    F->toXML(OscCurElem, *Progress, true);
    if (!ChangeSetId.isEmpty())
        OscCurElem.lastChildElement().setAttribute("changeset", ChangeSetId);
}

void DirtyListExecutorOSC::OscDelete(Feature* F)
{
    if (OscCurElem.tagName() != "delete") {
        OscCurElem = OscDoc.createElement("delete");
        OscRoot.appendChild(OscCurElem);
    }

    F->toXML(OscCurElem, *Progress, true);
    if (!ChangeSetId.isEmpty())
        OscCurElem.lastChildElement().setAttribute("changeset", ChangeSetId);
}



bool DirtyListExecutorOSC::addRelation(Relation *F)
{
    qDebug() << QString("ADD relation %1").arg(F->id());

    Progress->setLabelText(tr("ADD relation %1").arg(F->id()) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscCreate(F);

    return false;
}

bool DirtyListExecutorOSC::addRoad(Way *F)
{
    Progress->setValue(++Done);

    qDebug() << QString("ADD road %1").arg(F->id());

    Progress->setLabelText(tr("ADD road %1").arg(F->id()) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscCreate(F);

    return false;
}


bool DirtyListExecutorOSC::addPoint(Node* F)
{
    Progress->setValue(++Done);

    qDebug() << QString("ADD trackpoint %1").arg(F->id());

    Progress->setLabelText(tr("ADD trackpoint %1").arg(F->id()) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscCreate(F);

    return false;
}



bool DirtyListExecutorOSC::updateRelation(Relation* F)
{
    Progress->setValue(++Done);

    qDebug() << QString("UPDATE relation %1").arg(F->id());

    Progress->setLabelText(tr("UPDATE relation %1").arg(F->id()) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscModify(F);

    return false;
}


bool DirtyListExecutorOSC::updateRoad(Way* F)
{
    Progress->setValue(++Done);

    qDebug() << QString("UPDATE road %1").arg(F->id());

    Progress->setLabelText(tr("UPDATE road %1").arg(F->id()) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscModify(F);

    return false;
}

bool DirtyListExecutorOSC::updatePoint(Node* F)
{
    Progress->setValue(++Done);

    qDebug() << QString("UPDATE trackpoint %1").arg(F->id());

    Progress->setLabelText(tr("UPDATE trackpoint %1").arg(F->id()) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscModify(F);

    return false;
}

bool DirtyListExecutorOSC::erasePoint(Node *F)
{
    Progress->setValue(++Done);

    qDebug() << QString("REMOVE trackpoint %1").arg(F->id());

    Progress->setLabelText(tr("REMOVE trackpoint %1").arg(F->id()) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscDelete(F);

    return false;
}

bool DirtyListExecutorOSC::eraseRoad(Way *F)
{
    Progress->setValue(++Done);

    qDebug() << QString("REMOVE road %1").arg(F->id());

    Progress->setLabelText(tr("REMOVE road %1").arg(F->id()) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscDelete(F);

    return false;
}

bool DirtyListExecutorOSC::eraseRelation(Relation *F)
{
    Progress->setValue(++Done);

    qDebug() << QString("REMOVE relation %1").arg(F->id());

    Progress->setLabelText(tr("REMOVE relation %1").arg(F->id()) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscDelete(F);

    return false;
}
