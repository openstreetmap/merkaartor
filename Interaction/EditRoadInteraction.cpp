#include "Interaction/EditRoadInteraction.h"

#include "MainWindow.h"
#include "MapView.h"
#include "PropertiesDock.h"
#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Map/MapDocument.h"
#include "Map/Road.h"
#include "Map/Way.h"

#include <QtGui/QMouseEvent>

EditRoadInteraction::EditRoadInteraction(MapView* aView)
: WaySnapInteraction(aView), Current(0)
{
	main()->properties()->setSelection(0);
}

EditRoadInteraction::EditRoadInteraction(MapView* aView, Road* R)
: WaySnapInteraction(aView), Current(R)
{
	main()->properties()->setSelection(0);
}

EditRoadInteraction::~EditRoadInteraction(void)
{
}

void EditRoadInteraction::paintEvent(QPaintEvent* event, QPainter& thePainter)
{
	WaySnapInteraction::paintEvent(event,thePainter);
	if (Current)
		Current->drawFocus(thePainter,projection());
}

void EditRoadInteraction::snapMouseReleaseEvent(QMouseEvent *anEvent, Way * W)
{
	if (anEvent->button() == Qt::LeftButton)
	{
		// TODO warn to click on ways?
		if (!W) return;
		if (!Current)
		{
			Current = new Road;
			Current->add(W);
			document()->history().add(new AddFeatureCommand( main()->activeLayer() ,Current, true));
			main()->properties()->setSelection(Current);
		}
		else
		{
			if (Current->find(W) < Current->size())
				document()->history().add(new RoadRemoveWayCommand(Current, W));
			else
				document()->history().add(new RoadAddWayCommand(Current, W));
		}
		Current->setLastUpdated(MapFeature::User);
		view()->invalidate();
	}
}




