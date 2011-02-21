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
    QByteArray doc = Exec.getChanges();

    return (Device->write(doc) != -1);
}

// IMPORT


// import the  input
bool ImportExportOSC::import(Layer* aLayer)
{
    QXmlStreamReader stream(Device);

    while (stream.readNext() && stream.tokenType() != QXmlStreamReader::Invalid && stream.tokenType() != QXmlStreamReader::StartElement);
    if (stream.name() != "osmChange" && stream.name() != "osmchange") {
//        QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid osmChange file.").arg(fn));
        return false;
    }

    CommandList* theList = new CommandList(QApplication::tr("Import osmChange"), NULL);

    Layer* dLayer = theDoc->getLastDownloadLayer();
    if (!dLayer) {
        dLayer = new DrawingLayer(QApplication::translate("Downloader","%1 download").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
        theDoc->add(dLayer);
    }

    QList<IFeature::FId> featIdList;
    Feature* F;
    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "create") {
            stream.readNext();
            while(!stream.atEnd() && !stream.isEndElement()) {
                if (stream.name() == "node") {
                    F = Node::fromXML(theDoc, aLayer, stream);
                    theList->add(new AddFeatureCommand(aLayer, F, true));
                } else if (stream.name() == "way") {
                    F = Way::fromXML(theDoc, aLayer, stream);
                    theList->add(new AddFeatureCommand(aLayer, F, true));
                } else if (stream.name() == "relation") {
                    F = Relation::fromXML(theDoc, aLayer, stream);
                    theList->add(new AddFeatureCommand(aLayer, F, true));
                } else if (!stream.isWhitespace()) {
                    qDebug() << "OSC: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
                    stream.skipCurrentElement();
                }
                if (F) {
                    for (int i=0; i<F->size(); ++i) {
                        if (F->get(i)->notEverythingDownloaded() && F->get(i)->hasOSMId())
                            featIdList << F->get(i)->id();
                    }
                }

                stream.readNext();
            }
        } else if (stream.name() == "modify") {
            stream.readNext();
            while(!stream.atEnd() && !stream.isEndElement()) {
                QString sid = (stream.attributes().hasAttribute("id") ? stream.attributes().value("id").toString() : stream.attributes().value("xml:id").toString());
                if (stream.name() == "node") {
                    IFeature::FId id(Feature::Point, sid.toLongLong());
                    F = theDoc->getFeature(id);
                    if (!F || F->notEverythingDownloaded())
                        downloadFeature(0, id, theDoc, dLayer);
                    F = Node::fromXML(theDoc, aLayer, stream);
                    theList->add(new AddFeatureCommand(aLayer, F, true));
                } else if (stream.name() == "way") {
                    IFeature::FId id(Feature::LineString, sid.toLongLong());
                    F = theDoc->getFeature(id);
                    if (!F || F->notEverythingDownloaded())
                        downloadFeature(0, id, theDoc, dLayer);
                    F = Way::fromXML(theDoc, aLayer, stream);
                    theList->add(new AddFeatureCommand(aLayer, F, true));
                } else if (stream.name() == "relation") {
                    IFeature::FId id(Feature::OsmRelation, sid.toLongLong());
                    F = theDoc->getFeature(id);
                    if (!F || F->notEverythingDownloaded())
                        downloadFeature(0, id, theDoc, dLayer);
                    F = Relation::fromXML(theDoc, aLayer, stream);
                    theList->add(new AddFeatureCommand(aLayer, F, true));
                } else if (!stream.isWhitespace()) {
                    qDebug() << "OSC: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
                    stream.skipCurrentElement();
                }
                if (F) {
                    for (int i=0; i<F->size(); ++i) {
                        if (F->get(i)->notEverythingDownloaded() && F->get(i)->hasOSMId())
                            featIdList << F->get(i)->id();
                    }
                }
                stream.readNext();
            }
        } else if (stream.name() == "delete") {
            stream.readNext();
            while(!stream.atEnd() && !stream.isEndElement()) {
                if (stream.name() == "node") {
                    Node* N = Node::fromXML(theDoc, aLayer, stream);
                    theList->add(new RemoveFeatureCommand(theDoc, N));
                } else if (stream.name() == "way") {
                    Way* W = Way::fromXML(theDoc, aLayer, stream);
                    theList->add(new RemoveFeatureCommand(theDoc, W));
                } else if (stream.name() == "relation") {
                    Relation* R = Relation::fromXML(theDoc, aLayer, stream);
                    theList->add(new RemoveFeatureCommand(theDoc, R));
                } else if (!stream.isWhitespace()) {
                    qDebug() << "OSC: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
                    stream.skipCurrentElement();
                }
                stream.readNext();
            }
        }

        stream.readNext();
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

