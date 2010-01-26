#include "CreateSingleWayInteraction.h"
#include "DocumentCommands.h"
#include "WayCommands.h"
#include "NodeCommands.h"
#include "Maps/Painting.h"
#include "Way.h"
#include "Node.h"
#include "Utils/LineF.h"
#include "MainWindow.h"
#include "PropertiesDock.h"

#include <QtGui/QDockWidget>
#include <QtGui/QPainter>

CreateSingleWayInteraction::CreateSingleWayInteraction(MainWindow* aMain, MapView* aView, Node *firstNode, bool aCurved)
	: FeatureSnapInteraction(aView), Main(aMain), theRoad(0), FirstPoint(0,0),
	 FirstNode(firstNode), HaveFirst(false), Prepend(false), IsCurved(aCurved), Creating(false)
{
	if (firstNode)
	{
		FirstPoint = firstNode->position();
		LastCursor = COORD_TO_XY(FirstPoint);
		HaveFirst = true;
		if ((theRoad = Way::GetSingleParentRoad(firstNode))) {
			if (theRoad->isExtrimity(firstNode)) {
				Prepend = (theRoad->get(0) == firstNode) ? true : false;
			} else
				theRoad = NULL;

		}
	}
}

CreateSingleWayInteraction::~CreateSingleWayInteraction()
{
}

QString CreateSingleWayInteraction::toHtml()
{
	QString help;
	//help = (MainWindow::tr("LEFT-CLICK to select; LEFT-DRAG to move"));

	QString desc;
	desc = QString("<big><b>%1</b></big><br/>").arg(MainWindow::tr("Create way Interaction"));
	desc += QString("<b>%1</b><br/>").arg(help);

	QString S =
	"<html><head/><body>"
	"<small><i>" + QString(metaObject()->className()) + "</i></small><br/>"
	+ desc;
	S += "</body></html>";

	return S;
}

void CreateSingleWayInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
	if (theRoad && (!theRoad->layer() || theRoad->isDeleted())) { // The road was begon and then undoed. Restarting....
		HaveFirst = false;
		theRoad = NULL;
	}

	if (HaveFirst)
	{
		QPointF PreviousPoint;
		if (theRoad && theRoad->size() && !Prepend)
			PreviousPoint = COORD_TO_XY(CAST_NODE(theRoad->get(theRoad->size()-1))->position());
		else
			PreviousPoint = COORD_TO_XY(FirstPoint);
		QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
		QPen TP(SomeBrush,view()->pixelPerM()*4+2);
		::draw(thePainter,TP,Feature::UnknownDirection, PreviousPoint,LastCursor ,4 ,view()->projection());

		Coord NewPoint = XY_TO_COORD(LastCursor);
		const double distance = FirstPoint.distanceFrom(NewPoint);

		QString distanceTag;
		if (distance < 1.0)
			distanceTag = QString("%1 m").arg(int(distance * 1000));
		else
			distanceTag = QString("%1 km").arg(distance, 0, 'f', 3);

		thePainter.drawText(LastCursor + QPointF(10,-10), distanceTag);
	}
	FeatureSnapInteraction::paintEvent(anEvent,thePainter);
}

void CreateSingleWayInteraction::snapMouseMoveEvent(QMouseEvent* ev, Feature* aFeature)
{
	if (Node* Pt = dynamic_cast<Node*>(aFeature))
		LastCursor = COORD_TO_XY(Pt);
	else if (Way* R = dynamic_cast<Way*>(aFeature))
	{
		Coord P(XY_TO_COORD(ev->pos()));
		findSnapPointIndex(R, P);
		LastCursor = COORD_TO_XY(P);
	}
	else
		LastCursor = ev->pos();
	view()->update();
}

void CreateSingleWayInteraction::snapMousePressEvent(QMouseEvent* anEvent, Feature* aFeature)
{
	Q_UNUSED(aFeature)
	Creating = true;
}

void CreateSingleWayInteraction::snapMouseReleaseEvent(QMouseEvent* anEvent, Feature* aFeature)
{
	if (M_PREFS->getMouseSingleButton() && anEvent->button() == Qt::RightButton) {
		HaveFirst = false;
		theRoad = NULL;
	} else
	if ( Creating && !panning() )
	{
		Node* Pt = dynamic_cast<Node*>(aFeature);
		if (!HaveFirst)
		{
			HaveFirst = true;
			if (Pt)
				FirstNode = Pt;
			else if (Way* aRoad = dynamic_cast<Way*>(aFeature))
			{
				Coord P(XY_TO_COORD(anEvent->pos()));
				int SnapIdx = findSnapPointIndex(aRoad, P);
				Node* N = new Node(P);
				if (M_PREFS->apiVersionNum() < 0.6)
					N->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
				CommandList* theList  = new CommandList(MainWindow::tr("Create Node %1 in Road %2").arg(N->description()).arg(aRoad->description()), N);
				theList->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),N,true));
				theList->add(new WayAddNodeCommand(aRoad,N,SnapIdx,Main->document()->getDirtyOrOriginLayer(aRoad)));
				document()->addHistory(theList);
				view()->invalidate(true, false);
				FirstNode = N;
			}
		}
		else
		{
			CommandList* L  = new CommandList();
			if (!theRoad)
			{
				Node* From = 0;
				theRoad = new Way;
				if (M_PREFS->apiVersionNum() < 0.6)
					theRoad->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
				if (IsCurved)
					theRoad->setTag("smooth","yes");
				if (FirstNode) {
					From = FirstNode;
					if (!From->isDirty() && !From->hasOSMId() && From->isUploadable())
						L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),From,true));
				}
				else
				{
					From = new Node(FirstPoint);
					if (M_PREFS->apiVersionNum() < 0.6)
						From->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
					L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),From,true));
				}
				L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),theRoad,true));
				L->add(new WayAddNodeCommand(theRoad,From));
				L->setDescription(MainWindow::tr("Create Road: %1").arg(theRoad->description()));
				L->setFeature(theRoad);
			}
			Node* To = 0;
			if (Pt) {
				To = Pt;
				if (!To->isDirty() && !To->hasOSMId() && To->isUploadable()) {
					L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),To,true));
					L->setDescription(MainWindow::tr("Create Node: %1").arg(To->description()));
				}
			}
			else if (Way* aRoad = dynamic_cast<Way*>(aFeature))
			{
				Coord P(XY_TO_COORD(anEvent->pos()));
				int SnapIdx = findSnapPointIndex(aRoad, P);
				Node* N = new Node(P);
				if (M_PREFS->apiVersionNum() < 0.6)
					N->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
				CommandList* theList  = new CommandList(MainWindow::tr("Create Node %1 in Road %2").arg(N->description()).arg(aRoad->description()), N);
				theList->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),N,true));
				theList->add(new WayAddNodeCommand(aRoad,N,SnapIdx,Main->document()->getDirtyOrOriginLayer(aRoad)));
				document()->addHistory(theList);
				view()->invalidate(true, false);
				To = N;
			}
			if (!To)
			{
				To = new Node(XY_TO_COORD(anEvent->pos()));
				if (M_PREFS->apiVersionNum() < 0.6)
					To->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
				L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),To,true));
				L->setDescription(MainWindow::tr("Create Node %1 in Road %2").arg(To->description()).arg(theRoad->description()));
				L->setFeature(To);
			}
			L->setDescription(MainWindow::tr("Add Node %1 to Road %2").arg(To->description()).arg(theRoad->description()));
			if (Prepend)
				L->add(new WayAddNodeCommand(theRoad,To,(int)0,Main->document()->getDirtyOrOriginLayer(theRoad)));
			else
				L->add(new WayAddNodeCommand(theRoad,To,Main->document()->getDirtyOrOriginLayer(theRoad)));
			document()->addHistory(L);
			view()->invalidate(true, false);
			Main->properties()->setSelection(theRoad);
		}
		FirstPoint = XY_TO_COORD(anEvent->pos());
	}
	Creating = false;
	LastCursor = anEvent->pos();
}

#ifndef Q_OS_SYMBIAN
QCursor CreateSingleWayInteraction::cursor() const
{
	return QCursor(Qt::CrossCursor);
}
#endif
