#include "ExportOSM.h"

#include "Map/Relation.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"

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

QString exportOSM(const Road& R)
{
	QString S;
	S += QString("<way id=\"%1\">").arg(stripToOSMId(R.id()));
	for (unsigned int i=0; i<R.size(); ++i)
		S+=QString("<nd ref=\"%1\"/>").arg(stripToOSMId(R.get(i)->id()));
	S += tagOSM(R);
	S += "</way>";
	return S;
}

QString exportOSM(const Relation& R)
{
	QString S;
	S += QString("<relation id=\"%1\">").arg(stripToOSMId(R.id()));
	for (unsigned int i=0; i<R.size(); ++i)
	{
		QString Type("node");
		if (dynamic_cast<const Road*>(R.get(i)))
			Type="way";
		else if (dynamic_cast<const Relation*>(R.get(i)))
			Type="relation";
		S+=QString("<member type=\"%1\" ref=\"%2\" role=\"%3\"/>").arg(Type).arg(stripToOSMId(R.get(i)->id())).arg(R.getRole(i));
	}
	S += tagOSM(R);
	S += "</relation>";
	return S;
}


QString wrapOSM(const QString& S, const QString& ChangeSetId)
{
	if (ChangeSetId.isEmpty())
		return "<osm version=\"0.5\">"+S+"</osm>"+QChar(0);
	else
		return "<osm version=\"0.6\" changeset=\""+ChangeSetId+"\">"+S+"</osm>"+QChar(0);
}


