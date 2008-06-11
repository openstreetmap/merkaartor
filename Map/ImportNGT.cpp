#include "ImportNGT.h"
#include "Command/DocumentCommands.h"
#include "Map/MapDocument.h"
#include "Map/TrackPoint.h"
#include "Map/TrackSegment.h"

#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

bool importNGT(QWidget* /* aParent */, const QString& aFilename, MapDocument* theDocument, MapLayer* theLayer)
{
	QFile f(aFilename);
	if (f.open(QIODevice::ReadOnly))
	{
		QTextStream s(&f);
		CommandList* theList  = new CommandList(MainWindow::tr("Import NGT"), NULL);
		TrackSegment* theSegment = new TrackSegment;
		while (!f.atEnd())
		{
			QString Line(f.readLine());
			QStringList Items(Line.split('|'));
			if (Items.count() >= 5)
			{
				TrackPoint* Pt = new TrackPoint(Coord(Items[4].toDouble()*3.141592,Items[3].toDouble()*3.141592));
				theList->add(new AddFeatureCommand(theLayer,Pt, true));
				theSegment->add(Pt);
			}
		}
		if (theList->empty())
		{
			delete theList;
			delete theSegment;
		}
		else
		{
			theList->add(new AddFeatureCommand(theLayer,theSegment,true));
			theDocument->addHistory(theList);
		}
		return true;
	}
	return false;
}
