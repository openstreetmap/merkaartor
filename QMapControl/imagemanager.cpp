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
#include "imagemanager.h"
ImageManager* ImageManager::m_Instance = 0;
ImageManager::ImageManager(QObject* parent)
	:QObject(parent), emptyPixmap(QPixmap(1,1)), net(new MapNetwork(this))
{
	emptyPixmap.fill(Qt::transparent);
	
	if (QPixmapCache::cacheLimit() <= 20000)
	{
		QPixmapCache::setCacheLimit(20000);	// in kb
	}
}


ImageManager::~ImageManager()
{
	delete net;
}

QPixmap ImageManager::getImage(const QString& host, const QString& url)
{
// 	qDebug() << "ImageManager::getImage";
	QPixmap pm;
	pm.fill(Qt::black);
	
	// is image cached or currently loading?
	if (!QPixmapCache::find(url, pm) && !net->imageIsLoading(url))
// 	if (!images.contains(url) && !net->imageIsLoading(url))
	{
		// load from net, add empty image
		net->loadImage(host, url);
// 		QPixmapCache::insert(url, emptyPixmap);
		return emptyPixmap;
	}
	return pm;
}

void ImageManager::receivedImage(const QPixmap pixmap, const QString& url)
{
// 	qDebug() << "ImageManager::receivedImage";
	QPixmapCache::insert(url, pixmap);
// 	images[url] = pixmap;
	
// 	((Layer*)this->parent())->imageReceived();
	emit(imageReceived());
}

void ImageManager::loadingQueueEmpty()
{
	emit(loadingFinished());
// 	((Layer*)this->parent())->removeZoomImage();
// 	qDebug() << "size of image-map: " << images.size();
// 	qDebug() << "size: " << QPixmapCache::cacheLimit();
}

void ImageManager::abortLoading()
{
	net->abortLoading();
}
void ImageManager::setProxy(QString host, int port)
{
	net->setProxy(host, port);
}
