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

#include "../ImportExport/ExportGPX.h"


ExportGPX::ExportGPX(MapDocument* doc)
 : IImportExport(doc)
{
}


ExportGPX::~ExportGPX()
{
}

// export
bool ExportGPX::export_(const QList<MapFeature *>& featList)
{
	QList<TrackPoint*>	waypoints;
	QList<TrackSegment*>	segments;
	QList<MapLayer*>	tracks;
	QList<Road*>	routes;

	if(! IImportExport::export_(featList) ) return false;

	bool OK = true;

	QDomDocument theXmlDoc;
	theXmlDoc.appendChild(theXmlDoc.createProcessingInstruction("xml", "version=\"1.0\""));

	QProgressDialog progress("Exporting GPX...", "Cancel", 0, 0);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMaximum(progress.maximum() + featList.count());

	QDomElement o = theXmlDoc.createElement("gpx");
	theXmlDoc.appendChild(o);
	o.setAttribute("version", "1.1");
	o.setAttribute("creator", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
	o.setAttribute("xmlns", "http://www.topografix.com/GPX/1/1");

	for (int i=0; i<theFeatures.size(); ++i) {
		if (TrackSegment* S = dynamic_cast<TrackSegment*>(theFeatures[i])) {
			segments.push_back(S);
			if (!tracks.contains(S->layer()))
				tracks.push_back(S->layer());
		} else
		if (TrackPoint* P = CAST_NODE(theFeatures[i])) {
			if (!P->tagValue("_waypoint_","").isEmpty())
				waypoints.push_back(P);
			if (!P->tagValue("name","").isEmpty() && !P->sizeParents())
				waypoints.push_back(P);
		} else
		if (Road* R = CAST_WAY(theFeatures[i])) {
			if (R->size())
				routes.push_back(R);
		}
	}

	for (int i=0; i < waypoints.size(); ++i) {
		waypoints[i]->toGPX(o, progress, true);
	}

	for (int i=0; i < routes.size(); ++i) {
		routes[i]->toGPX(o, progress, true);
	}

	for (int i=0; i<tracks.size(); ++i) {
		QDomElement t = o.ownerDocument().createElement("trk");
		o.appendChild(t);

		QDomElement n = o.ownerDocument().createElement("name");
		t.appendChild(n);
		QDomText v = o.ownerDocument().createTextNode(tracks[i]->name());
		n.appendChild(v);

		for (int j=0; j < segments.size(); ++j)
			if (tracks[i]->exists(segments[j]))
				segments[j]->toGPX(t, progress, true);
	}

	progress.setValue(progress.maximum());
	if (progress.wasCanceled())
		return false;

	Device->write(theXmlDoc.toString().toUtf8());
	return OK;
}

