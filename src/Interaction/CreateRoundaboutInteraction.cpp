#include "Interaction/CreateRoundaboutInteraction.h"
#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Maps/Painting.h"
#include "Maps/Road.h"
#include "Maps/TrackPoint.h"
#include "Utils/LineF.h"
#include "PropertiesDock.h"
#include "Preferences/MerkaartorPreferences.h"

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
	DockData.DriveRight->setChecked(MerkaartorPreferences::instance()->getRightSideDriving());
}

CreateRoundaboutInteraction::~CreateRoundaboutInteraction()
{
	MerkaartorPreferences::instance()->setRightSideDriving(DockData.DriveRight->isChecked());
	delete theDock;
	view()->update();
}

QString CreateRoundaboutInteraction::toHtml()
{
	QString help;
	//help = (MainWindow::tr("LEFT-CLICK to select; LEFT-DRAG to move"));

	QString desc;
	desc = QString("<big><b>%1</b></big><br/>").arg(MainWindow::tr("Create roundabout Interaction"));
	desc += QString("<b>%1</b><br/>").arg(help);

	QString S =
	"<html><head/><body>"
	"<small><i>" + QString(metaObject()->className()) + "</i></small><br/>"
	+ desc;
	S += "</body></html>";

	return S;
}

void CreateRoundaboutInteraction::testIntersections(CommandList* L, Road* Left, int FromIdx, Road* Right, int RightIndex)
{
	LineF L1(COORD_TO_XY(Right->getNode(RightIndex-1)),
		COORD_TO_XY(Right->getNode(RightIndex)));
	for (int i=FromIdx; i<Left->size(); ++i)
	{
		LineF L2(COORD_TO_XY(Left->getNode(i-1)),
			COORD_TO_XY(Left->getNode(i)));
		QPointF Intersection(L1.intersectionWith(L2));
		if (L1.segmentContains(Intersection) && L2.segmentContains(Intersection))
		{
			TrackPoint* Pt = new TrackPoint(XY_TO_COORD(Intersection));
			L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),Pt,true));
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
			Center = XY_TO_COORD(event->pos());
		}
		else
		{
			QPointF CenterF(COORD_TO_XY(Center));
			double Radius = distance(CenterF,LastCursor)/view()->pixelPerM();
			double Precision = 2.49;
			if (Radius<2.5)
				Radius = 2.5;
			double Angle = 2*acos(1-Precision/Radius);
			double Steps = ceil(2*M_PI/Angle);
			Angle = 2*M_PI/Steps;
			Radius *= view()->pixelPerM();
			double Modifier = DockData.DriveRight->isChecked()?-1:1;
			QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
			QPen TP(SomeBrush,view()->pixelPerM()*4);
			QPointF Prev(CenterF.x()+cos(Modifier*Angle/2)*Radius,CenterF.y()+sin(Modifier*Angle/2)*Radius);
			TrackPoint* First = new TrackPoint(XY_TO_COORD(Prev));
			Road* R = new Road;
			R->add(First);
			// "oneway" is implied on roundabouts
			//R->setTag("oneway","yes");
			R->setTag("junction","roundabout");
			if (M_PREFS->apiVersionNum() < 0.6)
				R->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
			CommandList* L  = new CommandList(MainWindow::tr("Create Roundabout %1").arg(R->id()), R);
			L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),First,true));
			for (double a = Angle*3/2; a<2*M_PI; a+=Angle)
			{
				QPointF Next(CenterF.x()+cos(Modifier*a)*Radius,CenterF.y()+sin(Modifier*a)*Radius);
				TrackPoint* New = new TrackPoint(XY_TO_COORD(Next));
				if (M_PREFS->apiVersionNum() < 0.6)
					New->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
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
		QPointF CenterF(COORD_TO_XY(Center));
		double Radius = distance(CenterF,LastCursor)/view()->pixelPerM();
		double Precision = 1.99;
		if (Radius<2)
			Radius = 2;
		double Angle = 2*acos(1-Precision/Radius);
		double Steps = ceil(2*M_PI/Angle);
		Angle = 2*M_PI/Steps;
		Radius *= view()->pixelPerM();
		double Modifier = DockData.DriveRight->isChecked()?-1:1;
		QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
		QPen TP(SomeBrush,view()->pixelPerM()*4);
		QPointF Prev(CenterF.x()+cos(Modifier*Angle/2)*Radius,CenterF.y()+sin(Modifier*Angle/2)*Radius);
		for (double a = Angle*3/2; a<2*M_PI; a+=Angle)
		{
			QPointF Next(CenterF.x()+cos(Modifier*a)*Radius,CenterF.y()+sin(Modifier*a)*Radius);
			::draw(thePainter,TP,MapFeature::OneWay, Prev,Next,4,view()->projection());
			Prev = Next;
		}
		QPointF Next(CenterF.x()+cos(Modifier*Angle/2)*Radius,CenterF.y()+sin(Modifier*Angle/2)*Radius);
		::draw(thePainter,TP,MapFeature::OneWay, Prev,Next,4,view()->projection());
	}
}

#ifndef Q_OS_SYMBIAN
QCursor CreateRoundaboutInteraction::cursor() const
{
	return QCursor(Qt::CrossCursor);
}
#endif
