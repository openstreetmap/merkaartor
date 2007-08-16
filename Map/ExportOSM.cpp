#include "ExportOSM.h"

#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Map/Way.h"

static QString stripToOSMId(const QString& id)
{
	int f = id.lastIndexOf("_");
	if (f>0)
		return id.right(id.length()-(f+1));
	return id;
}

static QString tagOSM(const MapFeature& F)
{
	QString S;
	for (unsigned int i=0; i<F.tagSize(); ++i)
	{
		if (F.tagKey(i) == "width") continue;
		S += QString("<tag k=\"%1\" v=\"%2\"/>").arg(F.tagKey(i)).arg(F.tagValue(i));
	}
	return S;
}

QString exportOSM(const TrackPoint& Pt)
{
	QString S("<node id=\"%1\" lat=\"%2\" lon=\"%3\">");
	S+=tagOSM(Pt);
	S+="</node>";
	return S.arg(stripToOSMId(Pt.id())).arg(radToAng(Pt.position().lat()),0,'f',8).arg(radToAng(Pt.position().lon()),0,'f',8);
}

QString exportOSM(const Way& W)
{
	QString S;
	S += QString("<segment id=\"%1\" from=\"%2\" to=\"%3\">").arg(stripToOSMId(W.id())).arg(stripToOSMId(W.from()->id())).arg(stripToOSMId(W.to()->id()));
	S += tagOSM(W);
	S += "</segment>";
	return S;
}

QString exportOSM(const Road& R)
{
	QString S;
	S += QString("<way id=\"%1\">").arg(stripToOSMId(R.id()));
	for (unsigned int i=0; i<R.size(); ++i)
		S+=QString("<seg id=\"%1\"/>").arg(stripToOSMId(R.get(i)->id()));
	S += tagOSM(R);
	S += "</way>";
	return S;
}

QString wrapOSM(const QString& S)
{
	return "<osm version=\"0.3\">"+S+"</osm>"+QChar(0);
}


