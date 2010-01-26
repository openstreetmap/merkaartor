//
// C++ Implementation: ImportNMEA
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

#include "../ImportExport/ImportNMEA.h"


ImportNMEA::ImportNMEA(Document* doc)
 : IImportExport(doc), curAltitude(0.0)
{
}


ImportNMEA::~ImportNMEA()
{
}

// no export
bool ImportNMEA::export_(const QList<Feature *>& featList)
{
	IImportExport::export_(featList);

	return false;
}

// import the  input
bool ImportNMEA::import(Layer* aLayer)
{
	bool goodFix = false;
	bool goodFix3D = true;
	QTextStream in(Device);

	theLayer = dynamic_cast <TrackLayer *> (aLayer);
	theList = new CommandList(MainWindow::tr("Import NMEA"), NULL);

	TrackSegment* TS = new TrackSegment;

	while (!in.atEnd()) {
		QString line = in.readLine();

		if (line.left(3) != "$GP")
			continue;

		QString command = line.mid(3, 3);
		if (command == "GSA") {
			bool prevGoodFix = goodFix3D;
			goodFix3D = importGSA(line);
			if (!goodFix3D && prevGoodFix) {
				if (TS->size())
					theList->add(new AddFeatureCommand(theLayer,TS, true));
				else
					delete TS;
				TS = new TrackSegment;
			}
		} else
		if (command == "GSV") {
			if (goodFix && goodFix3D)
				importGSV(line);
		} else
		if (command == "GGA") {
			bool prevGoodFix = goodFix;
			goodFix = importGGA(line);
			if (!goodFix && prevGoodFix) {
				if (TS->size())
					theList->add(new AddFeatureCommand(theLayer,TS, true));
				else
					delete TS;
				TS = new TrackSegment;
			}
		} else
		if (command == "GLL") {
			bool prevGoodFix = goodFix;
			goodFix = importGLL(line);
			if (!goodFix && prevGoodFix) {
				if (TS->size())
					theList->add(new AddFeatureCommand(theLayer,TS, true));
				else
					delete TS;
				TS = new TrackSegment;
			}
		} else
		if (command == "RMC") {
			if (goodFix && goodFix3D) {
				Node* p = importRMC(line);
				if (p)
					TS->add(p);
			}
		} else
		{/* Not handled */}
	}

	if (TS->size())
		theList->add(new AddFeatureCommand(theLayer,TS, true));
	else
		delete TS;

	delete theList;
	return true;
}

bool ImportNMEA::importGSA (QString line)
{
	if (line.count('$') > 1)
		return false;

	QStringList tokens = line.split(",");
	if (tokens.size() < 3)
		return false;

	QString autoSelectFix = tokens[1];
	int Fix3D = tokens[2].toInt();

	// double PDOP = tokens[15].toDouble();
	// double HDOP = tokens[16].toDouble();
	// double VDOP = tokens[17].toDouble();

	return (Fix3D == 1 ? false: true);
}

bool ImportNMEA::importGSV (QString /* line */)
{
	return true;
}

bool ImportNMEA::importGGA (QString line)
{
	if (line.count('$') > 1)
		return false;

	QStringList tokens = line.split(",");

	if (tokens.size() < 10)
		return false;

	//double lat = tokens[2].left(2).toDouble();
	//double lon = tokens[4].left(3).toDouble();

	//double latmin = tokens[2].mid(2).toDouble();
	//lat += latmin / 60.0;
	//if (tokens[3] != "N")
	//	lat = -lat;
	//double lonmin = tokens[4].mid(3).toDouble();
	//lon += lonmin / 60.0;
	//if (tokens[5] != "E")
	//	lon = -lon;

	int fix = tokens[6].toInt();
	if (fix == 0)
		return false;

	curAltitude = tokens[9].toDouble();

	return true;
}

bool ImportNMEA::importGLL (QString line)
{
	if (line.count('$') > 1)
		return false;

	QStringList tokens = line.split(",");

	if (tokens.size() < 7)
		return false;

	if (tokens[6] != "A")
		return false;

	return true;
}

Node* ImportNMEA::importRMC (QString line)
{
	if (line.count('$') > 1)
		return NULL;

	QStringList tokens = line.split(",");
	if (tokens.size() < 10)
		return NULL;

	//int time = tokens[1];
	if (tokens[2] != "A")
		return NULL;

	double lat = tokens[3].left(2).toDouble();
	double latmin = tokens[3].mid(2).toDouble();
	lat += latmin / 60.0;
	if (tokens[4] != "N")
		lat = -lat;
	double lon = tokens[5].left(3).toDouble();
	double lonmin = tokens[5].mid(3).toDouble();
	lon += lonmin / 60.0;
	if (tokens[6] != "E")
		lon = -lon;
	double speed = tokens[7].toDouble() * 1.852;
	//int date = token[9];

	QString strDate = tokens[9] + tokens[1];
	QDateTime date = QDateTime::fromString(strDate, "ddMMyyHHmmss.zzz");
	if (!date.isValid())
		return NULL;

	if (date.date().year() < 1970)
		date = date.addYears(100);
	//date.setTimeSpec(Qt::UTC);

	Node* Pt = new Node(Coord(angToInt(lat),angToInt(lon)));
	Pt->setLastUpdated(Feature::Log);
	Pt->setElevation(curAltitude);
	Pt->setSpeed(speed);
	theList->add(new AddFeatureCommand(theLayer,Pt, true));
	Pt->setTime(date);

	return Pt;
}
