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

#include "imagemanager.h"
#ifdef USE_WEBKIT
#include "browserimagemanager.h"
#endif

#include <QWidget>

LayerManager::LayerManager(QWidget* aParent, QSize size)
	:scroll(QPoint(0,0)), size(size), whilenewscroll(QPoint(0,0))
{
// 	genauer berechnen?
	offSize = size *2;
	composedOffscreenImage = QPixmap(offSize);
	composedOffscreenImage2 = QPixmap(offSize);
	zoomImage = QPixmap(size);
	zoomImage.fill(Qt::transparent);

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
	if (!layers.size())
		return NULL;
	return layers.first();
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
	scroll += point;
	zoomImageScroll+=point;
	mapmiddle_px += point;
	mapmiddle = getLayer()->getMapAdapter()->displayToCoordinate(mapmiddle_px);
}

void LayerManager::setView(const QPointF& coordinate, bool newImage)
{
	mapmiddle_px = getLayer()->getMapAdapter()->coordinateToDisplay(coordinate);
	mapmiddle = coordinate;

	if (!newImage)
		return;

	newOffscreenImage();
}


void LayerManager::setView(const QList<QPointF> coordinates)
{
	setMiddle(coordinates);
	while ((!containsAll(coordinates)) && (getLayer()->getMapAdapter()->getAdaptedZoom() > getLayer()->getMapAdapter()->getAdaptedMinZoom()))
	{
		setMiddle(coordinates);
		backZoomOut();
		//QCoreApplication::processEvents();
	}

	while ((containsAll(coordinates)) && (getLayer()->getMapAdapter()->getAdaptedZoom() < getLayer()->getMapAdapter()->getAdaptedMaxZoom()))
	{
		setMiddle(coordinates);
		backZoomIn();
		//QCoreApplication::processEvents();
	}

// 	if (getLayer()->getMapAdapter()->getAdaptedZoom() > getLayer()->getMapAdapter()->getAdaptedMinZoom()) {
// 		setMiddle(coordinates);
// 		backZoomOut();
// 	}

	if ((!containsAll(coordinates)) && (getLayer()->getMapAdapter()->getAdaptedZoom() > getLayer()->getMapAdapter()->getAdaptedMinZoom()))
	{
		setMiddle(coordinates);
		backZoomOut();
	}

	ImageManager::instance()->abortLoading();
#ifdef USE_WEBKIT
	BrowserImageManager::instance()->abortLoading();
#endif

//	parentWidget->update();
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

	setView(middle, false);
}

bool LayerManager::containsAll(QList<QPointF> coordinates) const
{
	QRectF bb = getViewport();
	bool containsall = true;
	for (int i=0; i<coordinates.size(); i++)
	{
		QPointF p = coordinates.at(i);
		if (!bb.contains(p))
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

void LayerManager::addLayer(Layer* layer, int pos)
{
	if (pos  < 0)
		layers.append(layer);
	else
		layers.insert(pos, layer);

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

void LayerManager::removeLayer()
{
	Q_ASSERT_X(layers.size()>0, "LayerManager::removeLayer()", "No layers existing!");
	ImageManager::instance()->abortLoading();
#ifdef USE_WEBKIT
	BrowserImageManager::instance()->abortLoading();
#endif
	layers.removeAt(0);
}

void LayerManager::removeLayer(const QString& aLyerId)
{
	Q_ASSERT_X(layers.size()>0, "LayerManager::removeLayer()", "No layers existing!");
	ImageManager::instance()->abortLoading();
#ifdef USE_WEBKIT
	BrowserImageManager::instance()->abortLoading();
#endif
	layers.removeAt(getLayers().indexOf(aLyerId));
}

void LayerManager::newOffscreenImage(bool clearImage, bool showZoomImage)
{
// 	qDebug() << "LayerManager::newOffscreenImage()";
// 	if (refreshMutex.tryLock())
	{
		whilenewscroll = mapmiddle_px;

		if (clearImage) {
			composedOffscreenImage2.fill(Qt::transparent);
		}

		QPainter painter(&composedOffscreenImage2);
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
// 		if (scrollMutex.tryLock())
		{
			scroll = mapmiddle_px-whilenewscroll;
// 			scroll = QPoint(0,0);
// 			scrollMutex.unlock();
		}
		parentWidget->update();
//		refreshMutex.unlock();
	}

}

void LayerManager::backZoomIn()
{
	QListIterator<Layer*> it(layers);
	//TODO: remove hack, that mapadapters wont get set zoom multiple times
	QList<const IMapAdapter*> doneadapters;
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
}

void LayerManager::zoomIn()
{
	ImageManager::instance()->abortLoading();
#ifdef USE_WEBKIT
	BrowserImageManager::instance()->abortLoading();
#endif

	zoomImageScroll = QPoint(0,0);

	zoomImage.fill(Qt::transparent);
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

	backZoomIn();

	//newOffscreenImage();

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
void LayerManager::backZoomOut()
{
	QListIterator<Layer*> it(layers);
	//TODO: remove hack, that mapadapters wont get set zoom multiple times
	QList<const IMapAdapter*> doneadapters;
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
}

void LayerManager::zoomOut()
{
	ImageManager::instance()->abortLoading();
#ifdef USE_WEBKIT
	BrowserImageManager::instance()->abortLoading();
#endif

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

	backZoomOut();

	//newOffscreenImage();
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
	zoomImage.fill(Qt::transparent);
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
void LayerManager::setSize()
{
	setSize(parentWidget->size());
}

void LayerManager::setSize(QSize newSize)
{
	offSize = newSize *2;
	composedOffscreenImage = QPixmap(offSize);
	composedOffscreenImage2 = QPixmap(offSize);
	zoomImage = QPixmap(newSize);
	zoomImage.fill(Qt::transparent);

	screenmiddle = QPoint(newSize.width()/2, newSize.height()/2);

	QListIterator<Layer*> it(layers);
	while (it.hasNext())
	{
		Layer* l = it.next();
		l->setSize(newSize);
	}

	newOffscreenImage();

}

int LayerManager::getCurrentZoom() const
{
	return getLayer()->getMapAdapter()->getZoom();
}
