#include "ImportOSM.h"

#include "Command.h"
#include "DocumentCommands.h"
#include "FeatureCommands.h"
#include "RelationCommands.h"
#include "DownloadOSM.h"
#include "Document.h"
#include "Layer.h"
#include "Features.h"

#include <QApplication>
#include <QtCore/QBuffer>
#include <QtCore/QDateTime>
#include <QtCore/QEventLoop>
#include <QtCore/QFile>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressBar>
#include <QtGui/QProgressDialog>
#include <QtXml/QDomDocument>
#include <QtXml/QXmlAttributes>


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
    QString ts = atts.value("timestamp");
    QDateTime time = QDateTime::fromString(ts.left(19), Qt::ISODate);
    QString user = atts.value("user");
    F->setTime(time);
    F->setUser(user);
    QString version = atts.value("version");
    if (!version.isEmpty())
        F->setVersionNumber(version.toInt());
}

void OSMHandler::parseNode(const QXmlAttributes& atts)
{
    double Lat = atts.value("lat").toDouble();
    double Lon = atts.value("lon").toDouble();
    QString id = atts.value("id");
    if (id[0] != '-' && id[0] != '{')
        id = "node_"+id;
    Node* Pt = dynamic_cast<Node*>(theDocument->getFeature(id));
    if (Pt)
    {
        if (Pt->lastUpdated() == Feature::User)
        {
            // conflict
            Pt->setLastUpdated(Feature::UserResolved);
            Node* userPt = Pt;
            Pt = new Node(Coord(angToInt(Lat),angToInt(Lon)));
            Pt->setId("conflict_"+id);
            Pt->setLastUpdated(Feature::OSMServerConflict);
            parseStandardAttributes(atts,Pt);
            if (Pt->time() > userPt->time() || Pt->versionNumber() != userPt->versionNumber()) {
                if (conflictLayer)
                    conflictLayer->add(Pt);
                userPt->setVersionNumber(Pt->versionNumber());
                NewFeature = true;
            } else {
                delete Pt;
                Pt = userPt;
                NewFeature = false;
            }
        }
        else if (Pt->lastUpdated() != Feature::UserResolved)
        {
            Pt->layer()->remove(Pt);
            theLayer->add(Pt);
            Pt->setPosition(Coord(angToInt(Lat),angToInt(Lon)));
            NewFeature = true;
            if (Pt->lastUpdated() == Feature::NotYetDownloaded)
                Pt->setLastUpdated(Feature::OSMServer);
        }
    }
    else
    {
        Pt = new Node(Coord(angToInt(Lat),angToInt(Lon)));
        theLayer->add(Pt);
        NewFeature = true;
        Pt->setId(id);
        Pt->setLastUpdated(Feature::OSMServer);
    }
    if (NewFeature) {
        parseStandardAttributes(atts,Pt);
        Current = Pt;
    } else
        Current = NULL;
}

void OSMHandler::parseNd(const QXmlAttributes& atts)
{
    Way* R = dynamic_cast<Way*>(Current);
    if (!R) return;
    Node *Part = Feature::getTrackPointOrCreatePlaceHolder(theDocument, theLayer, atts.value("ref"));
    if (NewFeature)
        R->add(Part);
}

void OSMHandler::parseWay(const QXmlAttributes& atts)
{
    QString id = atts.value("id");
    if (id[0] != '-' && id[0] != '{')
        id = "way_"+id;
    QString ts = atts.value("timestamp"); ts.truncate(19);
    Way* R = dynamic_cast<Way*>(theDocument->getFeature(id));
    if (R)
    {
        if (R->lastUpdated() == Feature::User)
        {
            R->setLastUpdated(Feature::UserResolved);
            NewFeature = false;
            // conflict
            Way* userRd = R;
            R = new Way();
            R->setId("conflict_"+id);
            R->setLastUpdated(Feature::OSMServerConflict);
            parseStandardAttributes(atts,R);
            if (R->time() > userRd->time() || R->versionNumber() != userRd->versionNumber()) {
                if (conflictLayer)
                    conflictLayer->add(R);
                userRd->setVersionNumber(R->versionNumber());
                NewFeature = true;
            } else {
                delete R;
                R = userRd;
                NewFeature = false;
            }
        }
        else if (R->lastUpdated() != Feature::UserResolved)
        {
            R->layer()->remove(R);
            theLayer->add(R);
            while (R->size())
                R->remove((int)0);
            NewFeature = true;
            if (R->lastUpdated() == Feature::NotYetDownloaded)
                R->setLastUpdated(Feature::OSMServer);
        }
    }
    else
    {
        R = new Way;
        theLayer->add(R);
        NewFeature = true;
        R->setId(id);
        R->setLastUpdated(Feature::OSMServer);
    }
    if (NewFeature) {
        parseStandardAttributes(atts,R);
        Current = R;
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
        F = Feature::getTrackPointOrCreatePlaceHolder(theDocument, theLayer, atts.value("ref"));
    else if (Type == "way")
        F = Feature::getWayOrCreatePlaceHolder(theDocument, theLayer, atts.value("ref"));
    else if (Type == "relation")
        F = Feature::getRelationOrCreatePlaceHolder(theDocument, theLayer, atts.value("ref"));

    if (F && F != R)
        R->add(atts.value("role"),F);
}

void OSMHandler::parseRelation(const QXmlAttributes& atts)
{
    QString id = atts.value("id");
    if (id[0] != '-' && id[0] != '{')
        id = "rel_"+id;
    QString ts = atts.value("timestamp"); ts.truncate(19);
    Relation* R = dynamic_cast<Relation*>(theDocument->getFeature(id));
    if (R)
    {
        if (R->lastUpdated() == Feature::User)
        {
            R->setLastUpdated(Feature::UserResolved);
            NewFeature = false;
            // conflict
            Relation* userR = R;
            R = new Relation();
            R->setId("conflict_"+id);
            R->setLastUpdated(Feature::OSMServerConflict);
            parseStandardAttributes(atts,R);
            if (R->time() > userR->time() || R->versionNumber() != userR->versionNumber()) {
                if (conflictLayer)
                    conflictLayer->add(R);
                userR->setVersionNumber(R->versionNumber());
                NewFeature = true;
            } else {
                delete R;
                R = userR;
                NewFeature = false;
            }
        }
        else if (R->lastUpdated() != Feature::UserResolved)
        {
            R->layer()->remove(R);
            theLayer->add(R);
            while (R->size())
                R->remove((int)0);
            NewFeature = true;
            if (R->lastUpdated() == Feature::NotYetDownloaded)
                R->setLastUpdated(Feature::OSMServer);
        }
    }
    else
    {
        R = new Relation;
        NewFeature = true;
        R->setId(id);
        theLayer->add(R);
        R->setLastUpdated(Feature::OSMServer);
    }
    if (NewFeature) {
        parseStandardAttributes(atts,R);
        Current = R;
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
        QString URL = theDownloader->getURLToFetchFull(Resolution[i]);
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
        recurseDelete(F->getParent(i), MustDelete);
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

    IProgressWindow* aProgressWindow = dynamic_cast<IProgressWindow*>(aParent);
    if (!aProgressWindow)
        return false;

    QProgressDialog* dlg = aProgressWindow->getProgressDialog();
    if (dlg)
        dlg->setWindowTitle(QApplication::translate("Downloader", "Parsing..."));

    QProgressBar* Bar = aProgressWindow->getProgressBar();
    Bar->setTextVisible(false);

    QLabel* Lbl = aProgressWindow->getProgressLabel();
    Lbl->setText(QApplication::translate("Downloader","Parsing XML"));

    if (dlg)
        dlg->show();

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
    Bar->setMaximum(File.size());
    Bar->setValue(Bar->value()+buf.size());

    theLayer->blockVirtualUpdates(true);
    while (!File.atEnd())
    {
        QByteArray buf(File.read(20480));
        source.setData(buf);
        xmlReader.parseContinue();
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

    theLayer->blockVirtualUpdates(false);

    if (WasCanceled)
    {
        theDocument->remove(conflictLayer);
        delete conflictLayer;
        return false;
    }
    else
    {
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

        if (M_PREFS->getUseVirtualNodes()) {
            Lbl->setText(QApplication::translate("Downloader","Update virtuals"));
            Bar->setMaximum(theLayer->size());
            Bar->setValue(0);
        }

        // Check for empty Roads/Relations and update virtual nodes
        QList<Feature*> EmptyFeature;
        for (int i=0; i<theLayer->size(); ++i) {
            if (!theLayer->get(i)->notEverythingDownloaded()) {
                if (!theLayer->get(i)->size() && !CAST_NODE(theLayer->get(i)))
                        EmptyFeature.push_back(theLayer->get(i));
                if (M_PREFS->getUseVirtualNodes()) {
                    if (Way* w = CAST_WAY(theLayer->get(i)))
                        w->updateVirtuals();
                    Bar->setValue(i);
                    qApp->processEvents();
                }
            }
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



