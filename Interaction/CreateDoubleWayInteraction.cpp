#include "Interaction/CreateDoubleWayInteraction.h"
#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Map/Painting.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Utils/LineF.h"
#include "MainWindow.h"
#include "Preferences/MerkaartorPreferences.h"

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
	DockData.DriveRight->setChecked(MerkaartorPreferences::instance()->getRightSideDriving());
	DockData.RoadDistance->setText(QString().setNum(MerkaartorPreferences::instance()->getDoubleRoadDistance()));
}

CreateDoubleWayInteraction::~CreateDoubleWayInteraction()
{
	MerkaartorPreferences::instance()->setRightSideDriving(DockData.DriveRight->isChecked());
	MerkaartorPreferences::instance()->setDoubleRoadDistance(DockData.RoadDistance->text().toDouble());

	delete theDock;
	view()->update();
}

void CreateDoubleWayInteraction::paintEvent(QPaintEvent* /* anEvent */, QPainter& thePainter)
{
	double rB = view()->projection().pixelPerM()*DockData.RoadDistance->text().toDouble()/2;
	if (!HaveFirst)
	{
		thePainter.setPen(QColor(0,0,0));
		thePainter.drawEllipse(int(LastCursor.x()-rB),int(LastCursor.y()-rB),int(rB*2),int(rB*2));
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

      QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
			QPen TP(SomeBrush,projection().pixelPerM()*4);
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
			unsigned int i1 = R1->size()-1;
			unsigned int i2 = 1;
			LineF P1(
				view()->projection().project(R1->get(i1-1)->position()),
				view()->projection().project(R1->get(i1)->position()));
			LineF P2(
				view()->projection().project(R2->get(i2-1)->position()),
				view()->projection().project(R2->get(i2)->position()));

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
				A1 = R1->get(i1);
				A2 = R2->get(i2-1);
				L->add(new MoveTrackPointCommand(A1,view()->projection().inverse(
					P1.intersectionWith(N1))));
				L->add(new MoveTrackPointCommand(A2,view()->projection().inverse(
					P2.intersectionWith(N2))));
				TrackPoint* B1 = new TrackPoint(view()->projection().inverse(
					FB1.project(LastCursor)));
				TrackPoint* B2 = new TrackPoint(view()->projection().inverse(
					FB2.project(LastCursor)));

				B1->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				B2->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				L->add(new AddFeatureCommand(Main->activeLayer(),B1,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),B2,true));
				L->add(new RoadAddTrackPointCommand(R1,B1));
				L->add(new RoadAddTrackPointCommand(R2,B2,0));
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
				CommandList* L = new CommandList;
				A1->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				A2->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				B1->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				B2->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				L->add(new AddFeatureCommand(Main->activeLayer(),A1,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),A2,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),B1,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),B2,true));

				R1 = new Road;
				R2 = new Road;
				L->add(new AddFeatureCommand(Main->activeLayer(),R1,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),R2,true));
				R1->setTag("oneway","yes");
				R1->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				R2->setTag("oneway","yes");
				R2->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				L->add(new RoadAddTrackPointCommand(R1,A1));
				L->add(new RoadAddTrackPointCommand(R1,B1));
				L->add(new RoadAddTrackPointCommand(R2,B2));
				L->add(new RoadAddTrackPointCommand(R2,A2));
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

QCursor CreateDoubleWayInteraction::cursor() const
{
	return QCursor(Qt::CrossCursor);
}
