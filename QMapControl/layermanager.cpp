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
#include "layermanager.h"

LayerManager::LayerManager(QWidget* aParent, QSize size)
	:scroll(QPoint(0,0)), size(size), whilenewscroll(QPoint(0,0))
{
// 	genauer berechnen?
	offSize = size + QSize(256*2, 256*2);
	composedOffscreenImage = QPixmap(offSize);
	composedOffscreenImage2 = QPixmap(offSize);
	zoomImage = QPixmap(size);
	zoomImage.fill(Qt::white);

	screenmiddle = QPoint(size.width()/2, size.height()/2);
        parentWidget = aParent;
}


LayerManager::~LayerManager()
{
	layers.clear();
}

QPointF LayerManager::getCurrentCoordinate() const
{
	return mapmiddle;
}

QPixmap LayerManager::getImage() const
{
	return composedOffscreenImage;
}

Layer* LayerManager::getLayer() const
{
	Q_ASSERT_X(layers.size()>0, "LayerManager::getLayer()", "No layers were added!");
	return layers.at(0);
}

Layer* LayerManager::getLayer(const QString& layername) const
{
	QListIterator<Layer*> layerit(layers);
	while (layerit.hasNext())
	{
		Layer* l = layerit.next();
		if (l->getLayername() == layername)
			return l;
	}
	return 0;
}

QList<QString> LayerManager::getLayers() const
{
	QList<QString> keys;
	QListIterator<Layer*> layerit(layers);
	while (layerit.hasNext())
	{
		keys.append(layerit.next()->getLayername());
	}
	return keys;
}


void LayerManager::scrollView(const QPoint& point)
{
	if (scrollMutex.tryLock())
	{

	scroll += point;
	zoomImageScroll+=point;
	mapmiddle_px += point;
	scrollMutex.unlock();
	}
	mapmiddle = getLayer()->getMapAdapter()->displayToCoordinate(mapmiddle_px);
	if (!checkOffscreen())
	{

		newOffscreenImage();
	}
	else
	{
		moveWidgets();
	}
}

void LayerManager::moveWidgets()
{
	QListIterator<Layer*> it(layers);
	while (it.hasNext())
	{
		it.next()->moveWidgets(mapmiddle_px);
	}
}

void LayerManager::setView(const QPointF& coordinate)
{
	mapmiddle_px = getLayer()->getMapAdapter()->coordinateToDisplay(coordinate);
	mapmiddle = coordinate;

	//TODO: muss wegen moveTo() raus
	if (!checkOffscreen())
	{
		newOffscreenImage();
	}
	else
	{
		//TODO:
		// verschiebung ausrechnen
		// oder immer neues offscreenimage
		newOffscreenImage();
	}
}

void LayerManager::setView(const QList<QPointF> coordinates)
{
	while (containsAll(coordinates))
	{
		setMiddle(coordinates);
		zoomIn();
		QCoreApplication::processEvents();
	}


	if (!containsAll(coordinates))
	{
		zoomOut();
		QCoreApplication::processEvents();
	}

	parentWidget->update();
}
void LayerManager::setMiddle(QList<QPointF> coordinates)
{
	int sum_x = 0;
	int sum_y = 0;
	for (int i=0; i<coordinates.size(); i++)
	{
		// mitte muss in px umgerechnet werden, da aufgrund der projektion die mittebestimmung aus koordinaten ungenau ist
		QPoint p = getLayer()->getMapAdapter()->coordinateToDisplay(coordinates.at(i));
		sum_x += p.x();
		sum_y += p.y();
	}
	QPointF middle = getLayer()->getMapAdapter()->displayToCoordinate(QPoint(sum_x/coordinates.size(), sum_y/coordinates.size()));
	// middle in px rechnen!

	setView(middle);
}

bool LayerManager::containsAll(QList<QPointF> coordinates) const
{
	QRectF bb = getViewport();
	bool containsall = true;
	for (int i=0; i<coordinates.size(); i++)
	{
		if (!bb.contains(coordinates.at(i)))
			return false;
	}
	return containsall;
}
QPoint LayerManager::getMapmiddle_px() const
{
	return mapmiddle_px;
}

QRectF LayerManager::getViewport() const
{
	QPoint upperLeft = QPoint(mapmiddle_px.x()-screenmiddle.x(), mapmiddle_px.y()+screenmiddle.y());
	QPoint lowerRight = QPoint(mapmiddle_px.x()+screenmiddle.x(), mapmiddle_px.y()-screenmiddle.y());

	QPointF ulCoord = getLayer()->getMapAdapter()->displayToCoordinate(upperLeft);
	QPointF lrCoord = getLayer()->getMapAdapter()->displayToCoordinate(lowerRight);

	QRectF coordinateBB = QRectF(ulCoord, QSizeF( (lrCoord-ulCoord).x(), (lrCoord-ulCoord).y()));
	return coordinateBB;
}

void LayerManager::addLayer(Layer* layer)
{
	layers.append(layer);

	layer->setSize(size);

	connect(layer, SIGNAL(updateRequest(QRectF)),
			  this, SLOT(updateRequest(QRectF)));
	connect(layer, SIGNAL(updateRequest()),
			  this, SLOT(updateRequest()));

	if (layers.size()==1)
	{
		setView(QPointF(0,0));
	}
}

void LayerManager::newOffscreenImage(bool clearImage, bool showZoomImage)
{
// 	qDebug() << "LayerManager::newOffscreenImage()";
	if (refreshMutex.tryLock())
	{
		QPainter painter(&composedOffscreenImage2);
		whilenewscroll = mapmiddle_px;

		if (clearImage)
		{
			composedOffscreenImage2.fill(Qt::white);
		}
		if (showZoomImage)
		{
			painter.drawPixmap(screenmiddle.x()-zoomImageScroll.x(), screenmiddle.y()-zoomImageScroll.y(),zoomImage);
		}
	//only draw basemaps
		for (int i=0; i<layers.count(); i++)
		{
			Layer* l = layers.at(i);
			if (l->isVisible())
			{
				if (l->getLayertype() == Layer::MapLayer)
				{
					l->drawYourImage(&painter, whilenewscroll);
				}
			}
		}

		composedOffscreenImage = composedOffscreenImage2;
		if (scrollMutex.tryLock())
		{
			scroll = mapmiddle_px-whilenewscroll;
// 			scroll = QPoint(0,0);
			scrollMutex.unlock();
		}
                parentWidget->update();
		refreshMutex.unlock();
	}

}

void LayerManager::zoomIn()
{
// 	if (getLayer()->getMapAdapter()->getZoom() < getLayer
	ImageManager::instance()->abortLoading();
	// layer rendern abbrechen?
	zoomImageScroll = QPoint(0,0);

	zoomImage.fill(Qt::white);
	QPixmap tmpImg = composedOffscreenImage.copy(screenmiddle.x()+scroll.x(),screenmiddle.y()+scroll.y(), size.width(), size.height());

	QPainter painter(&zoomImage);
	painter.translate(screenmiddle);
	painter.scale(2, 2);
	painter.translate(-screenmiddle);

	painter.drawPixmap(0,0,tmpImg);

	painter.translate(screenmiddle);
	painter.scale(0.5, 0.5);
	painter.translate(-screenmiddle);
	painter.end();

	QListIterator<Layer*> it(layers);
	//TODO: remove hack, that mapadapters wont get set zoom multiple times
	QList<const MapAdapter*> doneadapters;
	while (it.hasNext())
	{
		Layer* l = it.next();
		if (!doneadapters.contains(l->getMapAdapter()))
		{
			l->zoomIn();
			doneadapters.append(l->getMapAdapter());
		}
	}
	mapmiddle_px = getLayer()->getMapAdapter()->coordinateToDisplay(mapmiddle);
	whilenewscroll = mapmiddle_px;
// 	zoomImageScroll = mapmiddle_px;
	newOffscreenImage();

}

bool LayerManager::checkOffscreen() const
{
	// calculate offscreenImage dimension (px)
	QPoint upperLeft = mapmiddle_px - screenmiddle;
	QPoint lowerRight = mapmiddle_px + screenmiddle;
	QRect viewport = QRect(upperLeft, lowerRight);

	QRect testRect = getLayer()->getOffscreenViewport();

	if (!testRect.contains(viewport))
	{
		return false;
	}

	return true;
}
void LayerManager::zoomOut()
{
	ImageManager::instance()->abortLoading();
	zoomImageScroll = QPoint(0,0);
	QPixmap tmpImg = composedOffscreenImage.copy(screenmiddle.x()+scroll.x(),screenmiddle.y()+scroll.y(), size.width(), size.height());
	QPainter painter(&zoomImage);
	painter.translate(screenmiddle);
	painter.scale(0.5,0.5);
	painter.translate(-screenmiddle);
	painter.drawPixmap(0,0,tmpImg);

	painter.translate(screenmiddle);
	painter.scale(2,2);
	painter.translate(-screenmiddle);

	QListIterator<Layer*> it(layers);
	//TODO: remove hack, that mapadapters wont get set zoom multiple times
	QList<const MapAdapter*> doneadapters;
	while (it.hasNext())
	{
		Layer* l = it.next();
		if (!doneadapters.contains(l->getMapAdapter()))
		{
			l->zoomOut();
			doneadapters.append(l->getMapAdapter());
		}
	}
	mapmiddle_px = getLayer()->getMapAdapter()->coordinateToDisplay(mapmiddle);
	whilenewscroll = mapmiddle_px;
	newOffscreenImage();
}

void LayerManager::setZoom(int zoomlevel)
{
	int current_zoom;
	if (getLayer()->getMapAdapter()->getMinZoom() < getLayer()->getMapAdapter()->getMaxZoom())
	{
		current_zoom = getLayer()->getMapAdapter()->getZoom();
	}
	else
	{
		current_zoom = getLayer()->getMapAdapter()->getMinZoom() - getLayer()->getMapAdapter()->getZoom();
	}


	if (zoomlevel < current_zoom)
	{
		for (int i=current_zoom; i>zoomlevel; i--)
		{
			zoomOut();
		}
	}
	else
	{
		for (int i=current_zoom; i<zoomlevel; i++)
		{
			zoomIn();
		}
	}
}

void LayerManager::mouseEvent(const QMouseEvent* evnt)
{
	QListIterator<Layer*> it(layers);
	while (it.hasNext())
	{
		Layer* l = it.next();
		if (l->isVisible())
		{
			l->mouseEvent(evnt, mapmiddle_px);
		}
	}
}

void LayerManager::updateRequest(QRectF rect)
{
	const QPoint topleft = mapmiddle_px - screenmiddle;

	QPointF c = rect.topLeft();

	if (getViewport().contains(c) || getViewport().contains(rect.bottomRight()))
	{
// 		QPoint point = getLayer()->getMapAdapter()->coordinateToDisplay(c);
// 		QPoint finalpoint = point-topleft;
// 		QRect rect_px = QRect(int(finalpoint.x()-(rect.width()-1)/2), int(finalpoint.y()-(rect.height()-1)/2),
// 									 int(rect.width()+1), int(rect.height()+1));
//
// 		mapcontrol->updateRequest(rect_px);
		parentWidget->update();
// 		newOffscreenImage();
	}
}
void LayerManager::updateRequest()
{
	newOffscreenImage();
}
void LayerManager::forceRedraw()
{
	newOffscreenImage();
}
void LayerManager::removeZoomImage()
{
	zoomImage.fill(Qt::white);
	forceRedraw();
}

void LayerManager::drawGeoms(QPainter* painter)
{
	QListIterator<Layer*> it(layers);
	while (it.hasNext())
	{
		Layer* l = it.next();
		if (l->getLayertype() == Layer::GeometryLayer && l->isVisible())
		{
			l->drawYourGeometries(painter, mapmiddle_px, getLayer()->getOffscreenViewport());
		}
	}
}
void LayerManager::drawImage(QPainter* painter)
{
	painter->drawPixmap(-scroll.x()-screenmiddle.x(),
								-scroll.y()-screenmiddle.y(),
									  composedOffscreenImage);
}


/*!
    \fn LayerManager::resize(QSize newSize)
 */
void LayerManager::setSize(QSize newSize)
{
	offSize = newSize + QSize(256*2, 256*2);
	composedOffscreenImage = QPixmap(offSize);
	composedOffscreenImage2 = QPixmap(offSize);
	zoomImage = QPixmap(newSize);
	zoomImage.fill(Qt::white);

	screenmiddle = QPoint(newSize.width()/2, newSize.height()/2);

	QListIterator<Layer*> it(layers);
	while (it.hasNext())
	{
		Layer* l = it.next();
		l->setSize(newSize);
	}

	newOffscreenImage();

}
