//
// C++ Implementation: ImportExportKML
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

#include "../ImportExport/ImportExportKML.h"

bool parseContainer(QDomElement& e, MapLayer* aLayer);

ImportExportKML::ImportExportKML(MapDocument* doc)
 : IImportExport(doc)
{
}


ImportExportKML::~ImportExportKML()
{
}


// export
bool ImportExportKML::export_(const QVector<MapFeature *>& featList)
{
	QVector<TrackPoint*>	waypoints;
	QVector<TrackSegment*>	segments;
	QDomElement k;
	QDomText v;

	if(! IImportExport::export_(featList) ) return false;

	bool OK = true;

	QDomDocument theXmlDoc;
	theXmlDoc.appendChild(theXmlDoc.createProcessingInstruction("xml", "version=\"1.0\""));

	QDomElement kml = theXmlDoc.createElement("kml");
	theXmlDoc.appendChild(kml);
	kml.setAttribute("xmlns", "http://earth.google.com/kml/2.2");

	QDomElement d = theXmlDoc.createElement("Document");
	kml.appendChild(d);


	//QDomElement g = theXmlDoc.createElement("MultiGeometry");
	//p.appendChild(g);

	for (int i=0; i<theFeatures.size(); ++i) {
		if (Road* R = dynamic_cast<Road*>(theFeatures[i])) {
			QDomElement p = theXmlDoc.createElement("Placemark");
			d.appendChild(p);

			k = theXmlDoc.createElement("name");
			p.appendChild(k);
			v = theXmlDoc.createTextNode(R->description());
			k.appendChild(v);

			k = theXmlDoc.createElement("description");
			p.appendChild(k);
			QString desc;
			for (unsigned int j=0; j<R->tagSize(); ++j) {
				desc += R->tagKey(j);
				desc += "=";
				desc += R->tagValue(j);
				desc += "<br/>";
			}
			v = theXmlDoc.createTextNode(desc);
			k.appendChild(v);

			k = theXmlDoc.createElement("Style");
			p.appendChild(k);

			QDomElement ls = theXmlDoc.createElement("LineStyle");
			k.appendChild(ls);

			const FeaturePainter* fp = R->getCurrentEditPainter();
			if (fp) {
				QDomElement color = theXmlDoc.createElement("color");
				ls.appendChild(color);
				QRgb kcolor = fp->ForegroundColor.rgba();
				v = theXmlDoc.createTextNode(QString::number(qRgba(qBlue(kcolor), qGreen(kcolor), qRed(kcolor), /*qAlpha(kcolor)*/ 192), 16));
				color.appendChild(v);
			} 
			QDomElement width = theXmlDoc.createElement("width");
			ls.appendChild(width);
			v = theXmlDoc.createTextNode(QString::number(R->widthOf()));
			width.appendChild(v);

			QDomElement l = theXmlDoc.createElement("LineString");
			p.appendChild(l);

			QDomElement c = theXmlDoc.createElement("coordinates");
			l.appendChild(c);
			
			QString s;
			for (unsigned int j=0; j<R->size(); ++j) {
				TrackPoint* N = dynamic_cast<TrackPoint*>(R->get(j));
				s += QString(" %1,%2").arg(QString::number(intToAng(N->position().lon()),'f',8)).arg(QString::number(intToAng(N->position().lat()),'f',8));
			}

			QDomText v = theXmlDoc.createTextNode(s);
			c.appendChild(v);
		}
		else if (TrackPoint* N = dynamic_cast<TrackPoint*>(theFeatures[i])) {
			if (N->sizeParents()) continue;

			QDomElement p = theXmlDoc.createElement("Placemark");
			d.appendChild(p);

			k = theXmlDoc.createElement("name");
			p.appendChild(k);
			v = theXmlDoc.createTextNode(N->description());
			k.appendChild(v);

			k = theXmlDoc.createElement("description");
			p.appendChild(k);
			QString desc;
			for (unsigned int j=0; j<N->tagSize(); ++j) {
				desc += N->tagKey(j);
				desc += "=";
				desc += N->tagValue(j);
				desc += "<br/>";
			}
			v = theXmlDoc.createTextNode(desc);
			k.appendChild(v);

			//k = theXmlDoc.createElement("Style");
			//p.appendChild(k);

			//QDomElement ls = theXmlDoc.createElement("LineStyle");
			//k.appendChild(ls);

			//FeaturePainter* fp = R->getCurrentEditPainter();
			//if (fp) {
			//	QDomElement color = theXmlDoc.createElement("color");
			//	ls.appendChild(color);
			//	QRgb kcolor = fp->ForegroundColor.rgba();
			//	v = theXmlDoc.createTextNode(QString::number(qRgba(qBlue(kcolor), qGreen(kcolor), qRed(kcolor), /*qAlpha(kcolor)*/ 164), 16));
			//	color.appendChild(v);
			//} 
			//QDomElement width = theXmlDoc.createElement("width");
			//ls.appendChild(width);
			//v = theXmlDoc.createTextNode(QString::number(widthOf(R)));
			//width.appendChild(v);

			QDomElement l = theXmlDoc.createElement("Point");
			p.appendChild(l);

			QDomElement c = theXmlDoc.createElement("coordinates");
			l.appendChild(c);
			
			QString s;
			s += QString(" %1,%2").arg(QString::number(intToAng(N->position().lon()),'f',8)).arg(QString::number(intToAng(N->position().lat()),'f',8));

			QDomText v = theXmlDoc.createTextNode(s);
			c.appendChild(v);
		}
	}

	Device->write(theXmlDoc.toString().toUtf8());
	return OK;

}

// IMPORT

QString kmlId;

MapFeature* parsePoint(QDomElement& e, MapLayer* aLayer)
{
	TrackPoint* P = NULL;

	QDomElement c = e.firstChildElement();
	while(!c.isNull() && !P) {
		if (c.tagName() == "coordinates") {
			QDomText t = c.firstChild().toText();
			QString s = t.nodeValue();
			QStringList tokens = s.split(",");
			double lon = tokens[0].toDouble();
			double lat = tokens[1].toDouble();
			Coord p(angToInt(lat), angToInt(lon));

			P = new TrackPoint(p);
			P->setTag("%kml:guid", kmlId);
			aLayer->add(P);
		}

		c = c.nextSiblingElement();
	}

	return P;
}

MapFeature* parseGeometry(QDomElement& e, MapLayer* aLayer)
{
	MapFeature* F = NULL;
	if (e.tagName() == "Point") {
		F = parsePoint(e, aLayer);
	}

	return F;
}

bool parsePlacemark(QDomElement& e, MapLayer* aLayer)
{
	MapFeature* F = NULL;
	QDomElement c = e.firstChildElement();
	QString name;
	QString address;
	QString description;
	QString phone;

	while(!c.isNull()) {
		if (c.tagName() == "name")
			name = c.firstChild().toText().nodeValue();
		else
		if (c.tagName() == "address")
			address = c.firstChild().toText().nodeValue();
		else
		if (c.tagName() == "description")
			description = c.firstChild().toText().nodeValue();
		else
		if (c.tagName() == "phoneNumber")
			phone = c.firstChild().toText().nodeValue();
		else
		F = parseGeometry(c, aLayer);

		c = c.nextSiblingElement();
	}

	if (F) {
		if (!name.isEmpty())
			F->setTag("name", name);
		if (!address.isEmpty())
			F->setTag("addr:full", address);
		if (!phone.isEmpty())
			F->setTag("addr:phone_number", phone);
		if (!description.isEmpty())
			F->setTag("description", description);
		return true;
	} else
		return false;
}

bool parseFeature(QDomElement& e, MapLayer* aLayer)
{
	bool ret= false;
	QDomElement c = e.cloneNode().toElement();

	while(!c.isNull()) {
		if (c.tagName() == "Placemark")
			ret = parsePlacemark(c, aLayer);
		else
			ret = parseContainer(c, aLayer);

		c = c.nextSiblingElement();
	}
	return ret;
}

bool parseContainer(QDomElement& e, MapLayer* aLayer)
{
	if ((e.tagName() != "Document") && (e.tagName() != "Folder"))
		return false;

	bool ret= false;
	QDomElement c = e.firstChildElement();

	while(!c.isNull()) {
		ret = parseFeature(c, aLayer);

		c = c.nextSiblingElement();
	}
	return ret;
}

bool parseKML(QDomElement& e, MapLayer* aLayer)
{
	bool ret= false;
	QDomElement c = e.firstChildElement();
	
	while(!c.isNull()) {
		ret = parseFeature(c, aLayer);
		if (!ret)
			ret = parseGeometry(c, aLayer);

		c = c.nextSiblingElement();
	}
	return ret;
}

// import the  input
bool ImportExportKML::import(MapLayer* aLayer)
{
	QDomDocument* theXmlDoc = new QDomDocument();
	if (!theXmlDoc->setContent(Device)) {
		//QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid XML file.").arg(fn));
		Device->close();
		delete theXmlDoc;
		theXmlDoc = NULL;
		return false;
	}
	Device->close();

	QDomElement docElem = theXmlDoc->documentElement();
	if (docElem.tagName() != "kml") {
		//QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid KML document.").arg(fn));
		return false;
	}
	kmlId = QUuid::createUuid().toString();
	return parseKML(docElem, aLayer);
}

