#include "ControlStrip.h"
#include "ControlButton.h"
#include "MouseMachine/MouseMachine.h"

#include <QtCore>
#include <QtGui>
#include <QObject>

#define STRIP_WIDTH 60
#define BAR_ALPHA 48

ControlStrip::ControlStrip(QWidget *parent)
    : QWidget(parent)
    , BarLayout(0)
    , collapsed(false)
{
    m_Charm = new MouseMachine(this, MouseMachine::SignalTap | MouseMachine::SignalScroll | MouseMachine::CascadedEventFilter);
    m_Charm->setObjectName("ControlStrip_charm");
    connect(m_Charm, SIGNAL(singleTap(QPoint)), SLOT(onSingleTap(QPoint)));
    connect(m_Charm, SIGNAL(tapAndHold(QPoint)), SLOT(onTapAndHold(QPoint)));
    connect(m_Charm, SIGNAL(scroll(QPoint,QPoint)), SLOT(onScroll(QPoint,QPoint)));
    connect(m_Charm, SIGNAL(doubleTap(QPoint)), SLOT(onDoubleTap(QPoint)));

    theBar = new QWidget(this);

//    bookmarkAct = new QAction(QPixmap(":/icons/bookmark"), "Menu", theBar);
//    connect(bookmarkAct, SIGNAL(triggered()), this, SIGNAL(bookmarkClicked()));
//    backAct = new QAction(QPixmap(":/icons/back"), "Back", this);
//    connect(backAct, SIGNAL(triggered()), this, SIGNAL(backClicked()));
//    forwardAct = new QAction(QPixmap(":/icons/forward"), "Forward", theBar);
//    connect(forwardAct, SIGNAL(triggered()), this, SIGNAL(forwardClicked()));
//    stopAct = new QAction(QPixmap(":/icons/stop"), "Stop", theBar);
//    connect(stopAct, SIGNAL(triggered()), this, SIGNAL(stopClicked()));
//    zoomBestAct = new QAction(QPixmap(":/icons/zoomBest"), "Zoom Best", theBar);
//    connect(zoomBestAct, SIGNAL(triggered()), SIGNAL(zoomBestClicked()));
//    refreshAct = new QAction(QPixmap(":/icons/refresh"), "Refresh", theBar);
//    connect(refreshAct, SIGNAL(triggered()), SIGNAL(refreshClicked()));
//    editAct = new QAction(QPixmap(":/icons/edit"), "Edit", theBar);
//    connect(editAct, SIGNAL(triggered()), SIGNAL(editClicked()));
    prefAct = new QAction(QPixmap(":/icons/settings"), "Preferences", theBar);
    connect(prefAct, SIGNAL(triggered()), this, SIGNAL(prefClicked()));
    searchAct = new QAction(QPixmap(":/icons/search"), "Search", theBar);
    connect(searchAct, SIGNAL(triggered()), SIGNAL(searchClicked()));
    zoomInAct = new QAction(QPixmap(":/icons/zoomIn"), "Zoom In", theBar);
    connect(zoomInAct, SIGNAL(triggered()), SIGNAL(zoomInClicked()));
    zoomOutAct = new QAction(QPixmap(":/icons/zoomOut"), "Zoom Out", theBar);
    connect(zoomOutAct, SIGNAL(triggered()), SIGNAL(zoomOutClicked()));
    menuAct = new QAction(QPixmap(":/icons/menu"), "Zoom Out", theBar);
    connect(menuAct, SIGNAL(triggered()), SIGNAL(menuClicked()));

    QSize btSize(48, 48);
//    backBt = new ControlButton(backAct, btSize, theBar);
//    forwardBt = new ControlButton(forwardAct, btSize, theBar);
//    bookmarkBt = new ControlButton(bookmarkAct, btSize, theBar);
//    stopBt = new ControlButton(stopAct, btSize, theBar);
//    searchBt = new ControlButton(searchAct, btSize, theBar);
    prefBt = new ControlButton(prefAct, btSize, theBar);
//    refreshBt = new ControlButton(refreshAct, btSize, theBar);
//    editBt = new ControlButton(editAct, btSize, theBar);
    searchBt = new ControlButton(searchAct, btSize, theBar);
    zoomInBt = new ControlButton(zoomInAct, btSize, theBar);
    zoomOutBt = new ControlButton(zoomOutAct, btSize, theBar);
    menuBt = new ControlButton(menuAct, btSize, theBar);

//    m_menuAct = new QAction(QPixmap(":/icons/back"), "Back", this);
//    connect(m_menuAct, SIGNAL(triggered()), this, SIGNAL(backClicked()));
//    m_menuButton = new ControlButton(backAct, btSize, this);
//    m_menuButton->hide();

//    QVBoxLayout* layout = new QVBoxLayout(this);
//    layout->addWidget(theBar);
//    layout->addWidget(m_menuButton);

//    pal.setBrush(QPalette::Window, QColor(32, 32, 32, BAR_ALPHA));
//    theBar->setPalette(pal);
//    theBar->setBackgroundRole(QPalette::Window);
//    theBar->setAutoFillBackground(true);

//    QSpacerItem* verticalSpacer = new QSpacerItem(20, STRIP_WIDTH, QSizePolicy::Minimum, QSizePolicy::Expanding);
//    theLayout->addItem(verticalSpacer);

    m_dock = DockLeft;

    m_timeLine = new QTimeLine(800, this);
    m_timeLine->setCurveShape(QTimeLine::EaseInOutCurve);
    m_timeLine->setUpdateInterval(100);
    connect(m_timeLine, SIGNAL(valueChanged(qreal)), SLOT(onBarAnimation(qreal)));
    connect(m_timeLine, SIGNAL(finished()), SLOT(onBarAnimationFinished()));

    installEventFilter(m_Charm);
}

void ControlStrip::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

//    p.setPen(QPen(Qt::green, 2));
//    p.drawRect(opt.rect);
}

void ControlStrip::slotLoadingStarted()
{
    setUpdatesEnabled(false);
    refreshBt->hide();
//    theLayout->removeWidget(refreshBt);
    stopBt->show();
//    theLayout->insertWidget(2, stopBt);
    setUpdatesEnabled(true);
}

void ControlStrip::slotLoadingFinished()
{
    setUpdatesEnabled(false);
    stopBt->hide();
//    theLayout->removeWidget(stopBt);
    refreshBt->show();
//    theLayout->insertWidget(2, refreshBt);
    setUpdatesEnabled(true);
}

ControlButton* ControlStrip::getButtonAt(const QPoint& p)
{
    QObjectList ol = children();
    foreach(QObject* o, ol) {
        if (ControlButton* b = dynamic_cast<ControlButton*>(o)) {
            if (b->geometry().contains(p) && b->isVisible()) {
                return b;
            }
        }
    }
    ol = theBar->children();
    foreach(QObject* o, ol) {
        if (ControlButton* b = dynamic_cast<ControlButton*>(o)) {
            if (b->geometry().contains(theBar->mapFrom(this, p)) && b->isVisible()) {
                return b;
            }
        }
    }
    return NULL;
}

void ControlStrip::resizeBar(const QSize& sz, bool resetPos)
{
    switch (m_dock) {
    case DockLeft:
    case DockRight:
        if (BarLayout)
            delete BarLayout;
        BarLayout = new QVBoxLayout(theBar);

        if (sz.height() > sz.width()) {
            theBar->resize(STRIP_WIDTH, sz.width());
        } else {
            theBar->resize(STRIP_WIDTH, sz.height());
        }
        break;

    case DockTop:
    case DockBottom:
        if (BarLayout)
            delete BarLayout;
        BarLayout = new QHBoxLayout(theBar);
        if (sz.height() > sz.width()) {
            theBar->resize(sz.width(), STRIP_WIDTH);
        } else {
            theBar->resize(sz.height(), STRIP_WIDTH);
        }
        break;

    default:
        break;
    }
    if (resetPos) {
        switch (m_dock) {
        case DockLeft:
            qDebug()  << sz.height() << theBar->height();
            move(0, (sz.height() - theBar->height())/2);
            break;
        case DockRight:
            move(sz.width() - STRIP_WIDTH, (sz.height() - height())/2);
            break;

        case DockTop:
            move((sz.width() - width())/2, 0);
            break;
        case DockBottom:
            move((sz.width() - width())/2, sz.height() - STRIP_WIDTH);
            break;

        default:
            break;
        }
    }

//    BarLayout->addWidget(backBt);
//    BarLayout->addWidget(forwardBt);
//    BarLayout->addWidget(stopBt);
//    BarLayout->addWidget(refreshBt);
//    BarLayout->addWidget(searchBt);
//    BarLayout->addWidget(bookmarkBt);
//    BarLayout->addWidget(editBt);
    BarLayout->addWidget(searchBt);
    BarLayout->addWidget(zoomInBt);
    BarLayout->addWidget(zoomOutBt);
    BarLayout->addWidget(menuBt);
    BarLayout->addWidget(prefBt);

    if (!collapsed)
        resize(theBar->size());
}

void ControlStrip::onSingleTap(QPoint p)
{
    ControlButton* b = getButtonAt(p);
    if (b)
        b->trigger();
}

void ControlStrip::Collapse()
{
    if (collapsed)
        return;

//    m_menuButton->resize(backBt->size());
//    resize(STRIP_WIDTH, STRIP_WIDTH);
//    QPoint src = backBt->mapToGlobal(QPoint());
//    QPoint dst = QPoint(STRIP_WIDTH/2, STRIP_WIDTH/2) - QPoint(m_menuButton->width()/2, m_menuButton->height()/2);
//    move(parentWidget()->mapFromGlobal(src-dst));
//    m_menuButton->move(mapFromGlobal(src));
//    m_menuButton->show();
//    theBar->hide();
////        QPoint dst = src - QPoint((width() - m_menuButton->width())/2, (height() - m_menuButton->height())/2);
////        move(parentWidget()->mapFromGlobal(dst));
////        if (m_timeLine->state() == QTimeLine::NotRunning) {
////            originalSize = theBar->size();
////            originalPos = theBar->pos();
////            m_timeLine->setDirection(QTimeLine::Backward);
////            m_timeLine->start();
////        }
    collapsed = true;
}

void ControlStrip::Expand()
{
    if (!collapsed)
        return;

//    QPoint src = m_menuButton->mapToGlobal(QPoint());
//    QPoint dest = backBt->pos();
//    move(parentWidget()->mapFromGlobal(src-dest));
//    m_menuButton->hide();
//    theBar->show();
//    resize(theBar->size());
////        if (m_timeLine->state() == QTimeLine::NotRunning) {
////            m_timeLine->setDirection(QTimeLine::Forward);
////            m_timeLine->start();
////        }
    collapsed = false;
}

void ControlStrip::onDoubleTap(QPoint p)
{
//    ControlButton* b = getButtonAt(p);
//    if (b && b == m_menuButton)
//        bookmarkBt->trigger();
}

void ControlStrip::onTapAndHold(QPoint p)
{
    if (!collapsed) {
        Collapse();
    } else {
        Expand();
    }
}

void ControlStrip::onScroll(QPoint oldP, QPoint newP)
{
    QPoint oldPos = pos();
    QPoint newPos = pos()+newP-oldP;
    QPoint curPos = parentWidget()->mapFromGlobal(QCursor::pos());

    if (curPos.y() > parentWidget()->height() - STRIP_WIDTH && newPos.y() > oldPos.y()) {
        m_dock = DockBottom;
        resizeBar(parentWidget()->size());
        move(newPos.x(), parentWidget()->height() - STRIP_WIDTH);
    } else if (curPos.x() > parentWidget()->width() - STRIP_WIDTH && newPos.x() > oldPos.x()) {
        m_dock = DockRight;
        resizeBar(parentWidget()->size());
        move(parentWidget()->width() - STRIP_WIDTH, newPos.y());
    } else if (curPos.y() < STRIP_WIDTH && newPos.y() < oldPos.y()) {
        m_dock = DockTop;
        resizeBar(parentWidget()->size());
        move(newPos.x(), 0);
    } else if (curPos.x() < STRIP_WIDTH && newPos.x() < oldPos.x()) {
        m_dock = DockLeft;
        resizeBar(parentWidget()->size());
        move(0, newPos.y());
    } else {
        m_dock = DockNone;
        move(newPos);
    }
}

void ControlStrip::onBarAnimation(qreal val)
{
    theBar->resize(originalSize*val);
    if (theBar->isHidden() && val > 0)
        theBar->show();
}

void ControlStrip::onBarAnimationFinished()
{
    if (m_timeLine->currentFrame() == 0)
        theBar->hide();
    if (m_timeLine->currentFrame() > 0)
        m_menuButton->hide();

}

bool ControlStrip::event(QEvent *event)
{
    return QWidget::event(event);
}
