//
// C++ Implementation: ImportExportKML
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QtGui>

#include "../ImportExport/ImportExportOSC.h"

#include "DirtyListExecutorOSC.h"

#include "DownloadOSM.h"

ImportExportOSC::ImportExportOSC(Document* doc)
 : IImportExport(doc)
{
}


ImportExportOSC::~ImportExportOSC()
{
}


// export
bool ImportExportOSC::export_(const QList<Feature *>&)
{
    DirtyListBuild Future;
    theDoc->history().buildDirtyList(Future);

    Future.resetUpdates();
    DirtyListExecutorOSC Exec(theDoc, Future);
    QString doc = Exec.getChanges();

    return (Device->write(doc.toUtf8()) != -1);
}

// IMPORT


// import the  input
bool ImportExportOSC::import(Layer* aLayer)
{
    QDomDocument* theXmlDoc = new QDomDocument();
    if (!theXmlDoc->setContent(Device)) {
//        QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid XML file.").arg(fn));
        Device->close();
        delete theXmlDoc;
        theXmlDoc = NULL;
        return false;
    }
    Device->close();

    QDomNodeList nl = theXmlDoc->elementsByTagName("osmChange");
    if (!nl.size()) {
//        QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid osmChange file.").arg(fn));
        delete theXmlDoc;
        theXmlDoc = NULL;
        return false;
    }

    CommandList* theList = new CommandList(MainWindow::tr("Import osmChange"), NULL);

    Layer* dLayer = theDoc->getLastDownloadLayer();
    if (!dLayer) {
        dLayer = new DrawingLayer(QApplication::translate("Downloader","%1 download").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
        theDoc->add(dLayer);
    }

    QList<QString> featIdList;
    Feature* F;
    QDomElement c = nl.at(0).toElement().firstChildElement();
    while (!c.isNull()) {
        if (c.tagName() == "create") {
            QDomElement f = c.firstChildElement();
            while (!f.isNull()) {
//                QString id = (f.hasAttribute("id") ? f.attribute("id") : f.attribute("xml:id"));
                if (f.tagName() == "node") {
                    F = Node::fromXML(theDoc, aLayer, f);
                    theList->add(new AddFeatureCommand(aLayer, F, true));
                } else if (f.tagName() == "way") {
                    F = Way::fromXML(theDoc, aLayer, f);
                    theList->add(new AddFeatureCommand(aLayer, F, true));
                } else if (f.tagName() == "relation") {
                    F = Relation::fromXML(theDoc, aLayer, f);
                    theList->add(new AddFeatureCommand(aLayer, F, true));
                }
                QList<QString> featIdList;
                for (int i=0; i<F->size(); ++i) {
                    if (F->get(i)->notEverythingDownloaded())
                        featIdList << F->get(i)->id();
                }

                f = f.nextSiblingElement();
            }
        } else if (c.tagName() == "modify") {
            QDomElement f = c.firstChildElement();
            while (!f.isNull()) {
                QString id = (f.hasAttribute("id") ? f.attribute("id") : f.attribute("xml:id"));
                if (f.tagName() == "node") {
                    F = theDoc->getFeature("node_"+id);
                    if (!F || F->notEverythingDownloaded())
                        downloadFeature(0, "node_"+id, theDoc, dLayer);
                    F = Node::fromXML(theDoc, aLayer, f);
                    theList->add(new AddFeatureCommand(aLayer, F, true));
                } else if (f.tagName() == "way") {
                    F = theDoc->getFeature("node_"+id);
                    if (!F || F->notEverythingDownloaded())
                        downloadFeature(0, "way_"+id, theDoc, dLayer);
                    F = Way::fromXML(theDoc, aLayer, f);
                    theList->add(new AddFeatureCommand(aLayer, F, true));
                } else if (f.tagName() == "relation") {
                    F = theDoc->getFeature("node_"+id);
                    if (!F || F->notEverythingDownloaded())
                        downloadFeature(0, "relation_"+id, theDoc, dLayer);
                    F = Relation::fromXML(theDoc, aLayer, f);
                    theList->add(new AddFeatureCommand(aLayer, F, true));
                }
                for (int i=0; i<F->size(); ++i) {
                    if (F->get(i)->notEverythingDownloaded())
                        featIdList << F->get(i)->id();
                }

                f = f.nextSiblingElement();
            }
        } else if (c.tagName() == "delete") {
            QDomElement f = c.firstChildElement();
            while (!f.isNull()) {
//                QString id = (f.hasAttribute("id") ? f.attribute("id") : f.attribute("xml:id"));
                if (f.tagName() == "node") {
                    Node* N = Node::fromXML(theDoc, aLayer, f);
                    theList->add(new RemoveFeatureCommand(theDoc, N));
                } else if (f.tagName() == "way") {
                    Way* W = Way::fromXML(theDoc, aLayer, f);
                    theList->add(new RemoveFeatureCommand(theDoc, W));
                } else if (f.tagName() == "relation") {
                    Relation* R = Relation::fromXML(theDoc, aLayer, f);
                    theList->add(new RemoveFeatureCommand(theDoc, R));
                }

                f = f.nextSiblingElement();
            }
        }

        c = c.nextSiblingElement();
    }
    downloadFeatures(0, featIdList, theDoc, dLayer);

    if (dLayer->size() == 0 && dLayer != theDoc->getLastDownloadLayer()) {
        theDoc->remove(dLayer);
        delete dLayer;
    }
    if (theList->empty()) {
        delete theList;
    } else {
        theDoc->addHistory(theList);
    }

    return true;
}

