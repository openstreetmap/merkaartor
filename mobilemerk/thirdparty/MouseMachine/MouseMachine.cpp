//
// C++ Implementation: MouseMachine
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "MouseMachine.h"

#include <QWidget>
#include <QMouseEvent>
#include <QDebug>
#include <QAbstractScrollArea>

#define SPEED_INTERVAL 20               // The interval in ms between 2 speed checks. The greater, the greater the speed will be
#define SPEEDVECTOR_MULTIPLIER 4        // the speed multiplier. Multiply the speed to get the end autoscroll pos.
#define MIN_SPEED 12                     // The minimum speed to switch to autoscroll
#define MIN_MOVE 12                     // The minimum move for a move event to be registered as such. Usefull on touch devices where it is nearly certain to get a mouse move between a press and a release.
#define DBLCLICK_INTERVAL 500
#define TAPHOLD_TIMEOUT 500

struct AutoscrollEvent : public QEvent
{
    AutoscrollEvent(bool val)
    : QEvent(QEvent::Type(QEvent::User+1)),
      value(val) {}

    bool value;
};

class AutoscrollTransition : public QAbstractTransition
{
 public:
     AutoscrollTransition(bool value)
         : m_value(value) {}

 protected:
     virtual bool eventTest(QEvent *e)
     {
         if (e->type() != QEvent::Type(QEvent::User+1))
             return false;
         AutoscrollEvent *se = static_cast<AutoscrollEvent*>(e);
         return (m_value == se->value);
     }

     virtual void onTransition(QEvent *) {}

 private:
     bool  m_value;
 };

struct FilteredMouseMoveEvent : public QEvent
{
    FilteredMouseMoveEvent(bool val)
    : QEvent(QEvent::Type(QEvent::User+2)),
      value(val) {}

    bool value;
};

class FilteredMouseMoveTransition : public QAbstractTransition
{
 public:
     FilteredMouseMoveTransition(bool value)
         : m_value(value) {}

 protected:
     virtual bool eventTest(QEvent *e)
     {
         if (e->type() != QEvent::Type(QEvent::User+2))
             return false;
         FilteredMouseMoveEvent *se = static_cast<FilteredMouseMoveEvent*>(e);
         return (m_value == se->value);
     }

     virtual void onTransition(QEvent *) {}

 private:
     bool  m_value;
 };

/**********************/

MouseMachine::MouseMachine(QWidget* parent, MouseMachine::Options options)
 : QObject(parent)
    , theParent(parent)
    , m_options(options)
    , machine(0)
    , trPressed2Idle(0)
    , trPressed2Man(0)
    , trManScrolling(0)
    , trIdle2Pressed(0)
    , trMan2Auto(0)
    , trMan2Idle(0)
    , trAuto2Idle(0)
    , trAuto2Man(0)
    , trDoubleclick(0)
{
#ifdef Q_OS_SYMBIAN
    iTouchFeedback = MTouchFeedback::Instance();
#endif

    speedTimer.setInterval(SPEED_INTERVAL);
    connect(&speedTimer, SIGNAL(timeout()), SLOT(slotCalculateSpeed()));

    dblclickTimer.setInterval(DBLCLICK_INTERVAL);
    dblclickTimer.setSingleShot(true);
    connect(&dblclickTimer, SIGNAL(timeout()), SLOT(slotSingleTap()));

    scrollTimeline.setCurveShape(QTimeLine::EaseOutCurve);
    scrollTimeline.setEasingCurve(QEasingCurve::OutQuad);
    connect(&scrollTimeline, SIGNAL(valueChanged(qreal)), SLOT(slotAutoscroll(qreal)));
    connect(&scrollTimeline, SIGNAL(finished()), SLOT(slotAutoscrollFinished()));

    tapHoldTimer.setInterval(TAPHOLD_TIMEOUT);
    tapHoldTimer.setSingleShot(true);

    theTarget = theParent;
    QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>(theParent);
    if (scrollArea) {
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        theTarget = scrollArea->viewport();
        theTarget->installEventFilter(this);
    }
    theParent->installEventFilter(this);

    buildMachine();
    machine->start();
}

void MouseMachine::buildMachine()
{
    delete machine;
    machine = NULL;

    machine = new QStateMachine(this);

    stIdle = new QState();
    stPressed = new QState();
    stManScroll = new QState();
    stAutoScroll = new QState();

    trIdle2Pressed = new QMouseEventTransition(theTarget, QEvent::MouseButtonPress, Qt::LeftButton);
    trIdle2Pressed->setTargetState(stPressed);
    connect (trIdle2Pressed, SIGNAL(triggered()), SLOT(slotStartTap()));

    trDoubleclick = new QMouseEventTransition(theTarget, QEvent::MouseButtonDblClick, Qt::LeftButton);
    trDoubleclick->setTargetState(stIdle);
    connect (trDoubleclick, SIGNAL(triggered()), SLOT(slotDoubleTap()));

    trPressed2Idle = new QMouseEventTransition(theTarget, QEvent::MouseButtonRelease, Qt::LeftButton);
    trPressed2Idle->setTargetState(stIdle);
    connect (trPressed2Idle, SIGNAL(triggered()), SLOT(slotPotentialDblclick()));

    trPressed2Man = new FilteredMouseMoveTransition(false);
    trPressed2Man->setTargetState(stManScroll);
    connect (trPressed2Man, SIGNAL(triggered()), SLOT(slotStartDrag()));

    trTapHold = new QSignalTransition(&tapHoldTimer, SIGNAL(timeout()));
    trTapHold->setTargetState(stIdle);
    connect (trTapHold, SIGNAL(triggered()), SLOT(slotTapAndHold()));

    trManScrolling = new QMouseEventTransition(theTarget, QEvent::MouseMove, Qt::NoButton);
    connect (trManScrolling, SIGNAL(triggered()), SLOT(slotDrag()));

    stIdle->addTransition(trIdle2Pressed);
    stIdle->addTransition(trDoubleclick);
    stPressed->addTransition(trPressed2Idle);
    stPressed->addTransition(trPressed2Man);
    stPressed->addTransition(trTapHold);
    stManScroll->addTransition(trManScrolling);

    machine->addState(stIdle);
    machine->addState(stPressed);
    machine->addState(stManScroll);

    if (m_options & MouseMachine::AutoScroll) {
        trMan2Auto = new QMouseEventTransition(theTarget, QEvent::MouseButtonRelease, Qt::LeftButton);
        trMan2Auto->setTargetState(stAutoScroll);
        connect (trMan2Auto, SIGNAL(triggered()), SLOT(slotStartAuto()));

        trAuto2Man = new QMouseEventTransition(theTarget, QEvent::MouseButtonPress, Qt::LeftButton);
        trAuto2Man->setTargetState(stManScroll);
        connect (trAuto2Man, SIGNAL(triggered()), SLOT(slotStartDrag()));

        trAuto2Idle = new AutoscrollTransition(false);
        trAuto2Idle->setTargetState(stIdle);
//        connect (trAuto2Idle, SIGNAL(triggered()), SLOT(slotPotentialDblclick()));

        stManScroll->addTransition(trMan2Auto);
        stAutoScroll->addTransition(trAuto2Idle);
        stAutoScroll->addTransition(trAuto2Man);

        machine->addState(stAutoScroll);
    } else {
        trMan2Idle = new QMouseEventTransition(theTarget, QEvent::MouseButtonRelease, Qt::LeftButton);
        trMan2Idle->setTargetState(stIdle);
//        connect (trMan2Idle, SIGNAL(triggered()), SLOT(slotPotentialDblclick()));

        stManScroll->addTransition(trMan2Idle);
    }

    machine->setInitialState(stIdle);
}

MouseMachine::~MouseMachine()
{
    theTarget->removeEventFilter(this);
    theParent->removeEventFilter(this);

    machine->stop();
}

bool MouseMachine::eventFilter(QObject *object, QEvent *event)
{
    if (!object->isWidgetType())
        return false;

    QEvent::Type type = event->type();
    if (type != QEvent::MouseButtonPress &&
            type != QEvent::MouseButtonRelease &&
            type != QEvent::MouseButtonDblClick &&
            type != QEvent::MouseMove &&
            type != QEvent::ContextMenu)
        return false;

    if (type == QEvent::MouseMove)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint curPos = mouseEvent->pos();
        if (oldFilteredPos.isNull())
            oldFilteredPos = curPos;

        if ((oldFilteredPos - curPos).manhattanLength() > MIN_MOVE) {
            machine->postEvent(new FilteredMouseMoveEvent(false));
        }
    } else
        oldFilteredPos = QPoint();

//#ifndef QT_NO_DEBUG_OUTPUT
//	qDebug() << object;
//	switch (event->type()) {
//	case QEvent::MouseButtonPress:
//		qDebug() << "-> MouseButtonPress " << curPos << " : " << theParent->mapFromGlobal(QCursor::pos());
//		break;
//	case QEvent::MouseButtonRelease:
//		qDebug() << "-> MouseButtonRelease" << curPos << " : " << theParent->mapFromGlobal(QCursor::pos());
//		break;
//	case QEvent::MouseMove:
//		qDebug() << "-> MouseMove" << curPos << " : " << theParent->mapFromGlobal(QCursor::pos());
//		break;
//	case QEvent::MouseButtonDblClick:
//		qDebug() << "-> MouseButtonDblClick" << curPos << " : " << theParent->mapFromGlobal(QCursor::pos());
//		break;
//	default:
//		break;
//	}
//#endif
    return true;
}

void MouseMachine::slotStartTap()
{
    qDebug() << "--- slotStartTap "<< curPos << " : " << theParent->mapFromGlobal(QCursor::pos());
    curPos = theParent->mapFromGlobal(QCursor::pos());

    firstPress = curPos;
    tapHoldTimer.start();
}

void MouseMachine::slotPotentialDblclick()
{
    tapHoldTimer.stop();

    if (dblclickTimer.isActive()) {
        dblclickTimer.stop();
        slotDoubleTap();
    } else {
        dblclickTimer.start();
    }
}

void MouseMachine::slotSingleTap()
{
    qDebug() << "--- slotSingleTap " << curPos << " : " << theParent->mapFromGlobal(QCursor::pos());
    tapHoldTimer.stop();

#ifdef Q_OS_SYMBIAN
        iTouchFeedback->InstantFeedback(ETouchFeedbackBasic);
#endif
    emit singleTap(firstPress);
}

void MouseMachine::slotDoubleTap()
{
    qDebug() << "--- slotDoubleTap " << curPos << " : " << theParent->mapFromGlobal(QCursor::pos());
    tapHoldTimer.stop();

    dblclickTimer.stop();
#ifdef Q_OS_SYMBIAN
        iTouchFeedback->InstantFeedback(ETouchFeedbackBasic);
#endif
    emit doubleTap(firstPress);
}

void MouseMachine::slotTapAndHold()
{
    qDebug() << "--- slotTapAndHold " << curPos << " : " << theParent->mapFromGlobal(QCursor::pos());
    tapHoldTimer.stop();

    emit tapAndHold(firstPress);
}

void MouseMachine::slotStartDrag()
{
    qDebug() << "--- slotStartDrag " << curPos << " : " << theParent->mapFromGlobal(QCursor::pos());
    curPos = theParent->mapFromGlobal(QCursor::pos());

    oldPos = curPos;
    oldSpeedPos = curPos;
    speed = QPoint();
    scrollTimeline.stop();
    speedTimer.start();
}

void MouseMachine::slotDrag()
{
    qDebug() << "--- slotDrag " << curPos << " : " << theParent->mapFromGlobal(QCursor::pos());
    curPos = theParent->mapFromGlobal(QCursor::pos());

    QPoint delta = curPos - oldPos;
    oldPos = curPos;
    if (!delta.isNull())
        emit scroll(delta);
}

void MouseMachine::slotCalculateSpeed()
{
    curPos = theParent->mapFromGlobal(QCursor::pos());

    speed = curPos - oldSpeedPos;
    oldSpeedPos = curPos;
}

void MouseMachine::slotAutoscroll(qreal val)
{
    qDebug() << "--- slotAutoscroll " << curPos << " : " << theParent->mapFromGlobal(QCursor::pos());
    curPos = theParent->mapFromGlobal(QCursor::pos());

    QPoint curAutoPos = speedVector.pointAt(val).toPoint();
    QPoint delta = curAutoPos - oldAutoPos;
    qDebug() << delta << " : " << curAutoPos << " : " << oldAutoPos << " : " << val;
    oldAutoPos = curAutoPos;
    if (!delta.isNull())
        emit scroll(delta);
}

void MouseMachine::slotStartAuto()
{
    qDebug() << "--- slotStartAuto " << curPos << " : " << theParent->mapFromGlobal(QCursor::pos());
    curPos = theParent->mapFromGlobal(QCursor::pos());

    speedTimer.stop();
    if (speed.manhattanLength() > MIN_SPEED) {
        speedVector = QLineF(curPos, curPos + (speed * SPEEDVECTOR_MULTIPLIER));
        oldAutoPos = curPos;
        scrollTimeline.start();
    } else {
        qDebug() << "--- StopAuto";
        machine->postEvent(new AutoscrollEvent(false));
    }

    slotDrag();
}

void MouseMachine::slotAutoscrollFinished()
{
    qDebug() << "--- slotAutoscrollFinished " << curPos << " : " << theParent->mapFromGlobal(QCursor::pos());

    machine->postEvent(new AutoscrollEvent(false));
}

void MouseMachine::setOptions(Options options)
{
    m_options = options;

    machine->stop();

    buildMachine();
    machine->start();
}
