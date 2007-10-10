#include "CreateNodeInteraction.h"

#include "MainWindow.h"
#include "PropertiesDock.h"
#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"
#include "Utils/LineF.h"

#include <vector>

CreateNodeInteraction::CreateNodeInteraction(MapView* aView)
: GenericFeatureSnapInteraction(aView)
{
}

CreateNodeInteraction::~CreateNodeInteraction(void)
{
}

void CreateNodeInteraction::snapMouseReleaseEvent(QMouseEvent * ev, Road* aRoad)
{
	if (ev->button() == Qt::LeftButton)
	{
		Coord P(projection().inverse(ev->pos()));
		if (aRoad)
		{
			main()->properties()->setSelection(0);
			CommandList* theList = new CommandList;
			unsigned int SnapIdx = findSnapPointIndex(aRoad, P);
			TrackPoint* N = new TrackPoint(P);
			theList->add(new AddFeatureCommand(main()->activeLayer(),N,true));
			theList->add(new RoadAddTrackPointCommand(aRoad,N,SnapIdx));
			document()->history().add(theList);
			view()->invalidate();
		}
		else
		{
			TrackPoint* N = new TrackPoint(P);
			document()->history().add(new AddFeatureCommand(main()->activeLayer(),N,true));
			view()->invalidate();
		}
	}
}


QCursor CreateNodeInteraction::cursor() const
{
	return QCursor(Qt::CrossCursor);
}




