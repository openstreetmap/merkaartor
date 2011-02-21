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

#include "Mousemachine.h"

#include <QApplication>
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

#define EVENT_AUTOSCROLL (QEvent::Type(QEvent::User+1))
#define EVENT_MOVE (QEvent::Type(QEvent::User+2))
#define EVENT_PRESS (QEvent::Type(QEvent::User+3))
#define EVENT_RELEASE (QEvent::Type(QEvent::User+4))
#define EVENT_DBLCLICK (QEvent::Type(QEvent::User+5))

int gSkipEvents;

struct AutoscrollEvent : public QEvent
{
    AutoscrollEvent(bool val)
    : QEvent(EVENT_AUTOSCROLL),
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
         if (e->type() != EVENT_AUTOSCROLL)
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
    : QEvent(EVENT_MOVE),
      value(val) {}

    bool value;
};

struct FilteredMousePressEvent : public QEvent
{
    FilteredMousePressEvent()
    : QEvent(EVENT_PRESS)
    {}
};

struct FilteredMouseReleaseEvent : public QEvent
{
    FilteredMouseReleaseEvent()
    : QEvent(EVENT_RELEASE)
    {}
};

struct FilteredDoubleClickEvent : public QEvent
{
    FilteredDoubleClickEvent()
    : QEvent(EVENT_DBLCLICK)
    {}
};

class FilteredMouseMoveTransition : public QAbstractTransition
{
 public:
     FilteredMouseMoveTransition(bool value)
         : m_value(value) {}

 protected:
     virtual bool eventTest(QEvent *e)
     {
         if (e->type() != EVENT_MOVE)
             return false;
         FilteredMouseMoveEvent *se = static_cast<FilteredMouseMoveEvent*>(e);
         return (m_value == se->value);
     }

     virtual void onTransition(QEvent *) {}

 private:
     bool  m_value;
 };

class FilteredMouseClickTransition : public QAbstractTransition
{
 public:
     FilteredMouseClickTransition(QEvent::Type typ)
         : m_typ(typ)
     {}

 protected:
     virtual bool eventTest(QEvent *e)
     {
         if (e->type() != m_typ)
             return false;
         return (gSkipEvents == 0);
     }

     virtual void onTransition(QEvent *e) {}

private:
    QEvent::Type  m_typ;
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
    speedTimer.setInterval(SPEED_INTERVAL);
    connect(&speedTimer, SIGNAL(timeout()), SLOT(slotCalculateSpeed()));

    if (options & MouseMachine::SignalDoubleTap)
        dblclickTimer.setInterval(DBLCLICK_INTERVAL);
    else
        dblclickTimer.setInterval(0);
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
        oldHzPolicy = scrollArea->horizontalScrollBarPolicy();
        oldVtPolicy = scrollArea->verticalScrollBarPolicy();
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        theTarget = scrollArea->viewport();
    }
    if (!(options.testFlag(MouseMachine::CascadedEventFilter)))
        theTarget->installEventFilter(this);
    gSkipEvents = 0;

    buildMachine();
    machine->start();
}

bool MouseMachine::eventFilter(QObject *object, QEvent *event)
{
    if (!object->isWidgetType())
        return false;
    if (gSkipEvents && gSkipEvents-- > 0)
        return false;

    QEvent::Type type = event->type();
    if (type != QEvent::MouseButtonPress &&
            type != QEvent::MouseButtonRelease &&
            type != QEvent::MouseButtonDblClick &&
            type != QEvent::MouseMove &&
            type != QEvent::ContextMenu)
        return false;

    QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event);
    if (mouseEvent) {
        if (type == QEvent::MouseMove)
        {
            QPoint curPos = mouseEvent->globalPos();
            if (oldFilteredPos.isNull())
                oldFilteredPos = curPos;

            if ((oldFilteredPos - curPos).manhattanLength() > MIN_MOVE) {
                machine->postEvent(new FilteredMouseMoveEvent(false));
            }
        } else {
            oldFilteredPos = QPoint();
            if (type == QEvent::MouseButtonPress && mouseEvent->button() == Qt::LeftButton && mouseEvent->modifiers() == Qt::NoModifier ) {
                machine->postEvent(new FilteredMousePressEvent());
            } else if (type == QEvent::MouseButtonRelease && mouseEvent->button() == Qt::LeftButton && mouseEvent->modifiers() == Qt::NoModifier) {
                machine->postEvent(new FilteredMouseReleaseEvent());
            } else if (type == QEvent::MouseButtonDblClick && mouseEvent->button() == Qt::LeftButton && mouseEvent->modifiers() == Qt::NoModifier) {
                machine->postEvent(new FilteredDoubleClickEvent());
            }
        }
    }

//#ifndef QT_NO_DEBUG_OUTPUT
//    qDebug() << object;
//    switch (event->type()) {
//    case QEvent::MouseButtonPress:
//        qDebug() << "-> MouseButtonPress " << curPos << " : " << QCursor::pos();
//        break;
//    case QEvent::MouseButtonRelease:
//        qDebug() << "-> MouseButtonRelease" << curPos << " : " << QCursor::pos();
//        break;
//    case QEvent::MouseMove:
//        qDebug() << "-> MouseMove" << curPos << " : " << QCursor::pos();
//        break;
//    case QEvent::MouseButtonDblClick:
//        qDebug() << "-> MouseButtonDblClick" << curPos << " : " << QCursor::pos();
//        break;
//    default:
//        break;
//    }
//#endif
    return true;
}

void MouseMachine::buildMachine()
{
    delete machine;
    machine = new QStateMachine(this);

    stIdle = new QState();
    stPressed = new QState();
    stManScroll = new QState();
    stAutoScroll = new QState();

    trIdle2Pressed = new FilteredMouseClickTransition(EVENT_PRESS);
    trIdle2Pressed->setTargetState(stPressed);
    connect (trIdle2Pressed, SIGNAL(triggered()), SLOT(slotStartTap()));

    trDoubleclick = new FilteredMouseClickTransition(EVENT_DBLCLICK);
    trDoubleclick->setTargetState(stIdle);
    connect (trDoubleclick, SIGNAL(triggered()), SLOT(slotDoubleTap()));

    trPressed2Idle = new FilteredMouseClickTransition(EVENT_RELEASE);
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
        trMan2Auto = new FilteredMouseClickTransition(EVENT_RELEASE);
        trMan2Auto->setTargetState(stAutoScroll);
        connect (trMan2Auto, SIGNAL(triggered()), SLOT(slotStartAuto()));

        trAuto2Man = new FilteredMouseClickTransition(EVENT_PRESS);
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
        trMan2Idle = new FilteredMouseClickTransition(EVENT_RELEASE);
        trMan2Idle->setTargetState(stIdle);
//        connect (trMan2Idle, SIGNAL(triggered()), SLOT(slotPotentialDblclick()));

        stManScroll->addTransition(trMan2Idle);
    }

    machine->setInitialState(stIdle);
}

MouseMachine::~MouseMachine()
{
    machine->stop();

    QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>(theParent);
    if (scrollArea) {
        scrollArea->setHorizontalScrollBarPolicy(oldHzPolicy);
        scrollArea->setVerticalScrollBarPolicy(oldVtPolicy);
    }
}

void MouseMachine::slotStartTap()
{
//    qDebug() << "--- slotStartTap "<< curPos << " : " << QCursor::pos();
    curPos = QCursor::pos();

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
//    qDebug() << "--- slotSingleTap " << curPos << " : " << QCursor::pos();
    tapHoldTimer.stop();

    if (m_options & MouseMachine::SignalTap) {
        emit singleTap(theParent->mapFromGlobal(firstPress));
    } else {
            QWidget*w = theTarget;
            QMouseEvent *event1 = new QMouseEvent(QEvent::MouseButtonPress,
                                                  theParent->mapFromGlobal(firstPress), Qt::LeftButton,
                                                  Qt::LeftButton, Qt::NoModifier);
            QMouseEvent *event2 = new QMouseEvent(QEvent::MouseButtonRelease,
                                                  theParent->mapFromGlobal(firstPress), Qt::LeftButton,
                                                  Qt::LeftButton, Qt::NoModifier);

            gSkipEvents = 2;
            QApplication::postEvent(w, event1);
            QApplication::postEvent(w, event2);
        }
}

void MouseMachine::slotDoubleTap()
{
//    qDebug() << "--- slotDoubleTap " << curPos << " : " << QCursor::pos();
    tapHoldTimer.stop();

    dblclickTimer.stop();

    emit doubleTap(theParent->mapFromGlobal(firstPress));
}

void MouseMachine::slotTapAndHold()
{
//    qDebug() << "--- slotTapAndHold " << curPos << " : " << QCursor::pos();
    tapHoldTimer.stop();

    emit tapAndHold(theParent->mapFromGlobal(firstPress));
}

void MouseMachine::slotStartDrag()
{
//    qDebug() << "--- slotStartDrag " << curPos << " : " << QCursor::pos();
    curPos = QCursor::pos();

    oldPos = curPos;
    oldSpeedPos = curPos;
    speed = QPoint();
    scrollTimeline.stop();
    speedTimer.start();
}

void MouseMachine::slotDrag()
{
//    qDebug() << "--- slotDrag " << curPos << " : " << QCursor::pos();
    curPos = QCursor::pos();

    QPoint delta = curPos - oldPos;

    if (!delta.isNull()) {
        if (m_options.testFlag(MouseMachine::SignalScroll)) {
            emit scroll(theParent->mapFromGlobal(oldPos), theParent->mapFromGlobal(curPos));
        } else {
            if (QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>(theParent)) {
                scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->value() - delta.x());
                scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() - delta.y());
            }
        }
    }
    oldPos = curPos;
}

void MouseMachine::slotCalculateSpeed()
{
    curPos = QCursor::pos();

    speed = curPos - oldSpeedPos;
    oldSpeedPos = curPos;
}

void MouseMachine::slotAutoscroll(qreal val)
{
//    qDebug() << "--- slotAutoscroll " << curPos << " : " << QCursor::pos();
    curPos = QCursor::pos();

    QPoint curAutoPos = speedVector.pointAt(val).toPoint();
    QPoint delta = curAutoPos - oldAutoPos;
//    qDebug() << delta << " : " << curAutoPos << " : " << oldAutoPos << " : " << val;

    if (!delta.isNull()) {
        if (m_options & MouseMachine::SignalScroll) {
            emit scroll(theParent->mapFromGlobal(oldAutoPos), theParent->mapFromGlobal(curAutoPos));
        } else {
            if (QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea*>(theParent)) {
                scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->value() - delta.x());
                scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() - delta.y());
            }
        }
    }
    oldAutoPos = curAutoPos;
}

void MouseMachine::slotStartAuto()
{
//    qDebug() << "--- slotStartAuto " << curPos << " : " << QCursor::pos();
    curPos = QCursor::pos();

    speedTimer.stop();
    if (speed.manhattanLength() > MIN_SPEED) {
        speedVector = QLineF(curPos, curPos + (speed * SPEEDVECTOR_MULTIPLIER));
        oldAutoPos = curPos;
        scrollTimeline.start();
    } else {
//        qDebug() << "--- StopAuto";
        machine->postEvent(new AutoscrollEvent(false));
    }

    slotDrag();
}

void MouseMachine::slotAutoscrollFinished()
{
//    qDebug() << "--- slotAutoscrollFinished " << curPos << " : " << QCursor::pos();

    machine->postEvent(new AutoscrollEvent(false));
}

void MouseMachine::setOptions(Options options)
{
    m_options = options;

    machine->stop();

    buildMachine();
    machine->start();
}
