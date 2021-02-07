#include "ImportOSM.h"

#include "Command.h"
#include "DocumentCommands.h"
#include "FeatureCommands.h"
#include "RelationCommands.h"
#include "DownloadOSM.h"
#include "Document.h"
#include "Layer.h"
#include "Features.h"
#include "Global.h"
#include "IProgressWindow.h"

#include <QApplication>
#include <QBuffer>
#include <QDateTime>
#include <QEventLoop>
#include <QFile>
#include <QMessageBox>
#include <QProgressBar>
#include <QProgressDialog>
#include <QDomDocument>
#include <QXmlAttributes>


OSMHandler::OSMHandler(Document* aDoc, Layer* aLayer, Layer* aConflict)
: theDocument(aDoc), theLayer(aLayer), conflictLayer(aConflict), Current(0)
{
}

void OSMHandler::parseTag(const QXmlAttributes &atts)
{
    if (!Current) return;

    Current->setTag(atts.value("k"),atts.value("v"));
}

void parseStandardAttributes(const QXmlAttributes& atts, Feature* F)
{
#ifndef FRISIUS_BUILD
    QString ts = atts.value("timestamp");
    QDateTime time = QDateTime::fromString(ts.left(19), Qt::ISODate);
    if (!time.isValid())
        time = QDateTime::currentDateTime();
    QString user = atts.value("user");
    F->setTime(time);
    F->setUser(user);
    QString version = atts.value("version");
    if (!version.isEmpty())
        F->setVersionNumber(version.toInt());
#endif
}

void OSMHandler::parseNode(const QXmlAttributes& atts)
{
    qreal Lat = atts.value("lat").toDouble();
    qreal Lon = atts.value("lon").toDouble();
    QString id = atts.value("id");
    Node* Pt = CAST_NODE(theDocument->getFeature(IFeature::FId(IFeature::Point, id.toLongLong())));
    if (Pt)
    {
        Node* userPt = Pt;
        Pt = g_backend.allocNode(theLayer, Coord(Lon,Lat));
        Pt->setId(IFeature::FId(IFeature::Point | IFeature::Conflict, id.toLongLong()));
        Pt->setLastUpdated(Feature::OSMServerConflict);
        parseStandardAttributes(atts,Pt);

        if (userPt->lastUpdated() == Feature::User)
        {
            // conflict
            userPt->setLastUpdated(Feature::UserResolved);
#ifndef FRISIUS_BUILD
            if (Pt->time() > userPt->time() || Pt->versionNumber() != userPt->versionNumber()) {
                if (conflictLayer)
                    conflictLayer->add(Pt);
                userPt->setVersionNumber(Pt->versionNumber());
                NewFeature = true;
            } else
#endif
            {
                g_backend.deallocFeature(theLayer, Pt);
                Pt = userPt;
                NewFeature = false;
            }
        }
        else if (userPt->lastUpdated() != Feature::UserResolved)
        {
#ifndef FRISIUS_BUILD
            if (userPt->lastUpdated() == Feature::NotYetDownloaded || (Pt->time() > userPt->time() || Pt->versionNumber() != userPt->versionNumber()))
#else
            if (userPt->lastUpdated() == Feature::NotYetDownloaded)
#endif
            {
                g_backend.deallocFeature(theLayer, Pt);
                Pt = userPt;
                Pt->layer()->remove(Pt);
                theLayer->add(Pt);
                Pt->setPosition(Coord(Lon,Lat));
                Pt->clearTags();
                NewFeature = true;
                if (Pt->lastUpdated() == Feature::NotYetDownloaded)
                    Pt->setLastUpdated(Feature::OSMServer);
            } else {
                g_backend.deallocFeature(theLayer, Pt);
                Pt = userPt;
                NewFeature = false;
            }
        } else {
            qDebug() << "Node conflicted, but already is tagged as Feature::UserResolved. Ignoring" << Pt->xmlId();
            g_backend.deallocFeature(theLayer, Pt);
            Pt = userPt;
            NewFeature = false;
        }
    }
    else
    {
        Pt = g_backend.allocNode(theLayer, Coord(Lon,Lat));
        Pt->setId(IFeature::FId(IFeature::Point, id.toLongLong()));
        Pt->setLastUpdated(Feature::OSMServer);
        theLayer->add(Pt);
        NewFeature = true;
    }

    if (NewFeature) {
        parseStandardAttributes(atts,Pt);
        Current = Pt;
        for (int i=0; i<Pt->sizeParents(); ++i) {
            if (Pt->getParent(i)->isDeleted()) continue;
            if (Way* w = CAST_WAY(Pt->getParent(i)))
                touchedWays << w;
        }
    } else
        Current = NULL;
}

void OSMHandler::parseNd(const QXmlAttributes& atts)
{
    Way* R = dynamic_cast<Way*>(Current);
    if (!R) return;
    Node *Part = Feature::getNodeOrCreatePlaceHolder(theDocument, theLayer, IFeature::FId(IFeature::Point, atts.value("ref").toLongLong()));
    if (NewFeature)
        R->add(Part);
}

void OSMHandler::parseWay(const QXmlAttributes& atts)
{
    QString id = atts.value("id");
    QString ts = atts.value("timestamp"); ts.truncate(19);
    Way* R = CAST_WAY(theDocument->getFeature(IFeature::FId(IFeature::LineString, id.toLongLong())));
    if (R)
    {
        Way* userRd = R;
        R = g_backend.allocWay(theLayer);
        R->setId(IFeature::FId(IFeature::LineString | IFeature::Conflict, id.toLongLong()));
        R->setLastUpdated(Feature::OSMServerConflict);
        parseStandardAttributes(atts,R);

        if (userRd->lastUpdated() == Feature::User)
        {
            userRd->setLastUpdated(Feature::UserResolved);
            NewFeature = false;
            // conflict
#ifndef FRISIUS_BUILD
            if (R->time() > userRd->time() || R->versionNumber() != userRd->versionNumber()) {
                if (conflictLayer)
                    conflictLayer->add(R);
                userRd->setVersionNumber(R->versionNumber());
                NewFeature = true;
            } else
#endif
            {
                g_backend.deallocFeature(theLayer, R);
                R = userRd;
                NewFeature = false;
            }
        }
        else if (R->lastUpdated() != Feature::UserResolved)
        {
#ifndef FRISIUS_BUILD
            if (userRd->lastUpdated() == Feature::NotYetDownloaded || (R->time() > userRd->time() || R->versionNumber() != userRd->versionNumber())) {
#else
            if (userRd->lastUpdated() == Feature::NotYetDownloaded) {
#endif
                g_backend.deallocFeature(theLayer, R);
                R = userRd;
                R->layer()->remove(R);
                theLayer->add(R);
                while (R->size())
                    R->remove((int)0);
                R->clearTags();
                NewFeature = true;
                if (R->lastUpdated() == Feature::NotYetDownloaded)
                    R->setLastUpdated(Feature::OSMServer);
            } else {
                g_backend.deallocFeature(theLayer, R);
                R = userRd;
                NewFeature = false;
            }
        }
    }
    else
    {
        R = g_backend.allocWay(theLayer);
        R->setId(IFeature::FId(IFeature::LineString, id.toLongLong()));
        R->setLastUpdated(Feature::OSMServer);
        theLayer->add(R);
        NewFeature = true;
    }

    if (NewFeature) {
        parseStandardAttributes(atts,R);
        Current = R;
        touchedWays << R;
    } else
        Current = NULL;
}

void OSMHandler::parseMember(const QXmlAttributes& atts)
{
    Relation* R = dynamic_cast<Relation*>(Current);
    if (!R)
        return;
    QString Type = atts.value("type");
    Feature* F = 0;
    if (Type == "node")
        F = Feature::getNodeOrCreatePlaceHolder(theDocument, theLayer, IFeature::FId(IFeature::Point, atts.value("ref").toLongLong()));
    else if (Type == "way")
        F = Feature::getWayOrCreatePlaceHolder(theDocument, theLayer, IFeature::FId(IFeature::LineString, atts.value("ref").toLongLong()));
    else if (Type == "relation")
        F = Feature::getRelationOrCreatePlaceHolder(theDocument, theLayer, IFeature::FId(IFeature::OsmRelation, atts.value("ref").toLongLong()));

    if (F && F != R)
        R->add(atts.value("role"),F);
}

void OSMHandler::parseRelation(const QXmlAttributes& atts)
{
    QString id = atts.value("id");
    QString ts = atts.value("timestamp"); ts.truncate(19);
    Relation* R = CAST_RELATION(theDocument->getFeature(IFeature::FId(IFeature::OsmRelation, id.toLongLong())));
    if (R)
    {
        Relation* userR = R;
        R = g_backend.allocRelation(theLayer);
        R->setId(IFeature::FId(IFeature::OsmRelation | IFeature::Conflict, id.toLongLong()));
        R->setLastUpdated(Feature::OSMServerConflict);
        parseStandardAttributes(atts,R);

        if (R->lastUpdated() == Feature::User)
        {
            R->setLastUpdated(Feature::UserResolved);
            NewFeature = false;
            // conflict
#ifndef FRISIUS_BUILD
            if (R->time() > userR->time() || R->versionNumber() != userR->versionNumber()) {
                if (conflictLayer)
                    conflictLayer->add(R);
                userR->setVersionNumber(R->versionNumber());
                NewFeature = true;
            } else
#endif
            {
                g_backend.deallocFeature(theLayer, R);
                R = userR;
                NewFeature = false;
            }
        }
        else if (R->lastUpdated() != Feature::UserResolved)
        {
#ifndef FRISIUS_BUILD
            if (userR->lastUpdated() == Feature::NotYetDownloaded || (R->time() > userR->time() || R->versionNumber() != userR->versionNumber())) {
#else
            if (userR->lastUpdated() == Feature::NotYetDownloaded) {
#endif
                g_backend.deallocFeature(theLayer, R);
                R = userR;
                R->layer()->remove(R);
                theLayer->add(R);
                while (R->size())
                    R->remove((int)0);
                R->clearTags();
                NewFeature = true;
                if (R->lastUpdated() == Feature::NotYetDownloaded)
                    R->setLastUpdated(Feature::OSMServer);
            } else {
                g_backend.deallocFeature(theLayer, R);
                R = userR;
                NewFeature = false;
            }
        }
    }
    else
    {
        R = g_backend.allocRelation(theLayer);
        R->setId(IFeature::FId(IFeature::OsmRelation, id.toLongLong()));
        R->setLastUpdated(Feature::OSMServer);
        NewFeature = true;
        theLayer->add(R);
    }

    if (NewFeature) {
        parseStandardAttributes(atts,R);
        Current = R;
        touchedRelations << R;
    } else
        Current = NULL;
}

bool OSMHandler::startElement ( const QString &, const QString & /* localName */, const QString & qName, const QXmlAttributes & atts )
{
    if (qName == "tag")
        parseTag(atts);
    else if (qName == "node")
        parseNode(atts);
    else if (qName == "nd")
        parseNd(atts);
    else if (qName == "way")
        parseWay(atts);
    else if (qName == "member")
        parseMember(atts);
    else if (qName == "relation")
        parseRelation(atts);
    return true;
}

bool OSMHandler::endElement ( const QString &, const QString & /* localName */, const QString & qName )
{
    if (qName == "node")
        Current = 0;
    return true;
}

static bool downloadToResolve(const QList<Feature*>& Resolution, QWidget* aParent, Document* theDocument, Layer* theLayer, Downloader* theDownloader)
{
    IProgressWindow* aProgressWindow = dynamic_cast<IProgressWindow*>(aParent);
    if (!aProgressWindow)
        return false;

    QProgressDialog* dlg = aProgressWindow->getProgressDialog();
    dlg->setWindowTitle(QApplication::translate("Downloader", "Downloading unresolved..."));
    QProgressBar* Bar = aProgressWindow->getProgressBar();
    QLabel* Lbl = aProgressWindow->getProgressLabel();

    for (int i=0; i<Resolution.size(); i++ )
    {
        QUrl URL = M_PREFS->getOsmApiUrl()+theDownloader->getURLToFetchFull(Resolution[i]);
        Lbl->setText(QApplication::translate("Downloader","Downloading unresolved %1 of %2").arg(i+1).arg(Resolution.size()));
        if (theDownloader->go(URL))
        {
            if (theDownloader->resultCode() == 410) {
                theLayer->remove(Resolution[i]);
                delete Resolution[i];
            }
            else
            {
                Lbl->setText(QApplication::translate("Downloader","Parsing unresolved %1 of %2").arg(i+1).arg(Resolution.size()));

                QByteArray ba(theDownloader->content());
                QBuffer  File(&ba);
                File.open(QIODevice::ReadOnly);

                OSMHandler theHandler(theDocument,theLayer,NULL);

                QXmlSimpleReader xmlReader;
                xmlReader.setContentHandler(&theHandler);
                QXmlInputSource source;
                QByteArray buf(File.read(10240));
                source.setData(buf);
                xmlReader.parse(&source,true);
                while (!File.atEnd())
                {
                    QByteArray buf(File.read(20480));
                    source.setData(buf);
                    xmlReader.parseContinue();
                    qApp->processEvents();
                    if (dlg && dlg->wasCanceled())
                        break;
                }
            }
            Resolution[i]->setLastUpdated(Feature::OSMServer);
        }
        else
            return false;
        Bar->setValue(i);
    }
    return true;
}

static void recurseDelete (Feature* F, QList<Feature*>& MustDelete)
{
    for (int i=0; i<F->sizeParents(); i++) {
        recurseDelete(CAST_FEATURE(F->getParent(i)), MustDelete);
    }
    if (!MustDelete.contains(F))
        MustDelete.push_back(F);
}

static bool resolveNotYetDownloaded(QWidget* aParent, Document* theDocument, Layer* theLayer, Downloader* theDownloader)
{
    // resolving nodes and roads makes no sense since the OSM api guarantees that they will be all downloaded,
    //  so only resolve for relations if the ResolveRelations pref is set
    if (theDownloader)
    {
        IProgressWindow* aProgressWindow = dynamic_cast<IProgressWindow*>(aParent);
        if (!aProgressWindow)
            return false;

        QProgressBar* Bar = aProgressWindow->getProgressBar();
        //QLabel* Lbl = aProgressWindow->getProgressLabel();

        QList<Feature*> MustResolve;
        MustResolve.clear();
        for (FeatureIterator it(theDocument); !it.isEnd(); ++it)
        {
            Relation* RR = CAST_RELATION(it.get());
            if (RR && RR->notEverythingDownloaded())
                MustResolve.push_back(RR);
        }
        if (MustResolve.size())
        {
            Bar->setMaximum(MustResolve.size());
            Bar->setValue(0);
            if (!downloadToResolve(MustResolve,aParent,theDocument,theLayer, theDownloader))
                return false;
        }
    }
    return true;
}

static bool deleteIncompleteRelations(QWidget* /*aParent*/, Document* /*theDocument*/, Layer* theLayer, Downloader* /*theDownloader*/)
{
    QList<Feature*> MustDelete;
    for (int i=0; i<theLayer->size(); i++)
    {
        if (theLayer->get(i)->notEverythingDownloaded()) {
            recurseDelete(theLayer->get(i), MustDelete);
        }
    }
    for (int i=0; i<MustDelete.size(); i++) {
        MustDelete[i]->layer()->remove(MustDelete[i]);
        delete MustDelete[i];
    }
    return true;
}

bool importOSM(QWidget* aParent, QIODevice& File, Document* theDocument, Layer* theLayer, Downloader* theDownloader)
{
    QDomDocument DomDoc;
    QString ErrorStr;
    /* int ErrorLine; */
    /* int ErrorColumn; */

    QProgressDialog* dlg = NULL;
    QProgressBar* Bar = NULL;
    QLabel* Lbl = NULL;
    IProgressWindow* aProgressWindow = dynamic_cast<IProgressWindow*>(aParent);
    if (aProgressWindow) {
        dlg = aProgressWindow->getProgressDialog();
        if (dlg) {
            dlg->setWindowTitle(QApplication::translate("Downloader", "Parsing..."));

            Bar = aProgressWindow->getProgressBar();
            Bar->setTextVisible(false);

            Lbl = aProgressWindow->getProgressLabel();
            Lbl->setText(QApplication::translate("Downloader","Parsing XML"));

            dlg->show();
        }
    }

    if (theDownloader)
        theDownloader->setAnimator(dlg,Lbl,Bar,false);
    Layer* conflictLayer = new DrawingLayer(QApplication::translate("Downloader","Conflicts from %1").arg(theLayer->name()));
    theDocument->add(conflictLayer);

    OSMHandler theHandler(theDocument,theLayer,conflictLayer);

    QXmlSimpleReader xmlReader;
    xmlReader.setContentHandler(&theHandler);
    QXmlInputSource source;
    QByteArray buf(File.read(10240));
    source.setData(buf);
    xmlReader.parse(&source,true);
    if (Bar) {
        Bar->setMaximum(File.size());
        Bar->setValue(Bar->value()+buf.size());
    }

    while (!File.atEnd())
    {
        QByteArray buf(File.read(20480));
        source.setData(buf);
        xmlReader.parseContinue();
        if (Bar)
            Bar->setValue(Bar->value()+buf.size());
        qApp->processEvents();
        if (dlg && dlg->wasCanceled())
            break;
    }

    bool WasCanceled = false;
    if (dlg)
        WasCanceled = dlg->wasCanceled();
    if (!WasCanceled && M_PREFS->getResolveRelations())
        WasCanceled = !resolveNotYetDownloaded(aParent,theDocument,theLayer,theDownloader);
    if (!WasCanceled && M_PREFS->getDeleteIncompleteRelations())
        WasCanceled = !deleteIncompleteRelations(aParent,theDocument,theLayer,theDownloader);

    if (WasCanceled)
    {
        theDocument->remove(conflictLayer);
        delete conflictLayer;
        return false;
    }
    else
    {
//        if (M_PREFS->getUseVirtualNodes()) {
//            if (dlg) {
//                Lbl->setText(QApplication::translate("Downloader","Update virtuals"));
//                Bar->setMaximum(theHandler.touchedWays.size());
//                Bar->setValue(0);
//            }
//            foreach (Way* w, theHandler.touchedWays) {
//                w->updateVirtuals();
//                if (Bar)
//                    Bar->setValue(Bar->value()+1);
//                qApp->processEvents();
//            }
//        }

        // Check for empty Roads/Relations and update virtual nodes
        QList<Feature*> EmptyFeature;
        foreach (Way* w, theHandler.touchedWays) {
            if (!w->size())
                EmptyFeature.push_back(w);
        }
        foreach (Relation* r, theHandler.touchedRelations) {
            if (!r->size())
                EmptyFeature.push_back(r);
        }

        if (EmptyFeature.size()) {
            if (QMessageBox::warning(aParent,QApplication::translate("Downloader","Empty roads/relations detected"),
                    QApplication::translate("Downloader",
                    "Empty roads/relations are probably errors.\n"
                    "Do you want to mark them for deletion?"),
                    QMessageBox::Ok | QMessageBox::Cancel,
                    QMessageBox::Cancel
                    ) == QMessageBox::Ok) {
                for (int i=0; i<EmptyFeature.size(); i++ ) {
                    CommandList* emptyFeatureList = new CommandList();
                    emptyFeatureList->setDescription(QApplication::translate("Downloader","Remove empty feature %1").arg(EmptyFeature[i]->description()));
                    emptyFeatureList->setFeature(EmptyFeature[i]);
                    emptyFeatureList->add(new RemoveFeatureCommand(theDocument, EmptyFeature[i]));
                    theDocument->addHistory(emptyFeatureList);
                }
            }
        }

        if (!conflictLayer->size()) {
            theDocument->remove(conflictLayer);
            delete conflictLayer;
        } else {
            QMessageBox::warning(aParent,QApplication::translate("Downloader","Conflicts have been detected"),
                QApplication::translate("Downloader",
                "This means that some of the feature you modified"
                " since your last download have since been modified by someone else on the server.\n"
                "The features have been duplicated as \"conflict_...\" on the \"Conflicts...\" layer.\n"
                "Before being able to upload your changes, you will have to manually merge the two versions"
                " and remove the one from the \"Conflicts...\" layer."
                ));
        }

    }
    return true;
}

bool importOSM(QWidget* aParent, const QString& aFilename, Document* theDocument, Layer* theLayer)
{
    QFile File(aFilename);
    if (!File.open(QIODevice::ReadOnly))
         return false;
    return importOSM(aParent, File, theDocument, theLayer, 0 );
}

bool importOSM(QWidget* aParent, QByteArray& Content, Document* theDocument, Layer* theLayer, Downloader* theDownloader)
{
    QBuffer File(&Content);
    File.open(QIODevice::ReadOnly);
    return importOSM(aParent, File, theDocument, theLayer, theDownloader);
}



