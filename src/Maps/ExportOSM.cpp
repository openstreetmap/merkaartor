#include "ExportOSM.h"

#include "Maps/Relation.h"
#include "Maps/Road.h"
#include "Maps/TrackPoint.h"

#include "Preferences/MerkaartorPreferences.h"

static QString stripToOSMId(const QString& id)
{
	int f = id.lastIndexOf("_");
	if (f>0)
		return id.right(id.length()-(f+1));
	return id;
}

const QString encodeAttributes(const QString & text)
{
	QString s = text;
	s.replace( "&", "&amp;" );
	s.replace( ">", "&gt;" );
	s.replace( "<", "&lt;" );
	s.replace( "\"", "&quot;" );
	s.replace( "\'", "&apos;" );
	return s;
};

static QString tagOSM(const MapFeature& F)
{
	QString S;
	for (int i=0; i<F.tagSize(); ++i)
	{
		if (F.tagKey(i).startsWith('_') && (F.tagKey(i).endsWith('_')))
			continue;

		if (M_PREFS->apiVersionNum() > 0.5 && F.tagKey(i) == "created_by")
			continue;

		const QString & tagKey = encodeAttributes( F.tagKey(i) );
		const QString & tagValue = encodeAttributes( F.tagValue(i) );

		S += QString("<tag k=\"%1\" v=\"%2\"/>").arg( tagKey ).arg( tagValue );
	}
	return S;
}

QString versionAttribute(const MapFeature& F)
{
	if (M_PREFS->apiVersionNum() > 0.5)
		return QString(" version=\"%1\"").arg(F.versionNumber());
	return "";
}

QString exportOSM(const TrackPoint& Pt, const QString& ChangesetId)
{
	QString S;
	if (ChangesetId.isEmpty())
		S += QString("<node id=\"%1\" lat=\"%2\" lon=\"%3\"%4>")
			.arg(stripToOSMId(Pt.id())).arg(intToAng(Pt.position().lat()),0,'f',8).arg(intToAng(Pt.position().lon()),0,'f',8).arg(versionAttribute(Pt));
	else
		S += QString("<node id=\"%1\" lat=\"%2\" lon=\"%3\"%4 changeset=\"%5\">")
			.arg(stripToOSMId(Pt.id())).arg(intToAng(Pt.position().lat()),0,'f',8).arg(intToAng(Pt.position().lon()),0,'f',8).arg(versionAttribute(Pt))
			.arg(ChangesetId);
	S+=tagOSM(Pt);
	S+="</node>";
	return S;
}

QString exportOSM(const Road& R, const QString& ChangesetId)
{
	if (!R.size()) return "";

	QString S;
	if (ChangesetId.isEmpty())
		S += QString("<way id=\"%1\"%2>").arg(stripToOSMId(R.id())).arg(versionAttribute(R));
	else
		S += QString("<way id=\"%1\"%2 changeset=\"%3\">").arg(stripToOSMId(R.id())).arg(versionAttribute(R)).arg(ChangesetId);
	S+=QString("<nd ref=\"%1\"/>").arg(stripToOSMId(R.get(0)->id()));
	for (int i=1; i<R.size(); ++i)
		if (R.get(i)->id() != R.get(i-1)->id())
			S+=QString("<nd ref=\"%1\"/>").arg(stripToOSMId(R.get(i)->id()));
	S += tagOSM(R);
	S += "</way>";
	return S;
}

QString exportOSM(const Relation& R, const QString& ChangesetId)
{
	QString S;
	if (ChangesetId.isEmpty())
		S += QString("<relation id=\"%1\"%2>").arg(stripToOSMId(R.id())).arg(versionAttribute(R));
	else
		S += QString("<relation id=\"%1\"%2 changeset=\"%3\">").arg(stripToOSMId(R.id())).arg(versionAttribute(R)).arg(ChangesetId);
	for (int i=0; i<R.size(); ++i)
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
		return "<osm>"+S+"</osm>"+QChar(0);
		//return "<osm version=\"0.6\" changeset=\""+ChangeSetId+"\">"+S+"</osm>"+QChar(0);
}


