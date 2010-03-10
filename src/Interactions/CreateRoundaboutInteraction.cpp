#include "CreateRoundaboutInteraction.h"
#include "DocumentCommands.h"
#include "WayCommands.h"
#include "NodeCommands.h"
#include "Maps/Painting.h"
#include "Way.h"
#include "Node.h"
#include "Utils/LineF.h"
#include "PropertiesDock.h"
#include "Preferences/MerkaartorPreferences.h"

#include <QtGui/QDockWidget>
#include <QtGui/QPainter>

#include <math.h>

CreateRoundaboutInteraction::CreateRoundaboutInteraction(MainWindow* aMain, MapView* aView)
    : Interaction(aView), Main(aMain), Center(0,0), HaveCenter(false)
{
    theDock = new QDockWidget(Main);
    QWidget* DockContent = new QWidget(theDock);
    DockData.setupUi(DockContent);
    theDock->setWidget(DockContent);
    theDock->setAllowedAreas(Qt::LeftDockWidgetArea);
    Main->addDockWidget(Qt::LeftDockWidgetArea, theDock);
    theDock->show();
    DockData.DriveRight->setChecked(MerkaartorPreferences::instance()->getRightSideDriving());
}

CreateRoundaboutInteraction::~CreateRoundaboutInteraction()
{
    MerkaartorPreferences::instance()->setRightSideDriving(DockData.DriveRight->isChecked());
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
            Center = XY_TO_COORD(event->pos());
        }
        else
        {
            QPointF CenterF(COORD_TO_XY(Center));
            double Radius = distance(CenterF,LastCursor)/view()->pixelPerM();
            double Precision = 2.49;
            if (Radius<2.5)
                Radius = 2.5;
            double Angle = 2*acos(1-Precision/Radius);
            double Steps = ceil(2*M_PI/Angle);
            Angle = 2*M_PI/Steps;
            Radius *= view()->pixelPerM();
            double Modifier = DockData.DriveRight->isChecked()?-1:1;
            QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
            QPen TP(SomeBrush,view()->pixelPerM()*4+1);
            QPointF Prev(CenterF.x()+cos(Modifier*Angle/2)*Radius,CenterF.y()+sin(Modifier*Angle/2)*Radius);
            Node* First = new Node(XY_TO_COORD(Prev));
            Way* R = new Way;
            R->add(First);
            // "oneway" is implied on roundabouts
            //R->setTag("oneway","yes");
            R->setTag("junction","roundabout");
            if (M_PREFS->apiVersionNum() < 0.6)
                R->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
            CommandList* L  = new CommandList(MainWindow::tr("Create Roundabout %1").arg(R->id()), R);
            L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),First,true));
            for (double a = Angle*3/2; a<2*M_PI; a+=Angle)
            {
                QPointF Next(CenterF.x()+cos(Modifier*a)*Radius,CenterF.y()+sin(Modifier*a)*Radius);
                Node* New = new Node(XY_TO_COORD(Next));
                if (M_PREFS->apiVersionNum() < 0.6)
                    New->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
                L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),New,true));
                R->add(New);
            }
            R->add(First);
            L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),R,true));
            for (FeatureIterator it(document()); !it.isEnd(); ++it)
            {
                Way* W1 = CAST_WAY(it.get());
                if (W1 && (W1 != R))
                    Way::createJunction(Main->document(), L, R, W1, true);
            }
            Main->properties()->setSelection(R);
            document()->addHistory(L);
            view()->invalidate(true, false);
            view()->launch(0);
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
        view()->update();
    }
}

void CreateRoundaboutInteraction::paintEvent(QPaintEvent* , QPainter& thePainter)
{
    if (HaveCenter)
    {
        QPointF CenterF(COORD_TO_XY(Center));
        double Radius = distance(CenterF,LastCursor)/view()->pixelPerM();
        double Precision = 1.99;
        if (Radius<2)
            Radius = 2;
        double Angle = 2*acos(1-Precision/Radius);
        double Steps = ceil(2*M_PI/Angle);
        Angle = 2*M_PI/Steps;
        Radius *= view()->pixelPerM();
        double Modifier = DockData.DriveRight->isChecked()?-1:1;
        QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
        QPen TP(SomeBrush,view()->pixelPerM()*4);
        QPointF Prev(CenterF.x()+cos(Modifier*Angle/2)*Radius,CenterF.y()+sin(Modifier*Angle/2)*Radius);
        for (double a = Angle*3/2; a<2*M_PI; a+=Angle)
        {
            QPointF Next(CenterF.x()+cos(Modifier*a)*Radius,CenterF.y()+sin(Modifier*a)*Radius);
            ::draw(thePainter,TP,Feature::OneWay, Prev,Next,4,view()->projection());
            Prev = Next;
        }
        QPointF Next(CenterF.x()+cos(Modifier*Angle/2)*Radius,CenterF.y()+sin(Modifier*Angle/2)*Radius);
        ::draw(thePainter,TP,Feature::OneWay, Prev,Next,4,view()->projection());
    }
}

#ifndef Q_OS_SYMBIAN
QCursor CreateRoundaboutInteraction::cursor() const
{
    return QCursor(Qt::CrossCursor);
}
#endif
