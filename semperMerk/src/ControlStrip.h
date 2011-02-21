#ifndef CONTROLSTRIP_H
#define CONTROLSTRIP_H

#include <QWidget>
#include <QAction>
#include <QTimeLine>

class QBoxLayout;
class ControlButton;
class MouseMachine;

class ControlStrip : public QWidget
{
    Q_OBJECT

    friend class MainWindow;

public:
    enum Docking {
        DockNone,
        DockTop,
        DockBottom,
        DockLeft,
        DockRight
    };
public:
    ControlStrip(QWidget *parent = 0);

    void resizeBar(const QSize& sz, bool resetPos=false);

signals:
    void bookmarkClicked();
    void backClicked();
    void forwardClicked();
    void stopClicked();
    void prefClicked();
    void searchClicked();
    void zoomInClicked();
    void zoomOutClicked();
    void refreshClicked();
    void editClicked();
    void menuClicked();

public slots:
    void slotLoadingStarted();
    void slotLoadingFinished();

    void Collapse();
    void Expand();

private slots:
    void onSingleTap(QPoint p);
    void onDoubleTap(QPoint p);
    void onTapAndHold(QPoint p);
    void onScroll(QPoint oldPos, QPoint newPos);
    void onBarAnimation(qreal);
    void onBarAnimationFinished();

protected:
    void paintEvent(QPaintEvent *event);
    bool event (QEvent * event);
    ControlButton * getButtonAt(const QPoint &p);

private:
    QPalette pal;
    QWidget* theBar;
    QBoxLayout* BarLayout;

    ControlButton *backBt;
    ControlButton *forwardBt;
    ControlButton *bookmarkBt;
    ControlButton *stopBt;
    ControlButton *zoomInBt;
    ControlButton *zoomOutBt;
    ControlButton *searchBt;
    ControlButton *prefBt;
    ControlButton *refreshBt;
    ControlButton *editBt;
    ControlButton *menuBt;

    ControlButton *m_menuButton;
    MouseMachine* m_Charm;

    QPoint lastMenuPos;
    Docking m_dock;
    QTimeLine *m_timeLine;
    QSize originalSize;
    QPoint originalPos;
    bool collapsed;

public:
    QAction* bookmarkAct;
    QAction* backAct;
    QAction* forwardAct;
    QAction* stopAct;
    QAction* prefAct;
    QAction* zoomInAct;
    QAction* zoomOutAct;
    QAction* searchAct;
    QAction* refreshAct;
    QAction* editAct;
    QAction* menuAct;

    QAction* m_menuAct;
};

#endif // CONTROLSTRIP_H
