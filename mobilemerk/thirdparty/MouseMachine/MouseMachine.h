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

#ifdef Q_OS_SYMBIAN
#include <touchfeedback.h>
#endif


class AutoscrollTransition;
class FilteredMouseMoveTransition;

class MouseMachine : public QObject
{
	Q_OBJECT

public:

	enum Option {
		NoOptions = 0x0,
		AutoScroll = 0x1,
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
    void scroll(QPoint delta);

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

	QMouseEventTransition* trPressed2Idle;
	FilteredMouseMoveTransition* trPressed2Man;
	QSignalTransition* trTapHold;
	QMouseEventTransition* trManScrolling;
	QMouseEventTransition* trIdle2Pressed;
	QMouseEventTransition* trMan2Auto;
    QMouseEventTransition* trMan2Idle;
    AutoscrollTransition* trAuto2Idle;
	QMouseEventTransition* trAuto2Man;
    QMouseEventTransition* trDoubleclick;

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

#ifdef Q_OS_SYMBIAN
	MTouchFeedback* iTouchFeedback; // For Tactile feedback
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS(MouseMachine::Options)

#endif // MOUSEMACHINE_H
