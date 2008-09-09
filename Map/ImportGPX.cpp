#include "Map/ImportGPX.h"

#include "Command/Command.h"
#include "Command/DocumentCommands.h"
#include "Map/MapDocument.h"
#include "Map/TrackPoint.h"
#include "Map/TrackSegment.h"

#include <QtCore/QBuffer>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtGui/QMessageBox>
#include <QtXml/QDomDocument>
#include <QProgressDialog>


static TrackPoint* importTrkPt(const QDomElement& Root, MapDocument* /* theDocument */, MapLayer* theLayer, CommandList* theList)
{
	double Lat = Root.attribute("lat").toDouble();
	double Lon = Root.attribute("lon").toDouble();

	TrackPoint* Pt = new TrackPoint(Coord(angToInt(Lat),angToInt(Lon)));
	Pt->setLastUpdated(MapFeature::Log);
	if (Root.hasAttribute("xml:id"))
		Pt->setId(Root.attribute("xml:id"));

	theList->add(new AddFeatureCommand(theLayer,Pt, true));

	if (Root.tagName() == "wpt")
		Pt->setTag("_waypoint_", "yes");
	
	for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement t = n.toElement();

		if (t.isNull())
			continue;

		if (t.tagName() == "time")
		{
			QString Value;
			for (QDomNode s = t.firstChild(); !s.isNull(); s = s.nextSibling())
			{
				QDomText ss = s.toText();
				if (!ss.isNull())
					Value += ss.data();
			}
			if (!Value.isEmpty())
			{
				QDateTime dt(QDateTime::fromString(Value.left(19),"yyyy-MM-ddTHH:mm:ss"));
				dt.setTimeSpec(Qt::UTC);
				Pt->setTime(dt);
			}
		}
		else if (t.tagName() == "ele")
		{
			Pt->setElevation( t.text().toDouble() );
		}
		else if (t.tagName() == "speed")
		{
			Pt->setSpeed( t.text().toDouble() );
		}
		else if (t.tagName() == "name")
		{
			Pt->setTag("name", t.text());
		}
		else if (t.tagName() == "desc")
		{
			Pt->setTag("_description_", t.text(), false);
		}
		else if (t.tagName() == "cmt")
		{
			Pt->setTag("_comment_", t.text(), false);
		}
	}

	return Pt;
}


static void importTrkSeg(const QDomElement& Root, MapDocument* theDocument, MapLayer* theLayer, CommandList* theList, bool MakeSegment, QProgressDialog & progress)
{
	TrackSegment* S = new TrackSegment;
	if (Root.hasAttribute("xml:id"))
		S->setId(Root.attribute("xml:id"));
	for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement t = n.toElement();
		if (!t.isNull() && t.tagName() == "trkpt")
		{
			TrackPoint* Pt = importTrkPt(t,theDocument, theLayer, theList);
			if (MakeSegment)
				S->add(Pt);
			progress.setValue(progress.value()+1);
			if (progress.wasCanceled()) {
				return;
			}
		}
	}
	if (S->size())
		theList->add(new AddFeatureCommand(theLayer,S, true));
	else
		delete S;
}

static void importTrk(const QDomElement& Root, MapDocument* theDocument, MapLayer* theLayer, CommandList* theList, bool MakeSegment, QProgressDialog & progress)
{
	for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement t = n.toElement();
		if (!t.isNull() && t.tagName() == "trkseg") {
			importTrkSeg(t,theDocument, theLayer, theList, MakeSegment, progress);
			if (progress.wasCanceled())
				return;
		} else
		if (!t.isNull() && t.tagName() == "name") {
			theLayer->setName(t.text());
		} else
		if (!t.isNull() && t.tagName() == "desc") {
			theLayer->setDescription(t.text());
		}
	}
}

static void importGPX(const QDomElement& Root, MapDocument* theDocument, QVector<TrackMapLayer*>& theTracklayers, CommandList* theList, bool MakeSegment, QProgressDialog & progress)
{
	for(QDomNode n = Root.firstChild(); !n.isNull(); n = n.nextSibling())
	{
		QDomElement t = n.toElement();
		if (t.isNull())
			continue;

		if (t.tagName() == "trk")
		{
			TrackMapLayer* newLayer = new TrackMapLayer();
			theDocument->add(newLayer);
			importTrk(t,theDocument, newLayer, theList, MakeSegment, progress);
			if (!newLayer->size()) {
				theDocument->remove(newLayer);
				delete newLayer;
			} else {
				theTracklayers.append(newLayer);
			}
		}
		else if (t.tagName() == "wpt")
		{
			importTrkPt(t,theDocument, theTracklayers[0], theList);
			progress.setValue(progress.value()+1);
		}
		if (progress.wasCanceled())
			return;
	}
}

bool importGPX(QWidget* aParent, QIODevice& File, MapDocument* theDocument, QVector<TrackMapLayer*>& theTracklayers, bool MakeSegment)
{
	// TODO remove debug messageboxes
	QDomDocument DomDoc;
	QString ErrorStr;
	int ErrorLine;
	int ErrorColumn;
	if (!DomDoc.setContent(&File, true, &ErrorStr, &ErrorLine,&ErrorColumn))
	{
		File.close();
		QMessageBox::warning(aParent,"Parse error",
			QString("Parse error at line %1, column %2:\n%3")
                                  .arg(ErrorLine)
                                  .arg(ErrorColumn)
                                  .arg(ErrorStr));
		return false;
	}
	QDomElement root = DomDoc.documentElement();
	if (root.tagName() != "gpx")
	{
		QMessageBox::information(aParent, "Parse error","Root is not a gpx node");
		return false;
	}

	QProgressDialog progress("Importing GPX...", "Cancel", 0, 0);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMaximum(progress.maximum() + DomDoc.elementsByTagName("trkpt").count());
	progress.setMaximum(progress.maximum() + DomDoc.elementsByTagName("wpt").count());

	CommandList* theList  = new CommandList(MainWindow::tr("Import GPX"), NULL);

	importGPX(root, theDocument, theTracklayers, theList, MakeSegment, progress);

	progress.setValue(progress.maximum());
	if (progress.wasCanceled())
		return false;

	delete theList;
/*	if (theList->empty())
		delete theList;
	else
		theDocument->addHistory(theList);*/
	return true;
}


bool importGPX(QWidget* aParent, const QString& aFilename, MapDocument* theDocument, QVector<TrackMapLayer*>& theTracklayers)
{
	QFile File(aFilename);
	if (!File.open(QIODevice::ReadOnly))
	{
		return false;
	}
	return importGPX(aParent, File, theDocument, theTracklayers, true);
}

bool importGPX(QWidget* aParent, QByteArray& aFile, MapDocument* theDocument, QVector<TrackMapLayer*>& theTracklayers, bool MakeSegment)
{
	QBuffer buf(&aFile);
	return importGPX(aParent,buf, theDocument, theTracklayers, MakeSegment);
}
