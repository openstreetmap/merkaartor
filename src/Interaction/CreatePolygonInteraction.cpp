#include "Interaction/CreatePolygonInteraction.h"
#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Maps/Painting.h"
#include "Maps/Road.h"
#include "Maps/TrackPoint.h"
#include "Utils/LineF.h"
#include "PropertiesDock.h"
#include "Preferences/MerkaartorPreferences.h"

#include <QtGui/QPainter>
#include <QInputDialog>

#include <math.h>

CreatePolygonInteraction::CreatePolygonInteraction(MainWindow* aMain, MapView* aView)
	: Interaction(aView), Main(aMain), Center(0,0), HaveCenter(false)
{
	Sides = QInputDialog::getInteger(aView, MainWindow::tr("Create Polygon"), MainWindow::tr("Specify the number of sides"), 4, 3);
}

CreatePolygonInteraction::~CreatePolygonInteraction()
{
	view()->update();
}

void CreatePolygonInteraction::testIntersections(CommandList* L, Road* Left, int FromIdx, Road* Right, int RightIndex)
{
	LineF L1(view()->projection().project(Right->getNode(RightIndex-1)),
		view()->projection().project(Right->getNode(RightIndex)));
	for (int i=FromIdx; i<Left->size(); ++i)
	{
		LineF L2(view()->projection().project(Left->getNode(i-1)),
			view()->projection().project(Left->getNode(i)));
		QPointF Intersection(L1.intersectionWith(L2));
		if (L1.segmentContains(Intersection) && L2.segmentContains(Intersection))
		{
			TrackPoint* Pt = new TrackPoint(view()->projection().inverse(Intersection));
			L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),Pt,true));
			L->add(new RoadAddTrackPointCommand(Left,Pt,i));
			L->add(new RoadAddTrackPointCommand(Right,Pt,RightIndex));
			testIntersections(L,Left,i+2,Right,RightIndex);
			testIntersections(L,Left,i+2,Right,RightIndex+1);
			return;
		}
	}
}

void CreatePolygonInteraction::mousePressEvent(QMouseEvent * event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		if (!HaveCenter)
		{
			HaveCenter = true;
			Center = view()->projection().inverse(event->pos());
		}
		else
		{
			QPointF CenterF(view()->projection().project(Center));
			double Radius = distance(CenterF,LastCursor)/view()->projection().pixelPerM();
			double Precision = 2.49;
			if (Radius<2.5)
				Radius = 2.5;
			double Angle = 2*acos(1-Precision/Radius);
			Angle = 2*M_PI/Sides;
			Radius *= view()->projection().pixelPerM();
			double StartAngle = angle(QPointF(10,0), LastCursor-CenterF);
			QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
			QPen TP(SomeBrush,projection().pixelPerM()*4);
			QPointF Prev(CenterF.x()+cos(StartAngle + Angle/2)*Radius,CenterF.y()+sin(StartAngle + Angle/2)*Radius);
			TrackPoint* First = new TrackPoint(view()->projection().inverse(Prev));
			Road* R = new Road;
			R->add(First);
			if (M_PREFS->apiVersionNum() < 0.6)
				R->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
			CommandList* L  = new CommandList(MainWindow::tr("Create Polygon %1").arg(R->id()), R);
			L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),First,true));
			for (double a = StartAngle + Angle*3/2; a<2*M_PI+StartAngle; a+=Angle)
			{
				QPointF Next(CenterF.x()+cos(a)*Radius,CenterF.y()+sin(a)*Radius);
				TrackPoint* New = new TrackPoint(view()->projection().inverse(Next));
				if (M_PREFS->apiVersionNum() < 0.6)
					New->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),New,true));
				R->add(New);
			}
			R->add(First);
			L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),R,true));
			for (FeatureIterator it(document()); !it.isEnd(); ++it)
			{
				Road* W1 = dynamic_cast<Road*>(it.get());
				if (W1 && (W1 != R))
					for (int i=1; i<W1->size(); ++i)
					{
						int Before = W1->size();
						testIntersections(L,R,1,W1,i);
						int After = W1->size();
						i += (After-Before);
					}
			}
			Main->properties()->setSelection(R);
			document()->addHistory(L);
			view()->invalidate(true, false);
			view()->launch(0);
		}
	}
	else
		Interaction::mousePressEvent(event);
}

void CreatePolygonInteraction::mouseMoveEvent(QMouseEvent* event)
{
	LastCursor = event->pos();
	if (HaveCenter)
		view()->update();
	Interaction::mouseMoveEvent(event);
}

void CreatePolygonInteraction::paintEvent(QPaintEvent* , QPainter& thePainter)
{
	if (HaveCenter)
	{
		QPointF CenterF(view()->projection().project(Center));
		double Radius = distance(CenterF,LastCursor)/view()->projection().pixelPerM();
		double StartAngle = angle(QPointF(10,0), LastCursor-CenterF);
		double Precision = 1.99;
		if (Radius<2)
			Radius = 2;
		double Angle = 2*acos(1-Precision/Radius);
		Angle = 2*M_PI/Sides;
		Radius *= view()->projection().pixelPerM();
		QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
		QPen TP(SomeBrush,projection().pixelPerM()*4);
		QPointF Prev(CenterF.x()+cos(StartAngle + Angle/2)*Radius,CenterF.y()+sin(StartAngle + Angle/2)*Radius);
		for (double a = StartAngle + Angle*3/2; a<2*M_PI+StartAngle; a+=Angle)
		{
			QPointF Next(CenterF.x()+cos(a)*Radius,CenterF.y()+sin(a)*Radius);
			::draw(thePainter,TP,MapFeature::UnknownDirection, Prev,Next,4,view()->projection());
			Prev = Next;
		}
		QPointF Next(CenterF.x()+cos(StartAngle + Angle/2)*Radius,CenterF.y()+sin(StartAngle + Angle/2)*Radius);
		::draw(thePainter,TP,MapFeature::UnknownDirection, Prev,Next,4,view()->projection());
	}
}

#ifndef Q_OS_SYMBIAN
QCursor CreatePolygonInteraction::cursor() const
{
	return QCursor(Qt::CrossCursor);
}
#endif
