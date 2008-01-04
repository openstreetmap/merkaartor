#include "Interaction/CreateRoundaboutInteraction.h"
#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Map/Painting.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Utils/LineF.h"
#include "PropertiesDock.h"

#include <QtCore/QSettings>
#include <QtGui/QDockWidget>
#include <QtGui/QPainter>

#include <math.h>

CreateRoundaboutInteraction::CreateRoundaboutInteraction(MainWindow* aMain, MapView* aView)
	: Interaction(aView), Main(aMain), Center(0,0), HaveCenter(false)
{
	theDock = new QDockWidget(Main);
	QWidget* DockContent = new QWidget(theDock);
	DockData.setupUi(DockContent);
	theDock->setWidget(DockContent);
	theDock->setAllowedAreas(Qt::LeftDockWidgetArea);
	Main->addDockWidget(Qt::LeftDockWidgetArea, theDock);
	theDock->show();
	QSettings Sets;
	Sets.beginGroup("roadstructure");
	DockData.DriveRight->setChecked(Sets.value("rightsidedriving",true).toBool());
}

CreateRoundaboutInteraction::~CreateRoundaboutInteraction()
{
	QSettings Sets;
	Sets.beginGroup("roadstructure");
	Sets.setValue("rightsidedriving",DockData.DriveRight->isChecked());
	delete theDock;
	view()->update();
}

void CreateRoundaboutInteraction::testIntersections(CommandList* L, Road* Left, unsigned int FromIdx, Road* Right, unsigned int RightIndex)
{
	LineF L1(view()->projection().project(Right->get(RightIndex-1)->position()),
		view()->projection().project(Right->get(RightIndex)->position()));
	for (unsigned int i=FromIdx; i<Left->size(); ++i)
	{
		LineF L2(view()->projection().project(Left->get(i-1)->position()),
			view()->projection().project(Left->get(i)->position()));
		QPointF Intersection(L1.intersectionWith(L2));
		if (L1.segmentContains(Intersection) && L2.segmentContains(Intersection))
		{
			TrackPoint* Pt = new TrackPoint(view()->projection().inverse(Intersection));
			L->add(new AddFeatureCommand(Main->activeLayer(),Pt,true));
			L->add(new RoadAddTrackPointCommand(Left,Pt,i));
			L->add(new RoadAddTrackPointCommand(Right,Pt,RightIndex));
			testIntersections(L,Left,i+2,Right,RightIndex);
			testIntersections(L,Left,i+2,Right,RightIndex+1);
			return;
		}
	}
}

void CreateRoundaboutInteraction::mousePressEvent(QMouseEvent * event)
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
			double Steps = ceil(2*3.141592/Angle);
			Angle = 2*3.141592/Steps;
			Radius *= view()->projection().pixelPerM();
			double Modifier = DockData.DriveRight->isChecked()?-1:1;		
			QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
			QPen TP(SomeBrush,projection().pixelPerM()*4);
			QPointF Prev(CenterF.x()+cos(Modifier*Angle/2)*Radius,CenterF.y()+sin(Modifier*Angle/2)*Radius);
			CommandList* L = new CommandList;
			TrackPoint* First = new TrackPoint(view()->projection().inverse(Prev));
			L->add(new AddFeatureCommand(Main->activeLayer(),First,true));
			Road* R = new Road;
			R->add(First);
			R->setTag("oneway","yes");
			for (double a = Angle*3/2; a<2*3.141592; a+=Angle)
			{
				QPointF Next(CenterF.x()+cos(Modifier*a)*Radius,CenterF.y()+sin(Modifier*a)*Radius);
				TrackPoint* New = new TrackPoint(view()->projection().inverse(Next));
				L->add(new AddFeatureCommand(Main->activeLayer(),New,true));
				R->add(New);
			}
			R->add(First);
			L->add(new AddFeatureCommand(Main->activeLayer(),R,true));
			for (FeatureIterator it(document()); !it.isEnd(); ++it)
			{
				Road* W1 = dynamic_cast<Road*>(it.get());
				if (W1 && (W1 != R))
					for (unsigned int i=1; i<W1->size(); ++i)
					{
						unsigned int Before = W1->size();
						testIntersections(L,R,1,W1,i);
						unsigned int After = W1->size();
						i += (After-Before);
					}
			}
			Main->properties()->setSelection(R);
			document()->history().add(L);
			view()->invalidate();
			view()->launch(0);
		}
	}
	else
		Interaction::mousePressEvent(event);
}

void CreateRoundaboutInteraction::mouseMoveEvent(QMouseEvent* event)
{
	LastCursor = event->pos();
	if (HaveCenter)
		view()->update();
	Interaction::mouseMoveEvent(event);
}

void CreateRoundaboutInteraction::paintEvent(QPaintEvent* , QPainter& thePainter)
{
	if (HaveCenter)
	{
		QPointF CenterF(view()->projection().project(Center));
		double Radius = distance(CenterF,LastCursor)/view()->projection().pixelPerM();
		double Precision = 1.99;
		if (Radius<2)
			Radius = 2;
		double Angle = 2*acos(1-Precision/Radius);
		double Steps = ceil(2*3.141592/Angle);
		Angle = 2*3.141592/Steps;
		Radius *= view()->projection().pixelPerM();
		double Modifier = DockData.DriveRight->isChecked()?-1:1;	
		QBrush SomeBrush(QColor(0xff,0x77,0x11,128));	
		QPen TP(SomeBrush,projection().pixelPerM()*4);
		QPointF Prev(CenterF.x()+cos(Modifier*Angle/2)*Radius,CenterF.y()+sin(Modifier*Angle/2)*Radius);
		for (double a = Angle*3/2; a<2*3.141592; a+=Angle)
		{
			QPointF Next(CenterF.x()+cos(Modifier*a)*Radius,CenterF.y()+sin(Modifier*a)*Radius);
			::draw(thePainter,TP,MapFeature::OneWay, Prev,Next,4,view()->projection());
			Prev = Next;
		}
		QPointF Next(CenterF.x()+cos(Modifier*Angle/2)*Radius,CenterF.y()+sin(Modifier*Angle/2)*Radius);
		::draw(thePainter,TP,MapFeature::OneWay, Prev,Next,4,view()->projection());
	}
}
