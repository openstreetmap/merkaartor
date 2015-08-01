#include "CreateRoundaboutInteraction.h"

#include "MainWindow.h"
#include "DocumentCommands.h"
#include "WayCommands.h"
#include "NodeCommands.h"
#include "Painting.h"
#include "Way.h"
#include "Node.h"
#include "LineF.h"
#include "PropertiesDock.h"
#include "MerkaartorPreferences.h"
#include "Global.h"

#include <QDockWidget>
#include <QPainter>

#include <math.h>

CreateRoundaboutInteraction::CreateRoundaboutInteraction(MainWindow* aMain)
    : Interaction(aMain), Center(0,0), HaveCenter(false), theDock(0)
{
#ifndef _MOBILE
    theDock = new QDockWidget(theMain);
    QWidget* DockContent = new QWidget(theDock);
    DockData.setupUi(DockContent);
    theDock->setWidget(DockContent);
    theDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    theMain->addDockWidget(Qt::LeftDockWidgetArea, theDock);
    theDock->show();
    connect(DockData.type, SIGNAL(currentIndexChanged(int)), this, SLOT(on_typeChanged(int)));
    DockData.drivingSide->setCurrentIndex(M_PREFS->getrightsidedriving());
    DockData.precision->setValue(M_PREFS->getRoundaboutPrecision());
    DockData.type->setCurrentIndex(M_PREFS->getRoundaboutType());

    theMain->view()->setCursor(cursor());
#endif
}

void CreateRoundaboutInteraction::on_typeChanged(int newType) {
    DockData.drivingSide->setEnabled(newType == 0);
}

CreateRoundaboutInteraction::~CreateRoundaboutInteraction()
{
    M_PREFS->setrightsidedriving(DockData.drivingSide->currentIndex()); /* 0 is left, 1 is right */
    M_PREFS->setRoundaboutType(DockData.type->currentIndex());
    M_PREFS->setRoundaboutPrecision(DockData.precision->value());
    delete theDock;
    view()->update();
}

QString CreateRoundaboutInteraction::toHtml()
{
    QString help;
    //help = (MainWindow::tr("LEFT-CLICK to select; LEFT-DRAG to move"));

    QString desc;
    desc = QString("<big><b>%1</b></big><br/>").arg(MainWindow::tr("Create roundabout Interaction"));
    desc += QString("<b>%1</b><br/>").arg(help);

    QString S =
    "<html><head/><body>"
    "<small><i>" + QString(metaObject()->className()) + "</i></small><br/>"
    + desc;
    S += "</body></html>";

    return S;
}

void CreateRoundaboutInteraction::mousePressEvent(QMouseEvent * event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if (!HaveCenter)
        {
            HaveCenter = true;
            view()->setInteracting(true);
            Center = XY_TO_COORD(event->pos());
        }
        else
        {
            calculatePoints();
            if (Points.size() == 0) return;

            QPointF Prev = Points[0];
            Node* First = g_backend.allocNode(theMain->document()->getDirtyOrOriginLayer(), XY_TO_COORD(Prev.toPoint()));
            Way* R = g_backend.allocWay(theMain->document()->getDirtyOrOriginLayer());
            CommandList* L  = new CommandList(MainWindow::tr("Create Roundabout %1").arg(R->id().numId), R);
            L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),R,true));
            R->add(First);
            L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),First,true));
            if (M_PREFS->getAutoSourceTag()) {
                QStringList sl = theMain->document()->getCurrentSourceTags();
                if (sl.size())
                    R->setTag("source", sl.join(";"));
            }
            // "oneway" is implied on roundabouts
            //R->setTag("oneway","yes");
            if (DockData.type->currentIndex() == 0)
                R->setTag("junction","roundabout");
            for (int i = 1; i < Points.size(); i++ ) {
                QPointF Next = Points[i];
                Node* New = g_backend.allocNode(theMain->document()->getDirtyOrOriginLayer(), XY_TO_COORD(Next.toPoint()));
                L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),New,true));
                R->add(New);
            }
            R->add(First);
            for (FeatureIterator it(document()); !it.isEnd(); ++it) {
                Way* W1 = CAST_WAY(it.get());
                if (W1 && (W1 != R))
                    Way::createJunction(theMain->document(), L, R, W1, true);
            }
            theMain->properties()->setSelection(R);
            document()->addHistory(L);
            view()->setInteracting(false);
            view()->invalidate(true, true, false);
            theMain->launchInteraction(0);
        }
    }
    else
        Interaction::mousePressEvent(event);
}

void CreateRoundaboutInteraction::mouseMoveEvent(QMouseEvent* event)
{
    LastCursor = event->pos();
    if (HaveCenter)
        view()->update();
    Interaction::mouseMoveEvent(event);
}

void CreateRoundaboutInteraction::mouseReleaseEvent(QMouseEvent* anEvent)
{
    if (M_PREFS->getMouseSingleButton() && anEvent->button() == Qt::RightButton) {
        HaveCenter = false;
        view()->setInteracting(false);
        view()->update();
    }
    Interaction::mouseReleaseEvent(anEvent);
}

void CreateRoundaboutInteraction::calculatePoints() {
    Points.clear();
    if (HaveCenter) {
        QPointF CenterF(COORD_TO_XY(Center));
        qreal Radius = distance(CenterF,LastCursor)/view()->pixelPerM();
        qreal Precision = DockData.precision->value(); //2.49;
        /* Let the precision be the approximate number of meters per arc */

        qreal Circumference = 2*M_PI*Radius;
        qreal Steps = Circumference/Precision;
        if (Steps < 3) Steps = 3;
        qreal Angle = 2*M_PI/Steps;

        /* Normalize the radius for drawing */
        Radius *= view()->pixelPerM();

        qreal Modifier = DockData.drivingSide->currentIndex() == 1 ? -1:1;
        for (qreal a = 0; a<2*M_PI; a+=Angle) {
            QPointF point(CenterF.x()+cos(Modifier*a)*Radius,CenterF.y()+sin(Modifier*a)*Radius);
            Points.append(point);
        }
    }
}

void CreateRoundaboutInteraction::paintEvent(QPaintEvent* , QPainter& thePainter)
{
    calculatePoints();

    if (Points.size() == 0) return;

    qreal Precision = DockData.precision->value(); //2.49;
    QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
    QPen TP(SomeBrush,view()->pixelPerM()*Precision/2);

    for (int i = 0; i < Points.size(); i++) {
        QPointF Prev = Points[i];
        QPointF Next = Points[ (i+1)%Points.size() ];
        ::draw(thePainter, TP, (DockData.type->currentIndex() == 0) ? Feature::OneWay : Feature::BothWays, Prev, Next, Precision, view()->projection());
        Prev = Next;
    }
}

#ifndef _MOBILE
QCursor CreateRoundaboutInteraction::cursor() const
{
    return QCursor(Qt::CrossCursor);
}
#endif
