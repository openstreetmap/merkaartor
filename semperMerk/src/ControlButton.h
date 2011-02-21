#ifndef CONTROLBUTTON_H
#define CONTROLBUTTON_H

#include <QToolButton>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QAction>

class ControlButton : public QToolButton
{
    Q_OBJECT

public:
    ControlButton(QAction* theAct, QSize sz, QWidget *parent)
        : QToolButton(parent)
    {
        setDefaultAction(theAct);
        setIconSize(sz);
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }

public slots:
    void trigger()
    {
        defaultAction()->trigger();
    }

};

#endif // CONTROLBUTTON_H
