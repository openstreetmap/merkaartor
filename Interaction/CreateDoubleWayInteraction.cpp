#include "Interaction/CreateDoubleWayInteraction.h"
#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Map/Painting.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Map/Way.h"
#include "Utils/LineF.h"
#include "MainWindow.h"

#include <QtCore/QSettings>
#include <QtGui/QDockWidget>
#include <QtGui/QPainter>

CreateDoubleWayInteraction::CreateDoubleWayInteraction(MainWindow* aMain, MapView* aView)
	: Interaction(aView), Main(aMain), R1(0), R2(0), FirstPoint(0,0), HaveFirst(false)
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
	DockData.RoadDistance->setText(Sets.value("doubleroaddistance","20").toString());
}

CreateDoubleWayInteraction::~CreateDoubleWayInteraction()
{
	QSettings Sets;
	Sets.beginGroup("roadstructure");
	Sets.setValue("rightsidedriving",DockData.DriveRight->isChecked());
	Sets.setValue("doubleroaddistance",DockData.RoadDistance->text());
	delete theDock;
	view()->update();
}

void CreateDoubleWayInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
	double rB = view()->projection().pixelPerM()*DockData.RoadDistance->text().toDouble()/2;
	if (!HaveFirst)
	{
		thePainter.setPen(QColor(0,0,0));
		thePainter.drawEllipse(LastCursor.x()-rB,LastCursor.y()-rB,rB*2,rB*2);
	}
	else
	{
		Coord PreviousPoint = FirstPoint;
		if (distance(view()->projection().project(PreviousPoint), LastCursor) > 1)
		{
			double rA = FirstDistance * view()->projection().pixelPerM()/2;
			LineF FA1(view()->projection().project(PreviousPoint),LastCursor);
			LineF FA2(FA1);
			LineF FB1(FA1);
			LineF FB2(FA1);
			FA1.slide(-rA);
			FA2.slide(rA);
			FB1.slide(-rB);
			FB2.slide(rB);
			QPointF A1(FA1.project(view()->projection().project(PreviousPoint)));
			QPointF A2(FA2.project(view()->projection().project(PreviousPoint)));
			QPointF B1(FB1.project(LastCursor));
			QPointF B2(FB2.project(LastCursor));


			QPen TP(QBrush(QColor(0xff,0x77,0x11,128)),projection().pixelPerM()*4);
			if (DockData.DriveRight->isChecked())
			{
				::draw(thePainter,TP,MapFeature::OneWay, B1,A1,rB/4,view()->projection());
				::draw(thePainter,TP,MapFeature::OneWay, A2,B2,rB/4,view()->projection());
			}
			else
			{
				::draw(thePainter,TP,MapFeature::OneWay, A1,B1,rB/4,view()->projection());
				::draw(thePainter,TP,MapFeature::OneWay, B2,A2,rB/4,view()->projection());
			}
		}
	}
}

void CreateDoubleWayInteraction::mouseMoveEvent(QMouseEvent* event)
{
	LastCursor = event->pos();
	Interaction::mouseMoveEvent(event);
	view()->update();
	Interaction::mouseMoveEvent(event);
}

void CreateDoubleWayInteraction::mousePressEvent(QMouseEvent* anEvent)
{
	if (anEvent->buttons() & Qt::LeftButton)
	{
		if (!HaveFirst)
		{
			HaveFirst = true;
			FirstPoint = view()->projection().inverse(anEvent->pos());
			FirstDistance = DockData.RoadDistance->text().toDouble();
		}
		else if (R1)
		{
			Way* W1 = R1->get(R1->size()-1);
			Way* W2 = R2->get(0);
			LineF P1(view()->projection().project(W1->from()->position()),view()->projection().project(W1->to()->position()));
			LineF P2(view()->projection().project(W2->to()->position()),view()->projection().project(W2->from()->position()));

			Coord PreviousPoint = FirstPoint;
			if (distance(view()->projection().project(PreviousPoint), LastCursor) > 1)
			{
				double rB = view()->projection().pixelPerM()*DockData.RoadDistance->text().toDouble()/2;
				double rA = FirstDistance * view()->projection().pixelPerM()/2;
				LineF FA1(view()->projection().project(PreviousPoint),LastCursor);
				LineF FA2(FA1);
				LineF FB1(FA1);
				LineF FB2(FA1);
				double Modifier = DockData.DriveRight->isChecked()?1:-1;
				FA1.slide(rA*Modifier);
				FA2.slide(-rA*Modifier);
				FB1.slide(rB*Modifier);
				FB2.slide(-rB*Modifier);
				LineF N1(FA1.project(view()->projection().project(PreviousPoint)), FB1.project(LastCursor));
				LineF N2(FA2.project(view()->projection().project(PreviousPoint)), FB2.project(LastCursor));

				TrackPoint* A1;
				TrackPoint* A2;
				CommandList* L = new CommandList;
				A1 = W1->to();
				A2 = W2->from();
				L->add(new MoveTrackPointCommand(A1,view()->projection().inverse(
					P1.intersectionWith(N1))));
				L->add(new MoveTrackPointCommand(A2,view()->projection().inverse(
					P2.intersectionWith(N2))));
				TrackPoint* B1 = new TrackPoint(view()->projection().inverse(
					FB1.project(LastCursor)));
				TrackPoint* B2 = new TrackPoint(view()->projection().inverse(
					FB2.project(LastCursor)));

				W1 = new Way(A1,B1);
				W2 = new Way(B2,A2);
				L->add(new AddFeatureCommand(Main->activeLayer(),B1,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),B2,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),W1,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),W2,true));
				L->add(new RoadAddWayCommand(R1,W1));
				L->add(new RoadAddWayCommand(R2,W2,0));
				document()->history().add(L);
				view()->invalidate();
				FirstPoint = view()->projection().inverse(anEvent->pos());
				FirstDistance = DockData.RoadDistance->text().toDouble();
			}
		}
		else
		{
			Coord PreviousPoint = FirstPoint;
			if (distance(view()->projection().project(PreviousPoint), LastCursor) > 1)
			{
				double rB = view()->projection().pixelPerM()*DockData.RoadDistance->text().toDouble()/2;
				double rA = FirstDistance * view()->projection().pixelPerM()/2;
				LineF FA1(view()->projection().project(PreviousPoint),LastCursor);
				LineF FA2(FA1);
				LineF FB1(FA1);
				LineF FB2(FA1);
				double Modifier = DockData.DriveRight->isChecked()?1:-1;
				FA1.slide(rA*Modifier);
				FA2.slide(-rA*Modifier);
				FB1.slide(rB*Modifier);
				FB2.slide(-rB*Modifier);

				TrackPoint* A1 = new TrackPoint(view()->projection().inverse(
					FA1.project(view()->projection().project(PreviousPoint))));
				TrackPoint* A2 = new TrackPoint(view()->projection().inverse(
					FA2.project(view()->projection().project(PreviousPoint))));
				TrackPoint* B1 = new TrackPoint(view()->projection().inverse(
					FB1.project(LastCursor)));
				TrackPoint* B2 = new TrackPoint(view()->projection().inverse(
					FB2.project(LastCursor)));

				Way* W1;
				Way* W2;
				W1 = new Way(A1,B1);
				W2 = new Way(B2,A2);
				R1 = new Road;
				R2 = new Road;
				R1->setTag("oneway","yes");
				R2->setTag("oneway","yes");
				R1->add(W1);
				R2->add(W2);
				CommandList* L = new CommandList;
				L->add(new AddFeatureCommand(Main->activeLayer(),A1,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),A2,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),B1,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),B2,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),W1,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),W2,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),R1,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),R2,true));
				document()->history().add(L);
				view()->invalidate();
				FirstPoint = view()->projection().inverse(anEvent->pos());
				FirstDistance = DockData.RoadDistance->text().toDouble();
			}
		}
	}
	else
		Interaction::mousePressEvent(anEvent);
}
