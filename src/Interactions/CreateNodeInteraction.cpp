#include "CreateNodeInteraction.h"

#include "MainWindow.h"
#include "PropertiesDock.h"
#include "DocumentCommands.h"
#include "WayCommands.h"
#include "Maps/Projection.h"
#include "Node.h"
#include "Utils/LineF.h"
#include "MoveNodeInteraction.h"

#include <QList>

CreateNodeInteraction::CreateNodeInteraction(MapView* aView)
	: FeatureSnapInteraction(aView)
	, theMoveInteraction(0)
{
}

CreateNodeInteraction::~CreateNodeInteraction(void)
{
	SAFE_DELETE(theMoveInteraction);
}

QString CreateNodeInteraction::toHtml()
{
	QString help;
	//help = (MainWindow::tr("LEFT-CLICK to select; LEFT-DRAG to move"));

	QString desc;
	desc = QString("<big><b>%1</b></big><br/>").arg(MainWindow::tr("Create node Interaction"));
	desc += QString("<b>%1</b><br/>").arg(help);

	QString S =
	"<html><head/><body>"
	"<small><i>" + QString(metaObject()->className()) + "</i></small><br/>"
	+ desc;
	S += "</body></html>";

	return S;
}

void CreateNodeInteraction::snapMousePressEvent(QMouseEvent * ev, Feature* aFeat)
{
	if (CAST_NODE(aFeat)) {
		return theMoveInteraction->snapMousePressEvent(ev, aFeat);
	} else {
		SAFE_DELETE(theMoveInteraction);
		theView->setCursor(cursor());
	}

}

void CreateNodeInteraction::snapMouseMoveEvent(QMouseEvent* ev, Feature* aFeat)
{
	if (CAST_NODE(aFeat)) {
		if (!theMoveInteraction) {
			theMoveInteraction = new MoveNodeInteraction(theView);
		}
		theView->setCursor(theMoveInteraction->cursor());
	} else
		theView->setCursor(cursor());

	if (theMoveInteraction)
		return theMoveInteraction->snapMouseMoveEvent(ev, aFeat);
}

void CreateNodeInteraction::snapMouseReleaseEvent(QMouseEvent * ev, Feature* aFeat)
{
	if (theMoveInteraction) {
		theMoveInteraction->snapMouseReleaseEvent(ev, aFeat);
		return;
	}

	Way* aRoad = dynamic_cast<Way*>(aFeat);
	if (!aFeat || aRoad) {
		SAFE_DELETE(theMoveInteraction);
		Coord P(XY_TO_COORD(ev->pos()));
		if (aRoad)
		{
			main()->properties()->setSelection(0);
			CommandList* theList  = new CommandList(MainWindow::tr("Create node in Road: %1").arg(aRoad->id()), aRoad);
			int SnapIdx = findSnapPointIndex(aRoad, P);
			Node* N = new Node(P);
			if (M_PREFS->apiVersionNum() < 0.6)
				N->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
			theList->add(new AddFeatureCommand(main()->document()->getDirtyOrOriginLayer(aRoad->layer()),N,true));
			theList->add(new WayAddNodeCommand(aRoad,N,SnapIdx,main()->document()->getDirtyOrOriginLayer(aRoad->layer())));
			document()->addHistory(theList);
			main()->properties()->setSelection(N);
			view()->invalidate(true, false);
		}
		else
		{
			Node* N = new Node(P);
			if (M_PREFS->apiVersionNum() < 0.6)
				N->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
			CommandList* theList  = new CommandList(MainWindow::tr("Create point %1").arg(N->id()), aRoad);
			theList->add(new AddFeatureCommand(main()->document()->getDirtyOrOriginLayer(),N,true));
			document()->addHistory(theList);
			main()->properties()->setSelection(N);
			view()->invalidate(true, false);
		}
		theMoveInteraction = new MoveNodeInteraction(theView);
		theView->setCursor(theMoveInteraction->cursor());
	}
}

#ifndef Q_OS_SYMBIAN
QCursor CreateNodeInteraction::cursor() const
{
	return QCursor(Qt::CrossCursor);
}
#endif




