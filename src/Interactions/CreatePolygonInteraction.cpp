#include "CreatePolygonInteraction.h"
#include "DocumentCommands.h"
#include "WayCommands.h"
#include "NodeCommands.h"
#include "Maps/Painting.h"
#include "Way.h"
#include "Node.h"
#include "Utils/LineF.h"
#include "PropertiesDock.h"
#include "Preferences/MerkaartorPreferences.h"

#include <QtGui/QPainter>
#include <QInputDialog>

#include <math.h>

CreatePolygonInteraction::CreatePolygonInteraction(MainWindow* aMain, MapView* aView, int sides)
	: Interaction(aView), Main(aMain), Origin(0,0), Sides(sides), HaveOrigin(false), bAngle(0.0), bScale(QPointF(1., 1.))
{
}

CreatePolygonInteraction::~CreatePolygonInteraction()
{
	view()->update();
}

QString CreatePolygonInteraction::toHtml()
{
	QString help;
	help = (MainWindow::tr("LEFT-CLICK to start;DRAG to scale;SHIFT-DRAG to rotate;LEFT-CLICK to end"));

	QStringList helpList = help.split(";");

	QString desc;
	desc = QString("<big><b>%1</b></big>").arg(MainWindow::tr("Create Polygon Interaction"));

	QString S =
	"<html><head/><body>"
	"<small><i>" + QString(metaObject()->className()) + "</i></small><br/>"
	+ desc;
	S += "<hr/>";
	S += "<ul style=\"margin-left: 0px; padding-left: 0px;\">";
	for (int i=0; i<helpList.size(); ++i) {
		S+= "<li>" + helpList[i] + "</li>";
	}
	S += "</ul>";
	S += "</body></html>";

	return S;
}


void CreatePolygonInteraction::mousePressEvent(QMouseEvent * event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		if (!HaveOrigin)
		{
			HaveOrigin = true;
			Origin = XY_TO_COORD(event->pos());
			OriginF = event->pos();
			bAngle = 0.;
			bScale = QPointF(1., 1.);
		}
		else
		{
			QPointF CenterF(0.5, 0.5);
			double Radius = 0.5;
			if (Sides == 4)
				Radius = sqrt(2.)/2.;
			double Angle = 2*M_PI/Sides;
			QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
			QPen TP(SomeBrush,view()->pixelPerM()*4);

			QMatrix m;
			m.translate(OriginF.x(), OriginF.y());
			m.rotate(bAngle);
			m.scale(bScale.x(), bScale.y());

			QPointF Prev(CenterF.x()+cos(-Angle/2)*Radius,CenterF.y()+sin(-Angle/2)*Radius);
			Node* First = new Node(XY_TO_COORD(m.map(Prev)));
			Way* R = new Way;
			R->add(First);
			if (M_PREFS->apiVersionNum() < 0.6)
				R->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
			CommandList* L  = new CommandList(MainWindow::tr("Create Polygon %1").arg(R->id()), R);
			L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),First,true));
			for (double a = 2*M_PI - Angle*3/2; a>0; a-=Angle)
			{
				QPointF Next(CenterF.x()+cos(a)*Radius,CenterF.y()+sin(a)*Radius);
				Node* New = new Node(XY_TO_COORD(m.map(Next)));
				if (M_PREFS->apiVersionNum() < 0.6)
					New->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
				L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),New,true));
				R->add(New);
			}
			R->add(First);
			L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),R,true));
			for (FeatureIterator it(document()); !it.isEnd(); ++it)
			{
				Way* W1 = dynamic_cast<Way*>(it.get());
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

void CreatePolygonInteraction::paintEvent(QPaintEvent* , QPainter& thePainter)
{
	if (HaveOrigin)
	{
		QPointF CenterF(0.5, 0.5);
		double Radius = 0.5;
		if (Sides == 4)
			Radius = sqrt(2.)/2.;

		QMatrix m;
		m.translate(OriginF.x(), OriginF.y());
		m.rotate(bAngle);
		m.scale(bScale.x(), bScale.y());
		QPolygonF thePoly = m.map(QRectF(QPointF(0.0, 0.0), QPointF(1.0, 1.0)));

		thePainter.setPen(QPen(QColor(0,0,255),1,Qt::DotLine));
		thePainter.drawPolygon(thePoly);

		double Angle = 2*M_PI/Sides;
		QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
		QPen TP(SomeBrush,view()->pixelPerM()*4);
		QPointF Prev(CenterF.x()+cos(-Angle/2)*Radius,CenterF.y()+sin(-Angle/2)*Radius);
		for (double a = 2*M_PI - Angle*3/2; a>0; a-=Angle)
		{
			QPointF Next(CenterF.x()+cos(a)*Radius,CenterF.y()+sin(a)*Radius);
			::draw(thePainter,TP,Feature::UnknownDirection, m.map(Prev),m.map(Next),4,view()->projection());
			Prev = Next;
		}
		QPointF Next(CenterF.x()+cos(-Angle/2)*Radius,CenterF.y()+sin(-Angle/2)*Radius);
		::draw(thePainter,TP,Feature::UnknownDirection, m.map(Prev),m.map(Next),4,view()->projection());
	}
}

void CreatePolygonInteraction::mouseMoveEvent(QMouseEvent* event)
{
	if (HaveOrigin) {
		QMatrix m;
		m.translate(OriginF.x(), OriginF.y());
		m.rotate(bAngle);

		if (event->modifiers() & Qt::ShiftModifier) {
			bAngle += radToAng(angle(m.inverted().map(LastCursor), m.inverted().map(event->pos())));

			QMatrix m2;
			m2.translate(OriginF.x(), OriginF.y());
			m2.rotate(bAngle);
			bScale = m2.inverted().map(event->pos());
		} else {
			bScale = m.inverted().map(event->pos());
		}

		view()->update();
	}
	LastCursor = event->pos();
	Interaction::mouseMoveEvent(event);
}

void CreatePolygonInteraction::mouseReleaseEvent(QMouseEvent* event)
{
	if (M_PREFS->getMouseSingleButton() && event->button() == Qt::RightButton) {
		HaveOrigin = false;
		view()->update();
	}
}


#ifndef Q_OS_SYMBIAN
QCursor CreatePolygonInteraction::cursor() const
{
	return QCursor(Qt::CrossCursor);
}
#endif
