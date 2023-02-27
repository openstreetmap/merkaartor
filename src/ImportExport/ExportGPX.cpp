//
// C++ Implementation: ExportGPX
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QtGui>
#include <QApplication>

#include "../ImportExport/ExportGPX.h"


ExportGPX::ExportGPX(Document* doc)
 : IImportExport(doc)
{
}


ExportGPX::~ExportGPX()
{
}

// export
bool ExportGPX::export_(const QList<Feature *>& featList)
{
    QList<Node*>	waypoints;
    QList<TrackSegment*>	segments;
    QList<Layer*>	tracks;
    QList<Way*>	routes;

    if(! IImportExport::export_(featList) ) return false;

    bool OK = true;

    QXmlStreamWriter stream(Device);
    stream.setAutoFormatting(true);
    stream.setAutoFormattingIndent(2);
    stream.writeStartDocument();

    QProgressDialog progress(QApplication::tr("Exporting GPX..."), QApplication::tr("Cancel"), 0, 0);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMaximum(progress.maximum() + featList.count());

    stream.writeStartElement("gpx");
    stream.writeAttribute("version", "1.1");
    stream.writeAttribute("creator", QString("%1 v%2%3").arg(BuildMetadata::PRODUCT).arg(BuildMetadata::VERSION).arg(BuildMetadata::REVISION));
    stream.writeAttribute("xmlns", "http://www.topografix.com/GPX/1/1");

    for (int i=0; i<theFeatures.size(); ++i) {
        if (TrackSegment* S = dynamic_cast<TrackSegment*>(theFeatures[i])) {
            segments.push_back(S);
            if (!tracks.contains(S->layer()))
                tracks.push_back(S->layer());
        } else
        if (Node* P = CAST_NODE(theFeatures[i])) {
            if (!P->tagValue("_waypoint_","").isEmpty())
                waypoints.push_back(P);
            if (!P->tagValue("name","").isEmpty() && !P->sizeParents())
                waypoints.push_back(P);
        } else
        if (Way* R = CAST_WAY(theFeatures[i])) {
            if (R->size())
                routes.push_back(R);
        }
    }

    for (int i=0; i < waypoints.size(); ++i) {
        waypoints[i]->toGPX(stream, &progress, "wpt", true);
    }

    for (int i=0; i < routes.size(); ++i) {
        routes[i]->toGPX(stream, &progress, true);
    }

    for (int i=0; i<tracks.size(); ++i) {
        stream.writeStartElement("trk");
        stream.writeTextElement("name", tracks[i]->name());

        for (int j=0; j < segments.size(); ++j)
            if (tracks[i]->exists(segments[j]))
                segments[j]->toGPX(stream, &progress, true);
        stream.writeEndElement();
    }
    stream.writeEndElement();
    stream.writeEndDocument();

    progress.setValue(progress.maximum());
    if (progress.wasCanceled())
        return false;

    return OK;
}

