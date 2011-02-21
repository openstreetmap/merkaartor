#include "Interaction.h"

#include "MapView.h"
#include "Document.h"
#include "Projection.h"
#include "Node.h"
#include "PropertiesDock.h"
#include "Utils.h"
#include "Global.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include <math.h>

#define CLEAR_DISTANCE 7.01

Interaction::Interaction(MapView* aView)
    : theView(aView), Panning(false)
    , LastSnap(0), SnapActive(true)
    , NoSelectPoints(false), NoSelectRoads(false), NoSelectVirtuals(true)
    , Dragging(false)
    , StartDrag(0,0), EndDrag(0,0)
{
    connect(this, SIGNAL(requestCustomContextMenu(const QPoint &)), theView, SLOT(on_customContextMenuRequested(const QPoint &)));
}

Interaction::~Interaction()
{
}

bool Interaction::panning() const
{
    return (Panning && (LastPan != FirstPan));
}

MainWindow* Interaction::main()
{
    return theView->main();
}

MapView* Interaction::view()
{
    return theView;
}

Document* Interaction::document()
{
    return theView->document();
}

void Interaction::mousePressEvent(QMouseEvent * anEvent)
{
    if (anEvent->buttons() & Qt::MidButton) {
        Panning = true;
        FirstPan = LastPan = anEvent->pos();
    } else
#if defined(Q_OS_MAC)
    // In the name of beautifull code, Steve, add a right mouse button
    if (	(anEvent->modifiers() & Qt::MetaModifier) ||
            (M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::LeftButton)) ||
            (!M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::RightButton))
        )
#else
    if (
        (M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::LeftButton)) ||
        (!M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::RightButton))
        )
#endif // Q_OS_MAC
    {
        if (anEvent->modifiers() & Qt::ControlModifier) {
            EndDrag = StartDrag = XY_TO_COORD(anEvent->pos());
            Dragging = true;
        } else
        if (anEvent->modifiers() & Qt::ShiftModifier) {
        } else {
            Panning = true;
            FirstPan = LastPan = anEvent->pos();
        }
    }
}

void Interaction::mouseReleaseEvent(QMouseEvent * anEvent)
{
    if (Panning) {
        if (FirstPan != LastPan)
            view()->invalidate(true, true);
#ifndef _MOBILE
        else
            if (anEvent->button() == Qt::RightButton)
                emit(requestCustomContextMenu(anEvent->pos()));
#endif
        Panning = false;
    } else
    if (Dragging)
    {
        CoordBox DragBox(StartDrag,XY_TO_COORD(anEvent->pos()));
        if (!DragBox.isEmpty()) {
            view()->setViewport(DragBox,view()->rect());
            view()->invalidate(true, true);
            view()->launch(0);
        }
        Dragging = false;
    }
}

void Interaction::mouseMoveEvent(QMouseEvent* anEvent)
{
    if (anEvent->buttons() & Qt::MidButton) {
        if (Panning)
        {
            QPoint Delta = LastPan;
            Delta -= anEvent->pos();
            view()->panScreen(-Delta);
            LastPan = anEvent->pos();
#if defined(ENABLE_NVIDIA_HACK)
            view()->invalidate(true, false);
#endif // ENABLE_NVIDIA_HACK
        }
    } else

#if defined(Q_OS_MAC)
    // In the name of beautifull code, Steve, add a right mouse button
    if (	(anEvent->modifiers() & Qt::MetaModifier) ||
            (M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::LeftButton)) ||
            (!M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::RightButton))
        )
#else
    if (
        (M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::LeftButton)) ||
        (!M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::RightButton))
        )
#endif // Q_OS_MAC
    {
        if (Panning)
        {
            QPoint Delta = LastPan;
            Delta -= anEvent->pos();
            view()->panScreen(-Delta);
            LastPan = anEvent->pos();
#if defined(ENABLE_NVIDIA_HACK)
            view()->invalidate(true, false);
#endif // ENABLE_NVIDIA_HACK
        } else
        if (Dragging)
        {
            EndDrag = XY_TO_COORD(anEvent->pos());
            view()->update();
        }
    }
}

void Interaction::mouseDoubleClickEvent(QMouseEvent* /*anEvent*/)
{
}

void Interaction::wheelEvent(QWheelEvent* ev)
{
    qreal finalZoom = 1.;
    int Steps = ev->delta() / 120;
    if (Steps > 0) {
        for (int i = 0; i < Steps; ++i) {
            finalZoom *= M_PREFS->getZoomIn()/100.0;
        }
    } else if (Steps < 0) {
        for (int i = 0; i < -Steps; ++i) {
            finalZoom *= M_PREFS->getZoomOut()/100.0;
        }
    }

    // Do not overzoom in one go (circular scroll on touchpads can scroll very
    // fast). Without this check we can end up scaling the background by e.g.
    // 40 times in each direction and running out of memory.
    if (finalZoom > 2.0)
        finalZoom = 2.0;
    else if (finalZoom < 0.5)
        finalZoom = 0.5;

    view()->zoom(finalZoom, ev->pos());
    view()->invalidate(true, true);
}

void Interaction::paintEvent(QPaintEvent*, QPainter& thePainter)
{
    if (Dragging)
    {
        thePainter.setPen(QPen(QColor(0,0,255),1,Qt::DotLine));
        thePainter.drawRect(QRectF(COORD_TO_XY(StartDrag),COORD_TO_XY(EndDrag)));
    }
}

void Interaction::updateSnap(QMouseEvent* event)
{
    if (panning())
    {
        LastSnap = 0;
        return;
    }
    bool NoRoads =
        ( (QApplication::keyboardModifiers() & Qt::AltModifier) &&  (QApplication::keyboardModifiers() &Qt::ControlModifier) );
    Feature* Prev = LastSnap;
    LastSnap = 0;
    Feature* ReadOnlySnap = 0;
    if (!SnapActive) return;
    //QTime Start(QTime::currentTime());
    CoordBox HotZone(XY_TO_COORD(event->pos()-QPoint(M_PREFS->getMaxGeoPicWidth()+5,M_PREFS->getMaxGeoPicWidth()+5)),XY_TO_COORD(event->pos()+QPoint(M_PREFS->getMaxGeoPicWidth()+5,M_PREFS->getMaxGeoPicWidth()+5)));
    CoordBox HotZoneSnap(XY_TO_COORD(event->pos()-QPoint(15,15)),XY_TO_COORD(event->pos()+QPoint(15,15)));
    SnapList.clear();
    qreal BestDistance = 5;
    qreal BestReadonlyDistance = 5;
    bool areNodesSelectable = (theView->nodeWidth() >= 1 && M_PREFS->getTrackPointsVisible());

    Way* R;
    Node* N;
    for (int j=0; j<document()->layerSize(); ++j) {
        QList < Feature* > ret = g_backend.indexFind(document()->getLayer(j), HotZone);
        foreach(Feature* F, ret) {
            if (F)
            {
                if (F->isHidden())
                    continue;
                if (std::find(NoSnap.begin(),NoSnap.end(),F) != NoSnap.end())
                    continue;
                if (F->notEverythingDownloaded())
                    continue;
                if ((R = CAST_WAY(F))) {
                    if ( NoRoads || NoSelectRoads)
                        continue;

                    if (HotZoneSnap.contains(R->boundingBox()))
                        SnapList.push_back(F);
                    else {
                        QPointF lastPoint = R->getNode(0)->position();
                        QPointF aP;
                        for (int j=1; j<R->size(); ++j) {
                            aP = R->getNode(j)->position();
                            QLineF l(lastPoint, aP);
                            QPointF a, b;
                            if (Utils::QRectInterstects(HotZoneSnap, l, a, b)) {
                                SnapList.push_back(F);
                                break;
                            }
                            lastPoint = aP;
                        }
                    }
                }
                if ((N = CAST_NODE(F))) {
                    if (NoSelectPoints)
                        continue;
                    if (!N->isSelectable(theView))
                        continue;
                    if (HotZoneSnap.contains(N->boundingBox()))
                        SnapList.push_back(F);
                }

                qreal Distance = F->pixelDistance(event->pos(), CLEAR_DISTANCE, areNodesSelectable, view());
                if (Distance < BestDistance && !F->isReadonly())
                {
                    BestDistance = Distance;
                    LastSnap = F;
                } else if (Distance < BestReadonlyDistance && F->isReadonly())
                {
                    BestReadonlyDistance = Distance;
                    ReadOnlySnap = F;
                }
            }
        }
    }
    if (areNodesSelectable) {
        R = CAST_WAY(LastSnap);
        if (R) {
            Node* N = R->pixelDistanceNode(event->pos(), CLEAR_DISTANCE, view(), NoSelectVirtuals);
            if (N)
                LastSnap = N;
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
    if (M_PREFS->getInfoOnHover() && main()->info() && main()->info()->isVisible()) {
        if (LastSnap) {
            main()->info()->setHoverHtml(LastSnap->toHtml());
        } else
            if (ReadOnlySnap)
                main()->info()->setHoverHtml(ReadOnlySnap->toHtml());
            else
                main()->info()->unsetHoverHtml();
    }
}

Feature* Interaction::lastSnap()
{
    return LastSnap;
}


/***************/

FeatureSnapInteraction::FeatureSnapInteraction(MapView* theView)
        : Interaction(theView)
{
//    handCursor = QCursor(QPixmap(":/Icons/grab.png"));
//    grabCursor = QCursor(QPixmap(":/Icons/grabbing.png"));
    handCursor = QCursor(Qt::OpenHandCursor);
    grabCursor = QCursor(Qt::ClosedHandCursor);
    defaultCursor = QCursor(Qt::ArrowCursor);
//    warningCursor = QCursor(Qt::ForbiddenCursor);
    warningCursor = QCursor(QPixmap(":/Icons/cursor-warning"), 16, 5);

#ifndef _MOBILE
    theView->setCursor(cursor());
#endif
}

void FeatureSnapInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
    Interaction::paintEvent(anEvent, thePainter);

#ifndef _MOBILE
    for (int i=0; i<main()->features()->highlightedSize(); ++i)
        if (document()->exists(main()->features()->highlighted(i)))
            main()->features()->highlighted(i)->drawHighlight(thePainter, view());
    for (int i=0; i<main()->properties()->size(); ++i)
        if (document()->exists(main()->properties()->selection(i)))
            main()->properties()->selection(i)->drawFocus(thePainter, view());
    for (int i=0; i<main()->properties()->highlightedSize(); ++i)
        if (document()->exists(main()->properties()->highlighted(i)))
            main()->properties()->highlighted(i)->drawHighlight(thePainter, view());

    //FIXME document()->exists necessary?
    if (LastSnap && document()->exists(LastSnap)) {
        LastSnap->drawHover(thePainter, view());
        view()->setToolTip(LastSnap->toHtml());
    } else {
        view()->setToolTip("");
    }
#endif
}

void FeatureSnapInteraction::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == Qt::LeftButton)
        snapMousePressEvent(event,LastSnap);
    if (!(M_PREFS->getMouseSingleButton() && LastSnap))
        Interaction::mousePressEvent(event);
}

void FeatureSnapInteraction::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->button() == Qt::RightButton && !Panning && !Dragging)
        emit(requestCustomContextMenu(event->pos()));

    snapMouseReleaseEvent(event,LastSnap);
    if (!(M_PREFS->getMouseSingleButton() && LastSnap))
        Interaction::mouseReleaseEvent(event);
}

void FeatureSnapInteraction::mouseMoveEvent(QMouseEvent* event)
{
#ifndef _MOBILE
//    if (!document()->isDownloadedSafe(theView->fromView(event->pos())))
//        view()->setCursor(warningCursor);
//    else
        view()->setCursor(cursor());
#endif
    snapMouseMoveEvent(event, LastSnap);
    if (!(M_PREFS->getMouseSingleButton() && LastSnap))
        Interaction::mouseMoveEvent(event);
}

void FeatureSnapInteraction::mouseDoubleClickEvent(QMouseEvent* event)
{
//    if (!document()->isDownloadedSafe(theView->fromView(event->pos())))
//        view()->setCursor(warningCursor);
//    else
#ifndef _MOBILE
        view()->setCursor(cursor());
#endif
    snapMouseDoubleClickEvent(event, LastSnap);
    if (!(M_PREFS->getMouseSingleButton() && LastSnap))
        Interaction::mouseDoubleClickEvent(event);
}

void FeatureSnapInteraction::snapMousePressEvent(QMouseEvent * , Feature*)
{
}

void FeatureSnapInteraction::snapMouseReleaseEvent(QMouseEvent * , Feature*)
{
}

void FeatureSnapInteraction::snapMouseMoveEvent(QMouseEvent* , Feature*)
{
}

void FeatureSnapInteraction::snapMouseDoubleClickEvent(QMouseEvent* , Feature*)
{
}

void FeatureSnapInteraction::activateSnap(bool b)
{
    SnapActive = b;
}

void FeatureSnapInteraction::addToNoSnap(Feature* F)
{
    NoSnap.push_back(F);
}

void FeatureSnapInteraction::clearNoSnap()
{
    NoSnap.clear();
}

void FeatureSnapInteraction::clearSnap()
{
    StackSnap.clear();
}

void FeatureSnapInteraction::clearLastSnap()
{
    LastSnap = 0;
}

QList<Feature*> FeatureSnapInteraction::snapList()
{
    return StackSnap;
}

void FeatureSnapInteraction::addSnap(Feature* aSnap)
{
    StackSnap.append(aSnap);
}

void FeatureSnapInteraction::setSnap(QList<Feature*> aSnapList)
{
    StackSnap = aSnapList;
    curStackSnap = 0;
}

void FeatureSnapInteraction::nextSnap()
{
    if (!StackSnap.size())
        return;
    curStackSnap++;
    if (curStackSnap > StackSnap.size() -1)
        curStackSnap = 0;
    view()->properties()->setSelection(StackSnap[curStackSnap]);
    view()->update();
}

void FeatureSnapInteraction::previousSnap()
{
    if (!StackSnap.size())
        return;
    curStackSnap--;
    if (curStackSnap < 0)
        curStackSnap = StackSnap.size() -1;
    view()->properties()->setSelection(StackSnap[curStackSnap]);
    view()->update();
}

void FeatureSnapInteraction::setDontSelectPoints(bool b)
{
    NoSelectPoints = b;
}

void FeatureSnapInteraction::setDontSelectRoads(bool b)
{
    NoSelectRoads = b;
}

void FeatureSnapInteraction::setDontSelectVirtual(bool b)
{
    NoSelectVirtuals = b;
}

#ifndef _MOBILE
QCursor FeatureSnapInteraction::cursor() const
{
    if (M_PREFS->getMouseSingleButton()) {
        if (!Panning) {
            return handCursor;
        } else {
            return grabCursor;
        }

    } else
        return defaultCursor;
}
#endif

