#ifndef MERKATOR_INTERACTION_H_
#define MERKATOR_INTERACTION_H_

class TrackPoint;
class MainWindow;
class Projection;
class TrackPoint;
class Way;

class QMouseEvent;
class QPaintEvent;
class QPainter;

#include "MainWindow.h"
#include "MapView.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"

#include <QtCore/QObject>
#include <QtCore/QTime>
#include <QtGui/QApplication>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>

#include <algorithm>

class Interaction : public QObject
{
	Q_OBJECT
	public:
		Interaction(MapView* theView);
		virtual ~Interaction();

		virtual void mousePressEvent(QMouseEvent * event);
		virtual void mouseReleaseEvent(QMouseEvent * event);
		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);

		virtual QCursor cursor() const;
		MapView* view();
		MapDocument* document();
		MainWindow* main();
		const Projection& projection() const;
		bool panning() const;
	private:
		MapView* theView;
		bool Panning;
		QPoint LastPan;
};

template<class FeatureType>
class GenericFeatureSnapInteraction : public Interaction
{
	public:
		GenericFeatureSnapInteraction(MapView* theView)
			: Interaction(theView), LastSnap(0), SnapActive(true),
			  NoSelectPoints(false), NoSelectRoads(false)
		{
		}

		virtual void paintEvent(QPaintEvent* , QPainter& thePainter)
		{
			if (LastSnap)
				LastSnap->drawFocus(thePainter, projection());
		}
		virtual void mousePressEvent(QMouseEvent * event)
		{
			updateSnap(event);
			snapMousePressEvent(event,LastSnap);
			Interaction::mousePressEvent(event);
		}
		virtual void mouseReleaseEvent(QMouseEvent * event)
		{
			updateSnap(event);
			snapMouseReleaseEvent(event,LastSnap);
			Interaction::mouseReleaseEvent(event);
		}
		virtual void mouseMoveEvent(QMouseEvent* event)
		{
			updateSnap(event);
			snapMouseMoveEvent(event, LastSnap);
			Interaction::mouseMoveEvent(event);
		}
		virtual void snapMousePressEvent(QMouseEvent * , FeatureType*)
		{
		}
		virtual void snapMouseReleaseEvent(QMouseEvent * , FeatureType*)
		{
		}
		virtual void snapMouseMoveEvent(QMouseEvent* , FeatureType*)
		{
		}
		void activateSnap(bool b)
		{
			SnapActive = b;
		}
		void addToNoSnap(FeatureType* F)
		{
			NoSnap.push_back(F);
		}
		void clearNoSnap()
		{
			NoSnap.clear();
		}
		void setDontSelectPoints(bool b)
		{
			NoSelectPoints = b;
		}
		void setDontSelectRoads(bool b)
		{
			NoSelectRoads = b;
		}
	private:
		void updateSnap(QMouseEvent* event)
		{
			if (panning())
			{
				LastSnap = 0;
				return;
			}
			bool NoRoads = 
				(QApplication::keyboardModifiers() & Qt::AltModifier | 
				QApplication::keyboardModifiers() & Qt::ShiftModifier);
			FeatureType* Prev = LastSnap;
			LastSnap = 0;
			if (!SnapActive) return;
			QTime Start(QTime::currentTime());
			CoordBox HotZone(projection().inverse(event->pos()-QPointF(5,5)),projection().inverse(event->pos()+QPointF(5,5)));
			double BestDistance = 5;
			for (VisibleFeatureIterator it(document()); !it.isEnd(); ++it)
			{
				FeatureType* Pt = dynamic_cast<FeatureType*>(it.get());
				if (Pt)
				{
					if ( (NoRoads || NoSelectRoads) && dynamic_cast<Road*>(Pt))
						continue;
					if (NoSelectPoints && dynamic_cast<TrackPoint*>(Pt))
						continue;
					if (std::find(NoSnap.begin(),NoSnap.end(),Pt) != NoSnap.end())
						continue;
					if (Pt->boundingBox().disjunctFrom(HotZone))
						continue;
					double Distance = Pt->pixelDistance(event->pos(), 5.01, projection());
					if (Distance < BestDistance)
					{
						BestDistance = Distance;
						LastSnap = Pt;
					}
				}
			}
/*			QTime Stop(QTime::currentTime());
			main()->statusBar()->clearMessage();
			main()->statusBar()->showMessage(QString("Update took %1ms").arg(Start.msecsTo(Stop))); */
			if (Prev != LastSnap)
				view()->update();
		}

		FeatureType* LastSnap;
		std::vector<FeatureType*> NoSnap;
		bool SnapActive;
		bool NoSelectPoints;
		bool NoSelectWays;
		bool NoSelectRoads;
};

typedef GenericFeatureSnapInteraction<MapFeature> FeatureSnapInteraction;
typedef GenericFeatureSnapInteraction<TrackPoint> TrackPointSnapInteraction;
typedef GenericFeatureSnapInteraction<Road> RoadSnapInteraction;

#endif


