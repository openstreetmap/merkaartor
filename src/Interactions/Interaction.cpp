#include "Interaction.h"

#include "MapView.h"
#include "Document.h"
#include "Maps/Projection.h"
#include "Node.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include <math.h>

Interaction::Interaction(MapView* aView)
: theView(aView), Panning(false), Dragging(false), StartDrag(0,0), EndDrag(0,0)
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

const Projection& Interaction::projection() const
{
    return theView->projection();
}

const QTransform& Interaction::transform() const
{
    return theView->transform();
}

void Interaction::mousePressEvent(QMouseEvent * anEvent)
{
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

void Interaction::wheelEvent(QWheelEvent* ev)
{
    double finalZoom = 1.;
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

/***************/

FeatureSnapInteraction::FeatureSnapInteraction(MapView* theView)
        : Interaction(theView), SnapActive(true),
      NoSelectPoints(false), NoSelectRoads(false)
          , NoSelectVirtuals(true), LastSnap(0)
{
    handCursor = QCursor(QPixmap(":/Icons/grab.png"));
    grabCursor = QCursor(QPixmap(":/Icons/grabbing.png"));
    defaultCursor = QCursor(Qt::ArrowCursor);
}

void FeatureSnapInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
    Interaction::paintEvent(anEvent, thePainter);

    for (int i=0; i<main()->features()->highlightedSize(); ++i)
        if (document()->exists(main()->features()->highlighted(i)))
            main()->features()->highlighted(i)->drawHighlight(thePainter, view(), true);
    for (int i=0; i<main()->properties()->size(); ++i)
        if (document()->exists(main()->properties()->selection(i)))
            main()->properties()->selection(i)->drawFocus(thePainter, view());
    for (int i=0; i<main()->properties()->highlightedSize(); ++i)
        if (document()->exists(main()->properties()->highlighted(i)))
            main()->properties()->highlighted(i)->drawHighlight(thePainter, view(), true);

#ifndef _MOBILE
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
    updateSnap(event);
    if (event->button() == Qt::LeftButton)
        snapMousePressEvent(event,LastSnap);
    if (!(M_PREFS->getMouseSingleButton() && LastSnap))
        Interaction::mousePressEvent(event);
}

void FeatureSnapInteraction::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->button() == Qt::RightButton && !Panning)
        emit(requestCustomContextMenu(event->pos()));

    updateSnap(event);
    snapMouseReleaseEvent(event,LastSnap);
    if (!(M_PREFS->getMouseSingleButton() && LastSnap))
        Interaction::mouseReleaseEvent(event);
}

void FeatureSnapInteraction::mouseMoveEvent(QMouseEvent* event)
{
    updateSnap(event);
    view()->setCursor(cursor());
    snapMouseMoveEvent(event, LastSnap);
    if (!(M_PREFS->getMouseSingleButton() && LastSnap))
        Interaction::mouseMoveEvent(event);
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
    curStackSnap++;
    if (curStackSnap > StackSnap.size() -1)
        curStackSnap = 0;
    view()->properties()->setSelection(StackSnap[curStackSnap]);
    view()->update();
}

void FeatureSnapInteraction::previousSnap()
{
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

void FeatureSnapInteraction::updateSnap(QMouseEvent* event)
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
    if (!SnapActive) return;
    //QTime Start(QTime::currentTime());
//    CoordBox HotZone(XY_TO_COORD(event->pos()-QPointF(15,15)),XY_TO_COORD(event->pos()+QPointF(15,15)));
    CoordBox HotZone(XY_TO_COORD(event->pos()-QPointF(M_PREFS->getMaxGeoPicWidth()+5,M_PREFS->getMaxGeoPicWidth()+5)),XY_TO_COORD(event->pos()+QPointF(M_PREFS->getMaxGeoPicWidth()+5,M_PREFS->getMaxGeoPicWidth()+5)));
    SnapList.clear();
    double BestDistance = 5;
    bool areNodesVisible = (view()->pixelPerM() >= M_PREFS->getLocalZoom());

    for (int j=0; j<document()->layerSize(); ++j) {
        if (!document()->getLayer(j)->isVisible() || document()->getLayer(j)->isReadonly())
            continue;

        QList < MapFeaturePtr > ret = document()->getLayer(j)->indexFind(HotZone);
        foreach(MapFeaturePtr F, ret) {
            if (F)
            {
                if (F->notEverythingDownloaded())
                    continue;
                if (CAST_WAY(F)) {
                    if ( NoRoads || NoSelectRoads)
                        continue;
                }
                if (Node* N = CAST_NODE(F)) {
                    if (NoSelectPoints)
                        continue;
                    if (!areNodesVisible && !N->hasPhoto())
                        continue;
                }
                if (std::find(NoSnap.begin(),NoSnap.end(),F) != NoSnap.end())
                    continue;

                double Distance = F->pixelDistance(event->pos(), 5.01, areNodesVisible, view());
                SnapList.push_back(F);
                if (Distance < BestDistance)
                {
                    BestDistance = Distance;
                    LastSnap = F;
                }
            }
        }
    }
    if (LastSnap && LastSnap->isVirtual() && NoSelectVirtuals)
        LastSnap = LastSnap->getParent(0);

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

#ifndef Q_OS_SYMBIAN
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

