/***************************************************************************
 *   Copyright (C) 2007 by Kai Winter   *
 *   kaiwinter@gmx.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "mapcontrol.h"

MapControl::MapControl(QSize size, MouseMode mousemode)
	: size(size), mousemode(mousemode)
{
	layermanager = new LayerManager(this, size);
	screen_middle = QPoint(size.width()/2, size.height()/2);
	
	mousepressed = false;
	
	connect(ImageManager::instance(), SIGNAL(imageReceived()),
			  this, SLOT(updateRequestNew()));
	
	connect(ImageManager::instance(), SIGNAL(loadingFinished()),
			  this, SLOT(loadingFinished()));
	
	this->setMaximumSize(size.width()+1, size.height()+1);
}

MapControl::~MapControl()
{
	delete layermanager;	
}

QPointF	MapControl::getCurrentCoordinate() const
{
	return layermanager->getCurrentCoordinate();
}

Layer* MapControl::getLayer(const QString& layername) const
{
	return layermanager->getLayer(layername);
}

QList<QString> MapControl::getLayers() const
{
	return layermanager->getLayers();
}

int MapControl::getNumberOfLayers() const
{
	return layermanager->getLayers().size();
}

void MapControl::followGeometry(const Geometry* geom) const
{
	connect(geom, SIGNAL(positionChanged(Geometry*)),
			  this, SLOT(positionChanged(Geometry*)));
}

void MapControl::positionChanged(Geometry* geom)
{
	QPoint start = layermanager->getLayer()->getMapAdapter()->coordinateToDisplay(getCurrentCoordinate());
	QPoint dest = layermanager->getLayer()->getMapAdapter()->coordinateToDisplay(((Point*)geom)->getCoordinate());

	QPoint step = (dest-start);

	layermanager->scrollView(step);
	
// 	setView(geom);
	update();
}

void MapControl::moveTo(QPointF coordinate)
{
	target = coordinate;
	steps = 25;
	if (moveMutex.tryLock())
	{
		QTimer::singleShot(40, this, SLOT(tick()));	
	}
	else
	{
// 		stopMove(coordinate);
	}
}
void MapControl::tick()
{
	QPoint start = layermanager->getLayer()->getMapAdapter()->coordinateToDisplay(getCurrentCoordinate());
	QPoint dest = layermanager->getLayer()->getMapAdapter()->coordinateToDisplay(target);
	
	QPoint step = (dest-start)/steps;
	QPointF next = getCurrentCoordinate()- step;

// 	setView(Coordinate(next.x(), next.y()));
	layermanager->scrollView(step);
	
	update();
	steps--;
	if (steps>0)
	{
		QTimer::singleShot(40, this, SLOT(tick()));
	}
	else
	{
		moveMutex.unlock();
	}
}

void MapControl::paintEvent(QPaintEvent* evnt)
{
	QWidget::paintEvent(evnt);
	QPainter painter(this);
	
// 	painter.translate(150,190);
// 	painter.scale(0.5,0.5);
	
// 	painter.setClipRect(0,0, size.width(), size.height());
	
// 	painter.setViewport(10000000000,0,size.width(),size.height());
	
	/*
	// rotating
	rotation = 45;
	painter.translate(256,256);
	painter.rotate(rotation);
	painter.translate(-256,-256);
	*/

	layermanager->drawImage(&painter);
	layermanager->drawGeoms(&painter);
	
	painter.drawLine(screen_middle.x(), screen_middle.y()-10,
						  screen_middle.x(), screen_middle.y()+10); // |
	painter.drawLine(screen_middle.x()-10, screen_middle.y(),
						  screen_middle.x()+10, screen_middle.y()); // -
	
// 	int cross_x = int(layermanager->getMapmiddle_px().x())%256;
// 	int cross_y = int(layermanager->getMapmiddle_px().y())%256;
// 	painter.drawLine(screen_middle.x()-cross_x+cross_x, screen_middle.y()-cross_y+0,
// 						  screen_middle.x()-cross_x+cross_x, screen_middle.y()-cross_y+256); // |
// 	painter.drawLine(screen_middle.x()-cross_x+0, screen_middle.y()-cross_y+cross_y,
// 						  screen_middle.x()-cross_x+256, screen_middle.y()-cross_y+cross_y); // -
	
	painter.drawRect(0,0, size.width(), size.height());
	/*
	// rotating
	painter.setMatrix(painter.matrix().inverted());
	//qt = painter.transform();
	qm = painter.combinedMatrix();
	*/

	if (mousepressed && mousemode == Dragging)
	{
		QRect rect = QRect(pre_click_px, current_mouse_pos);
		painter.drawRect(rect);
	}
}

// mouse events
void MapControl::mousePressEvent(QMouseEvent* evnt)
{
	//rotating
	
// 	QMouseEvent* me = new QMouseEvent(evnt->type(), qm.map(QPoint(evnt->x(),evnt->y())), evnt->button(), evnt->buttons(), evnt->modifiers());
// 	evnt = me;
// 	qDebug() << "evnt: " << evnt->x() << ", " << evnt->y() << ", " << evnt->pos();

	
	layermanager->mouseEvent(evnt);
	
	if (layermanager->getLayers().size()>0)
	{
		if (evnt->button() == 1)
		{
			mousepressed = true;
			pre_click_px = QPoint(evnt->x(), evnt->y());
		}
		else if (evnt->button() == 2 && mousemode != None)	// zoom in
		{
			zoomIn();
		} else if  (evnt->button() == 4 && mousemode != None)	// zoom out
		{
			zoomOut();
		}
	}
	
// 	emit(mouseEvent(evnt));
	emit(mouseEventCoordinate(evnt, clickToWorldCoordinate(evnt->pos())));
}

void MapControl::mouseReleaseEvent(QMouseEvent* evnt)
{
	mousepressed = false;
	if (mousemode == Dragging)
	{
		QPointF ulCoord = clickToWorldCoordinate(pre_click_px);
		QPointF lrCoord = clickToWorldCoordinate(current_mouse_pos);
		
		QRectF coordinateBB = QRectF(ulCoord, QSizeF( (lrCoord-ulCoord).x(), (lrCoord-ulCoord).y()));

		emit(draggedRect(coordinateBB));
	}
	
	emit(mouseEventCoordinate(evnt, clickToWorldCoordinate(evnt->pos())));
}

void MapControl::mouseMoveEvent(QMouseEvent* evnt)
{
// 	emit(mouseEvent(evnt));
	
	/*
	// rotating
	QMouseEvent* me = new QMouseEvent(evnt->type(), qm.map(QPoint(evnt->x(),evnt->y())), evnt->button(), evnt->buttons(), evnt->modifiers());
	evnt = me;
	*/
	if (mousepressed && mousemode == Panning)
	{
		QPoint offset = pre_click_px - QPoint(evnt->x(), evnt->y());
		layermanager->scrollView(offset);
		pre_click_px = QPoint(evnt->x(), evnt->y());
	}
	else if (mousepressed && mousemode == Dragging)
	{
		current_mouse_pos = QPoint(evnt->x(), evnt->y());
	}
// 	emit(mouseEventCoordinate(evnt, clickToWorldCoordinate(evnt->pos())));

	update();
// 	emit(mouseEventCoordinate(evnt, clickToWorldCoordinate(evnt->pos())));
}

QPointF MapControl::clickToWorldCoordinate(QPoint click)
{
	// click coordinate to image coordinate
	QPoint displayToImage= QPoint(click.x()-screen_middle.x()+layermanager->getMapmiddle_px().x(),
											click.y()-screen_middle.y()+layermanager->getMapmiddle_px().y());
	// image coordinate to world coordinate
	return layermanager->getLayer()->getMapAdapter()->displayToCoordinate(displayToImage);
}

void MapControl::updateRequest(QRect rect)
{
	update(rect);
}
void MapControl::updateRequestNew()
{
// 	qDebug() << "MapControl::updateRequestNew()";
	layermanager->forceRedraw();
	update();
}
// slots
void MapControl::zoomIn()
{
	layermanager->zoomIn();
	update();
}
void MapControl::zoomOut()
{
	layermanager->zoomOut();
	update();
}
void MapControl::setZoom(int zoomlevel)
{
	layermanager->setZoom(zoomlevel);
	update();
}
void MapControl::scrollLeft(int pixel)
{
	layermanager->scrollView(QPoint(-pixel,0));
	update();
}
void MapControl::scrollRight(int pixel)
{
	layermanager->scrollView(QPoint(pixel,0));
	update();
}
void MapControl::scrollUp(int pixel)
{
	layermanager->scrollView(QPoint(0,-pixel));
	update();
}
void MapControl::scrollDown(int pixel)
{
	layermanager->scrollView(QPoint(0,pixel));
	update();
}
void MapControl::scroll(const QPoint scroll)
{
	layermanager->scrollView(scroll);
	update();
}

void MapControl::setView(const QPointF& coordinate) const
{
	layermanager->setView(coordinate);
}

void MapControl::setView(const QList<QPointF> coordinates) const
{
	layermanager->setView(coordinates);
}

void MapControl::setView(const Point* point) const
{
	layermanager->setView(point->getCoordinate());
}

void MapControl::loadingFinished()
{
// 	qDebug() << "MapControl::loadingFinished()";
	layermanager->removeZoomImage();
}
void MapControl::addLayer(Layer* layer)
{
	layermanager->addLayer(layer);
}

void MapControl::setMouseMode(MouseMode mousemode)
{
	this->mousemode = mousemode;
}
MapControl::MouseMode MapControl::getMouseMode()
{
	return mousemode;
}

void MapControl::stopFollowing(Geometry* geom)
{
	geom->disconnect(SIGNAL(positionChanged(Geometry*)));
}
