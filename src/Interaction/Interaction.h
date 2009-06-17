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
#include "Maps/MapDocument.h"
#include "Maps/MapFeature.h"
#include "Maps/Road.h"
#include "Maps/TrackPoint.h"

#include <QtCore/QObject>
#include <QtCore/QTime>
#include <QtGui/QApplication>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QList>

#include <algorithm>

#define XY_TO_COORD(x)  projection().inverse(transform().inverted().map(QPointF(x)))
#define COORD_TO_XY(x)  transform().map(projection().project(x)).toPoint()

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
		virtual QString toHtml() = 0;

#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif
		MapView* view();
		MapDocument* document();
		MainWindow* main();
		const Projection& projection() const;
		const QTransform& transform() const;
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

			for (int i=0; i<view()->properties()->size(); ++i)
				if (document()->exists(view()->properties()->selection(i)))
					view()->properties()->selection(i)->drawFocus(thePainter, projection(), transform());

#ifndef _MOBILE
			if (LastSnap && document()->exists(LastSnap)) {
				LastSnap->drawHover(thePainter, projection(), transform());
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
			if (!(M_PREFS->getMouseSingleButton() && LastSnap))
				Interaction::mousePressEvent(event);
		}
		virtual void mouseReleaseEvent(QMouseEvent * event)
		{
			updateSnap(event);
			snapMouseReleaseEvent(event,LastSnap);
			if (!(M_PREFS->getMouseSingleButton() && LastSnap))
				Interaction::mouseReleaseEvent(event);
		}
		virtual void mouseMoveEvent(QMouseEvent* event)
		{
			updateSnap(event);
			snapMouseMoveEvent(event, LastSnap);
			if (!(M_PREFS->getMouseSingleButton() && LastSnap))
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
		void clearSnap()
		{
			StackSnap.clear();
		}
		void clearLastSnap()
		{
			LastSnap = 0;
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
			CoordBox HotZone(XY_TO_COORD(event->pos()-QPointF(15,15)),XY_TO_COORD(event->pos()+QPointF(15,15)));
			SnapList.clear();
			double BestDistance = 5;
#if 1
			//ggl::box < Coord > cb(HotZone.bottomLeft(), HotZone.topRight());

			for (int j=0; j<document()->layerSize(); ++j) {
				if (!document()->getLayer(j)->isVisible() || document()->getLayer(j)->isReadonly())
					continue;

				std::deque < MapFeaturePtr > ret = document()->getLayer(j)->getRTree()->find(HotZone);
				for (std::deque < MapFeaturePtr >::const_iterator it = ret.begin(); it < ret.end(); ++it) {
					FeatureType* Pt = dynamic_cast<FeatureType*>(*it);
					if (Pt)
					{
						if (Pt->notEverythingDownloaded())
							continue;
						if ( (NoRoads || NoSelectRoads) && dynamic_cast<Road*>(Pt))
							continue;
						if (NoSelectPoints && dynamic_cast<TrackPoint*>(Pt))
							continue;
						if (std::find(NoSnap.begin(),NoSnap.end(),Pt) != NoSnap.end())
							continue;
						double Distance = Pt->pixelDistance(event->pos(), 5.01, projection(), transform());
						SnapList.push_back(Pt);
						if (Distance < BestDistance)
						{
							BestDistance = Distance;
							LastSnap = Pt;
						}
					}
				}
			}

#else
			for (VisibleFeatureIterator it(document()); !it.isEnd(); ++it)
			{
				FeatureType* Pt = dynamic_cast<FeatureType*>(it.get());
				if (Pt)
				{
					if (Pt->layer()->isReadonly())
						continue;
					if (Pt->notEverythingDownloaded())
						continue;
					if ( (NoRoads || NoSelectRoads) && dynamic_cast<Road*>(Pt))
						continue;
					if (NoSelectPoints && dynamic_cast<TrackPoint*>(Pt))
						continue;
					if (std::find(NoSnap.begin(),NoSnap.end(),Pt) != NoSnap.end())
						continue;
					if (Pt->boundingBox().disjunctFrom(HotZone))
						continue;
					double Distance = Pt->pixelDistance(transform().inverted().map(event->pos()), 5.01, projection());
					SnapList.push_back(Pt);
					if (Distance < BestDistance)
					{
						BestDistance = Distance;
						LastSnap = Pt;
					}
				}
			}
#endif
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
		QList<FeatureType*> NoSnap;
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


