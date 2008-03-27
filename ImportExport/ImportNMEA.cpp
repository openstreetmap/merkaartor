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


ImportNMEA::ImportNMEA()
 : IImportExport()
{
}


ImportNMEA::~ImportNMEA()
{
}

// import the  input
bool ImportNMEA::import(MapLayer* aLayer)
{
	QTextStream in(source);

	theLayer = dynamic_cast <TrackMapLayer *> (aLayer);
	theList = new CommandList;

	TrackSegment* TS = new TrackSegment;

	while (!in.atEnd()) {
		QString line = in.readLine();

		if (line.left(3) != "$GP")
			continue;

		QString command = line.mid(3, 3);
		if (command == "GSA") {
			importGSA(line);
		} else
		if (command == "GSV") {
			importGSV(line);
		} else
		if (command == "GGA") {
/*			TrackPoint* p = importGGA(line);
			if (p)
				TS->add(p);*/
		} else
		if (command == "RMC") {
			TrackPoint* p = importRMC(line);
			if (p)
				TS->add(p);
		} else
		{/* Not handled */}
	}

	if (TS->size())
		theList->add(new AddFeatureCommand(theLayer,TS, true));
	else
		delete TS;

	return true;
}

bool ImportNMEA::importGSA (QString /* line */)
{
	return true;
}

bool ImportNMEA::importGSV (QString /* line */)
{
	return true;
}

TrackPoint* ImportNMEA::importGGA (QString line)
{
	QStringList tokens = line.split(",");

	double lat = tokens[2].left(2).toDouble();
	double lon = tokens[4].left(3).toDouble();

	double latmin = tokens[2].mid(2).toDouble();
	lat += latmin / 60.0;
	if (tokens[3] != "N")
		lat = -lat;
	double lonmin = tokens[4].mid(3).toDouble();
	lon += lonmin / 60.0;
	if (tokens[5] != "E")
		lon = -lon;

		int fix = tokens[6].toInt();
	if (fix == 0)
		return NULL;


	QDateTime date = QDateTime::fromString(tokens[9] + tokens[1], "ddMMyyHHmmss.zzz");
	if (date.date().year() < 1970)
		date = date.addYears(100);
	date.setTimeSpec(Qt::UTC);

	TrackPoint* Pt = new TrackPoint(Coord(angToRad(lat),angToRad(lon)));
	theList->add(new AddFeatureCommand(theLayer,Pt, true));
	Pt->setTime(date);

	return Pt;
}

TrackPoint* ImportNMEA::importRMC (QString line)
{
	QStringList tokens = line.split(",");
	if (tokens.size() < 10)
		return NULL;

	//int time = tokens[1];
	if (tokens[2] == "V")
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
	//int date = token[9];

	QString strDate = tokens[9] + tokens[1];
	QDateTime date = QDateTime::fromString(strDate, "ddMMyyHHmmss.zzz");
	if (date.date().year() < 1970)
		date = date.addYears(100);
	date.setTimeSpec(Qt::UTC);

	TrackPoint* Pt = new TrackPoint(Coord(angToRad(lat),angToRad(lon)));
	theList->add(new AddFeatureCommand(theLayer,Pt, true));
	Pt->setTime(date);

	return Pt;
}
