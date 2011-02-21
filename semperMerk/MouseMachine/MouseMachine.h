//
// C++ Interface: MouseMachine
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef MOUSEMACHINE_H
#define MOUSEMACHINE_H

#include <QObject>
#include <QPoint>

#include <QState>
#include <QStateMachine>
#include <QMouseEventTransition>
#include <QSignalTransition>
#include <QTimer>
#include <QTimeLine>
#include <QLineF>
#include <QFlags>
#include <QScrollBar>

#ifdef Q_OS_SYMBIAN
#include <touchfeedback.h>
#endif


class AutoscrollTransition;
class FilteredMouseMoveTransition;
class FilteredMouseClickTransition;

class MouseMachine : public QObject
{
    Q_OBJECT

public:

    enum Option {
        NoOptions = 0x0,
        AutoScroll = 0x1,
        SignalTap = 0x2,
        SignalDoubleTap = 0x4,
        SignalScroll = 0x8,
        CascadedEventFilter = 0x10
    };
    Q_DECLARE_FLAGS(Options, Option)

    MouseMachine(QWidget* parent = 0, MouseMachine::Options options = MouseMachine::NoOptions);
    ~MouseMachine();

    void setOptions(MouseMachine::Options options);
    bool eventFilter(QObject *object, QEvent *event);

protected slots:
    void slotStartTap();
    void slotSingleTap();
    void slotDoubleTap();
    void slotTapAndHold();
    void slotStartDrag();
    void slotDrag();
    void slotStartAuto();
    void slotAutoscrollFinished();

    void slotCalculateSpeed();
    void slotPotentialDblclick();
    void slotAutoscroll(qreal val);

signals:
    void singleTap(QPoint pos);
    void doubleTap(QPoint pos);
    void tapAndHold(QPoint pos);
    void scroll(QPoint oldPos, QPoint newPos);

protected:
    void buildMachine();

protected:
    QWidget* theParent;
    QWidget* theTarget;
    Options m_options;

    QStateMachine* machine;
    QState *stIdle;
    QState *stPressed;
    QState *stManScroll;
    QState *stAutoScroll;

    FilteredMouseClickTransition* trPressed2Idle;
    FilteredMouseMoveTransition* trPressed2Man;
    QSignalTransition* trTapHold;
    QMouseEventTransition* trManScrolling;
    FilteredMouseClickTransition* trIdle2Pressed;
    FilteredMouseClickTransition* trMan2Auto;
    FilteredMouseClickTransition* trMan2Idle;
    AutoscrollTransition* trAuto2Idle;
    FilteredMouseClickTransition* trAuto2Man;
    FilteredMouseClickTransition* trDoubleclick;

    QPoint firstPress;
    QPoint curPos;
    QPoint oldPos;
    QPoint oldFilteredPos;
    QPoint oldAutoPos;
    QPoint oldSpeedPos;
    QPoint speed;
    QLineF speedVector;

    QTimer speedTimer;
    QTimer dblclickTimer;
    QTimer tapHoldTimer;
    QTimeLine scrollTimeline;

    Qt::ScrollBarPolicy oldHzPolicy;
    Qt::ScrollBarPolicy oldVtPolicy;

#ifdef Q_OS_SYMBIAN
    MTouchFeedback* iTouchFeedback; // For Tactile feedback
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MouseMachine::Options)

#endif // MOUSEMACHINE_H
