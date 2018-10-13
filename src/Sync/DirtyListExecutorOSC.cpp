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

#include "Global.h"
#include "DirtyListExecutorOSC.h"

#include "MainWindow.h"
#include "Features.h"
#include "DownloadOSM.h"
#include "MerkaartorPreferences.h"
#include "Command.h"
#include "Utils.h"

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
    theDownloader = new Downloader(User, Pwd);
}

DirtyListExecutorOSC::~DirtyListExecutorOSC()
{
    delete theDownloader;
}


int DirtyListExecutorOSC::sendRequest(const QString& Method, const QString& URL, const QString& Data, QString& Rcv)
{
    if (inError())
        return false;

    int rCode = -1;

    QMessageBox::StandardButton theChoice = QMessageBox::Retry;
    while (theChoice == QMessageBox::Retry) {
        QUrl theUrl(Web+URL);
        if (!theDownloader->request(Method,theUrl,Data))
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

QByteArray DirtyListExecutorOSC::getChanges()
{
    Progress = new QProgressDialog(0);
    Progress->setWindowTitle(tr("Checking changes..."));
    Progress->setWindowFlags(Progress->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    Progress->setWindowFlags(Progress->windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
    Progress->setWindowModality(Qt::WindowModal);
    Progress->setMinimumDuration(0);
    Progress->setMaximum(Tasks+2);
    Progress->show();

    OscBuffer.buffer().clear();
    OscBuffer.open(QIODevice::WriteOnly);

    OscStream.setDevice(&OscBuffer);
    OscStream.writeStartDocument();

    OscStream.writeStartElement("osmChange ");
    OscStream.writeAttribute("version", "0.3");
    OscStream.writeAttribute("generator", QString("Merkaartor %1").arg(STRINGIFY(VERSION)));

    runVisit();

    SAFE_DELETE(Progress)

    OscStream.writeEndDocument();
    OscBuffer.close();

    return OscBuffer.buffer();
}

bool DirtyListExecutorOSC::executeChanges(QWidget* aParent)
{
    bool ok = true;

#ifndef _MOBILE
    MainWindow* main = dynamic_cast<MainWindow*>(aParent);
    main->createProgressDialog();

    Progress = main->getProgressDialog();
    if (Progress) {
        Progress->setMaximum(Tasks);
        Progress->setWindowTitle(QApplication::translate("Downloader", "Uploading..."));
    }

    QProgressBar* Bar = main->getProgressBar();
    Bar->setTextVisible(false);

    QLabel* Lbl = main->getProgressLabel();

    if (Progress)
        Progress->show();

    if (theDownloader)
        theDownloader->setAnimator(Progress,Lbl,Bar,false);

    if ((ok = start()))
    {
        OscBuffer.buffer().clear();
        OscBuffer.open(QIODevice::WriteOnly);
        OscStream.setDevice(&OscBuffer);
        OscStream.writeStartDocument();

        OscStream.writeStartElement("osmChange ");
        OscStream.writeAttribute("version", "0.3");
        OscStream.writeAttribute("generator", QString("Merkaartor %1").arg(STRINGIFY(VERSION)));

        Lbl->setText(QApplication::translate("Downloader","Preparing changes"));
        if ((ok = runVisit())) {
            Lbl->setText(QApplication::translate("Downloader","Waiting for server response"));
            ok = stop();
        }
    }
    main->deleteProgressDialog();
#endif

    return ok;
}

bool DirtyListExecutorOSC::start()
{
    ChangeSetId.clear();
    Progress->setValue(++Done);
    qDebug() << QString("OPEN changeset");

    Progress->setLabelText(tr("OPEN changeset"));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    QStringList sl = theDocument->getCurrentSourceTags();

    QString DataIn(
        "<osm>"
        "<changeset>"
        "<tag k=\"created_by\" v=\"Merkaartor %1 (%2)\"/>"
        "<tag k=\"comment\" v=\"%3\"/>%4"
        "</changeset>"
        "</osm>");
    DataIn = DataIn
        .arg(STRINGIFY(VERSION))
        .arg(QLocale::system().name().split("_")[0])
        .arg(Utils::encodeAttributes(glbChangeSetComment))
        .arg((sl.size() ? "<tag k=\"source\" v=\""+ Utils::encodeAttributes(sl.join(";")) +"\"/>" : ""));

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

    OscStream.writeEndDocument();
    OscBuffer.close();
    qDebug() << OscBuffer.buffer();
    QString URL = theDownloader->getURLToUploadDiff(ChangeSetId);
    switch (sendRequest("POST", URL, QString::fromUtf8(OscBuffer.buffer().data()), DataOut)) {
    case 200: {
        QDomDocument resDoc;
        if (resDoc.setContent(DataOut)) {

            QDomNodeList nl = resDoc.elementsByTagName("diffResult");
            if (nl.size()) {
                QDomElement resRoot = nl.at(0).toElement();
                QDomElement c = resRoot.firstChildElement();
                while (!c.isNull()) {
                    IFeature::FeatureType aType = IFeature::FeatureType::Uninitialized;
                    if (c.tagName() == "node")
                        aType = IFeature::Point;
                    else if (c.tagName() == "way")
                        aType = IFeature::LineString;
                    else if (c.tagName() == "relation")
                        aType = IFeature::OsmRelation;
                    else {
                        qDebug() << "Unknown element found in response.";
                    }

                    Feature* F = theDocument->getFeature(IFeature::FId(aType, c.attribute("old_id").toLongLong()));
                    if (F) {
                        F->setId(IFeature::FId(aType, c.attribute("new_id").toLongLong()));
                        F->setVersionNumber(c.attribute("new_version").toInt());
                        F->setLastUpdated(Feature::OSMServer);
                        F->setUser("me");
                        F->setTime(QDateTime::currentDateTime());

                        if (!g_Merk_Frisius) {
                            F->layer()->remove(F);
                            document()->getUploadedLayer()->add(F);
                        }
                        F->setUploaded(true);
                        F->setDirtyLevel(0);

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
//                        errFeat = "rel_" + rx.cap(1);
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
//                        errFeat = "rel_" + rx.cap(1);
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
    QUrl theUrl(Web+URL);
    theDownloader->setAnimator(NULL, NULL, NULL, false);
    if (!theDownloader->request("PUT",theUrl,DataIn)) {
        QMessageBox::warning(NULL, tr("Changeset could not be closed."), tr("An unknown error has occurred. It might already be closed, or will be closed automatically. If you want to be sure, please, check manually on the osm.org website."));
    }

    return true;
}

void DirtyListExecutorOSC::OscCreate(Feature* F)
{
    if (LastAction != "create") {
        if (!LastAction.isEmpty())
            OscStream.writeEndElement();
        OscStream.writeStartElement("create");
        LastAction = "create";
    }

    F->toXML(OscStream, Progress, true, ChangeSetId);
}

void DirtyListExecutorOSC::OscModify(Feature* F)
{
    if (LastAction != "modify") {
        if (!LastAction.isEmpty())
            OscStream.writeEndElement();
        OscStream.writeStartElement("modify");
        LastAction = "modify";
    }

    F->toXML(OscStream, Progress, true, ChangeSetId);
}

void DirtyListExecutorOSC::OscDelete(Feature* F)
{
    if (LastAction != "delete") {
        if (!LastAction.isEmpty())
            OscStream.writeEndElement();
        OscStream.writeStartElement("delete");
        LastAction = "modify";
    }

    F->toXML(OscStream, Progress, true, ChangeSetId);
}



bool DirtyListExecutorOSC::addRelation(Relation *F)
{
    qDebug() << QString("ADD relation %1").arg(F->id().numId);

    Progress->setLabelText(tr("ADD relation %1").arg(F->id().numId) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscCreate(F);

    return false;
}

bool DirtyListExecutorOSC::addRoad(Way *F)
{
    Progress->setValue(++Done);

    qDebug() << QString("ADD road %1").arg(F->id().numId);

    Progress->setLabelText(tr("ADD road %1").arg(F->id().numId) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscCreate(F);

    return false;
}


bool DirtyListExecutorOSC::addPoint(Node* F)
{
    Progress->setValue(++Done);

    qDebug() << QString("ADD trackpoint %1").arg(F->id().numId);

    Progress->setLabelText(tr("ADD trackpoint %1").arg(F->id().numId) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscCreate(F);

    return false;
}



bool DirtyListExecutorOSC::updateRelation(Relation* F)
{
    Progress->setValue(++Done);

    qDebug() << QString("UPDATE relation %1").arg(F->id().numId);

    Progress->setLabelText(tr("UPDATE relation %1").arg(F->id().numId) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscModify(F);

    return false;
}


bool DirtyListExecutorOSC::updateRoad(Way* F)
{
    Progress->setValue(++Done);

    qDebug() << QString("UPDATE road %1").arg(F->id().numId);

    Progress->setLabelText(tr("UPDATE road %1").arg(F->id().numId) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscModify(F);

    return false;
}

bool DirtyListExecutorOSC::updatePoint(Node* F)
{
    Progress->setValue(++Done);

    qDebug() << QString("UPDATE trackpoint %1").arg(F->id().numId);

    Progress->setLabelText(tr("UPDATE trackpoint %1").arg(F->id().numId) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscModify(F);

    return false;
}

bool DirtyListExecutorOSC::erasePoint(Node *F)
{
    Progress->setValue(++Done);

    qDebug() << QString("REMOVE trackpoint %1").arg(F->id().numId);

    Progress->setLabelText(tr("REMOVE trackpoint %1").arg(F->id().numId) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscDelete(F);

    return false;
}

bool DirtyListExecutorOSC::eraseRoad(Way *F)
{
    Progress->setValue(++Done);

    qDebug() << QString("REMOVE road %1").arg(F->id().numId);

    Progress->setLabelText(tr("REMOVE road %1").arg(F->id().numId) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscDelete(F);

    return false;
}

bool DirtyListExecutorOSC::eraseRelation(Relation *F)
{
    Progress->setValue(++Done);

    qDebug() << QString("REMOVE relation %1").arg(F->id().numId);

    Progress->setLabelText(tr("REMOVE relation %1").arg(F->id().numId) + userName(F));
    QEventLoop L; L.processEvents(QEventLoop::ExcludeUserInputEvents);

    OscDelete(F);

    return false;
}
