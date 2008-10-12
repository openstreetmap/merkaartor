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
#include "InfoDock.h"
#include "PropertiesDock.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"

#include <QtCore/QObject>
#include <QtCore/QTime>
#include <QtGui/QApplication>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QList>

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
		QPoint FirstPan;
		QPoint LastPan;
	signals:
		void requestCustomContextMenu(const QPoint & pos);

	private:
		bool Dragging;
		Coord StartDrag;
		Coord EndDrag;
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

		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
		{
			Interaction::paintEvent(anEvent, thePainter);

			for (unsigned int i=0; i<view()->properties()->size(); ++i)
				if (document()->exists(view()->properties()->selection(i)))
					view()->properties()->selection(i)->drawFocus(thePainter, projection());

#ifndef _MOBILE
			if (LastSnap && document()->exists(LastSnap)) {
				LastSnap->drawHover(thePainter, projection());
				view()->setToolTip(LastSnap->toHtml());
			} else {
				view()->setToolTip("");
			}
#endif
		}
		virtual void mousePressEvent(QMouseEvent * event)
		{
			updateSnap(event);
			snapMousePressEvent(event,LastSnap);
#ifndef _MOBILE
			Interaction::mousePressEvent(event);
#endif
		}
		virtual void mouseReleaseEvent(QMouseEvent * event)
		{
			updateSnap(event);
			snapMouseReleaseEvent(event,LastSnap);
#ifndef _MOBILE
			Interaction::mouseReleaseEvent(event);
#endif
		}
		virtual void mouseMoveEvent(QMouseEvent* event)
		{
			updateSnap(event);
			snapMouseMoveEvent(event, LastSnap);
#ifndef _MOBILE
			Interaction::mouseMoveEvent(event);
#endif
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
		void clearSnap()
		{
			StackSnap.clear();
		}
		QList<MapFeature*> snapList()
		{
			return StackSnap;
		}
		void addSnap(MapFeature* aSnap)
		{
			StackSnap.append(aSnap);
		}
		void setSnap(QList<MapFeature*> aSnapList)
		{
			StackSnap = aSnapList;
			curStackSnap = 0;
		}
		void nextSnap()
		{
			curStackSnap++;
			if (curStackSnap > StackSnap.size() -1)
				curStackSnap = 0;
			view()->properties()->setSelection(StackSnap[curStackSnap]);
			view()->update();
		}
		void previousSnap()
		{
			curStackSnap--;
			if (curStackSnap < 0)
				curStackSnap = StackSnap.size() -1;
			view()->properties()->setSelection(StackSnap[curStackSnap]);
			view()->update();
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
				( (QApplication::keyboardModifiers() & Qt::AltModifier) &&  (QApplication::keyboardModifiers() &Qt::ControlModifier) );
			FeatureType* Prev = LastSnap;
			LastSnap = 0;
			if (!SnapActive) return;
			//QTime Start(QTime::currentTime());
			CoordBox HotZone(projection().inverse(event->pos()-QPointF(5,5)),projection().inverse(event->pos()+QPointF(5,5)));
			SnapList.clear();
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
					SnapList.push_back(Pt);
					if (Distance < BestDistance)
					{
						BestDistance = Distance;
						LastSnap = Pt;
					}
				}
			}
			if (Prev != LastSnap) {
				curStackSnap = SnapList.indexOf(LastSnap);
				view()->update();
			}

			if (M_PREFS->getMapTooltip()) {
				if (LastSnap)
					view()->setToolTip(LastSnap->toHtml());
				else
					view()->setToolTip("");
			}
			if (M_PREFS->getInfoOnHover() && main()->info()->isVisible()) {
				if (LastSnap) {
					main()->info()->setHoverHtml(LastSnap->toHtml());
				} else
					main()->info()->unsetHoverHtml();
			}
		}

		FeatureType* LastSnap;
		std::vector<FeatureType*> NoSnap;
		bool SnapActive;
		bool NoSelectPoints;
		bool NoSelectWays;
		bool NoSelectRoads;

	protected:
		QList<MapFeature*> StackSnap;
		QList<MapFeature*> SnapList;
		int curStackSnap;
};

typedef GenericFeatureSnapInteraction<MapFeature> FeatureSnapInteraction;
typedef GenericFeatureSnapInteraction<TrackPoint> TrackPointSnapInteraction;
typedef GenericFeatureSnapInteraction<Road> RoadSnapInteraction;

#endif


