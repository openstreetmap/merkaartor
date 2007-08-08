#include "Interaction/CreateRoundaboutInteraction.h"
#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Map/Painting.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Map/Way.h"
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

void CreateRoundaboutInteraction::testIntersections(CommandList* L, Road* R, unsigned int FromIdx, Way* W1, double Radius)
{
	LineF L1(view()->projection().project(W1->from()->position()),
		view()->projection().project(W1->to()->position()));
	for (unsigned int i=FromIdx; i<R->size(); ++i)
	{
		Way* W2 = R->get(i);
		LineF L2(view()->projection().project(W2->from()->position()),
			view()->projection().project(W2->to()->position()));
		QPointF Intersection(L1.intersectionWith(L2));
		if (L1.segmentContains(Intersection) && L2.segmentContains(Intersection))
		{
			TrackPoint* Pt = new TrackPoint(view()->projection().inverse(Intersection));
			Way* W1A = new Way(W1->from(),Pt);
			Way* W1B = new Way(Pt,W1->to());
			L->add(new AddFeatureCommand(Main->activeLayer(),Pt,true));
			L->add(new AddFeatureCommand(Main->activeLayer(),W1A,true));
			L->add(new AddFeatureCommand(Main->activeLayer(),W1B,true));
			std::vector<MapFeature*> Alternatives;
			Alternatives.push_back(W1A);
			Alternatives.push_back(W1B);
			L->add(new RemoveFeatureCommand(document(),W1,Alternatives));
			Way* W2A = new Way(W2->from(),Pt);
			Way* W2B = new Way(Pt,W2->to());
			L->add(new AddFeatureCommand(Main->activeLayer(),W2A,true));
			L->add(new AddFeatureCommand(Main->activeLayer(),W2B,true));
			Alternatives.clear();
			Alternatives.push_back(W2A);
			Alternatives.push_back(W2B);
			L->add(new RemoveFeatureCommand(document(),W2,Alternatives));
			testIntersections(L,R,i+2,W1A,Radius);
			testIntersections(L,R,i+2,W1B,Radius);
			return;
		}
	}
	if (FromIdx)
	{
		QPointF CenterF(view()->projection().project(Center));
		QPointF FromF = view()->projection().project(W1->from()->position());
		double Dist = distance(CenterF,FromF);
		if (Dist > Radius) return;
		QPointF ToF = view()->projection().project(W1->to()->position());
		Dist = distance(CenterF,ToF);
		if (Dist > Radius) return;
		L->add(new RemoveFeatureCommand(document(),W1,std::vector<MapFeature*>()));
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
			QPen TP(QBrush(QColor(0xff,0x77,0x11,128)),projection().pixelPerM()*4);
			QPointF Prev(CenterF.x()+cos(Modifier*Angle/2)*Radius,CenterF.y()+sin(Modifier*Angle/2)*Radius);
			CommandList* L = new CommandList;
			TrackPoint* First = new TrackPoint(view()->projection().inverse(Prev));
			L->add(new AddFeatureCommand(Main->activeLayer(),First,true));
			TrackPoint* Last = First;
			Road* R = new Road;
			R->setTag("oneway","yes");
			for (double a = Angle*3/2; a<2*3.141592; a+=Angle)
			{
				QPointF Next(CenterF.x()+cos(Modifier*a)*Radius,CenterF.y()+sin(Modifier*a)*Radius);
				TrackPoint* New = new TrackPoint(view()->projection().inverse(Next));
				L->add(new AddFeatureCommand(Main->activeLayer(),New,true));
				Way* W = new Way(Last,New);
				R->add(W);
				L->add(new AddFeatureCommand(Main->activeLayer(),W,true));
				Last = New;
			}
			Way* W = new Way(Last,First);
			L->add(new AddFeatureCommand(Main->activeLayer(),W,true));
			R->add(W);
			L->add(new AddFeatureCommand(Main->activeLayer(),R,true));
			std::vector<Way*> ToTest;
			for (FeatureIterator it(document()); !it.isEnd(); ++it)
			{
				Way* W1 = dynamic_cast<Way*>(it.get());
				if (W1 && !W1->isPartOf(R))
					ToTest.push_back(W1);
			}
			for (unsigned int i=0; i<ToTest.size(); ++i)
			{
				Way* W1 = ToTest[i];
				testIntersections(L,R,0,W1,Radius);
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

void CreateRoundaboutInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
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
		QPen TP(QBrush(QColor(0xff,0x77,0x11,128)),projection().pixelPerM()*4);
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
