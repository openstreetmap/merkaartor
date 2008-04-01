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

#include <QDateTime>

ImageManager* ImageManager::m_ImageManagerInstance = 0;

ImageManager::ImageManager(QObject* parent)
	:IImageManager(parent), emptyPixmap(QPixmap(1,1)), net(new MapNetwork(this))
{
	emptyPixmap.fill(Qt::transparent);

	if (QPixmapCache::cacheLimit() <= 20000)
	{
		QPixmapCache::setCacheLimit(20000);	// in kb
	}
}

void ImageManager::setCacheDir(const QDir& path)
{
	cacheDir = path;
	cacheSize = 0;
	if (!cacheDir.exists()) {
		cacheDir.mkpath(cacheDir.absolutePath());
	} else {
		cacheInfo = cacheDir.entryInfoList(QDir::Files, QDir::Time);
		for (int i=0; i<cacheInfo.size(); i++) {
			cacheSize += cacheInfo[i].size();
		}
	}
}

void ImageManager::setCacheMaxSize(int max)
{
	cacheMaxSize = max*1024*1024;
}


ImageManager::~ImageManager()
{
	delete net;
}

//QPixmap ImageManager::getImage(const QString& host, const QString& url)
QPixmap ImageManager::getImage(MapAdapter* anAdapter, int x, int y, int z)
{
// 	qDebug() << "ImageManager::getImage";
	QPixmap pm;

	QString host = anAdapter->getHost();
	QString url = anAdapter->getQuery(x, y, z);
	QString strHash = QString("%1%2").arg(host).arg(url);
	QString hash = QString(strHash.toAscii().toBase64());

	// is image in picture cache
	if (QPixmapCache::find(hash, pm))
		return pm;

	// disk cache?
	if (useDiskCache(hash + ".png")) {
		if (pm.load(cacheDir.absolutePath() + "/" + hash + ".png"))
			return pm;
		else
			pm.fill(Qt::black);
	}

	// currently loading?
	if (!net->imageIsLoading(hash))
	{
		// load from net, add empty image
		net->loadImage(hash, host, url);
		emit(imageRequested());
		return emptyPixmap;
	}
	return pm;
}

bool ImageManager::useDiskCache(QString filename)
{
	if (!cacheMaxSize)
		return false;

	if (!cacheDir.exists(filename))
		return false;

	int random = qrand() % 100;
	QFileInfo info(cacheDir.absolutePath() + "/" + filename);
	int days = info.lastModified().daysTo(QDateTime::currentDateTime());

	return  random < (10 * days) ? false : true;
}

//QPixmap ImageManager::prefetchImage(const QString& host, const QString& url)
QPixmap ImageManager::prefetchImage(MapAdapter* anAdapter, int x, int y, int z)
{
	QString hash = QString("%1_%2_%3_%4").arg(anAdapter->getName()).arg(x).arg(y).arg(z);

	prefetch.append(hash);
	return getImage(anAdapter, x, y, z);
}

void ImageManager::receivedImage(const QPixmap pixmap, const QString& hash)
{
// 	qDebug() << "ImageManager::receivedImage";
	if (pixmap.isNull()) {
		QPixmap pm(256, 256);
		pm.fill(Qt::gray);
		QPixmapCache::insert(hash, pm);
	} else {
		QPixmapCache::insert(hash, pixmap);
		if (cacheMaxSize) {
			pixmap.save(cacheDir.absolutePath() + "/" + hash + ".png");
			QFileInfo info(cacheDir.absolutePath() + "/" + hash + ".png");
			cacheInfo.append(info);
			cacheSize += info.size();

			adaptCache();
		}
	}

	if (prefetch.contains(hash))
	{
		prefetch.remove(prefetch.indexOf(hash));
	}
	emit(imageReceived());
}

void ImageManager::adaptCache()
{
	QFileInfo info;
	while (cacheSize > cacheMaxSize) {
		info = cacheInfo.takeFirst();
		cacheDir.remove(info.fileName());
		cacheSize -= info.size();
	}
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
	loadingQueueEmpty();
}
void ImageManager::setProxy(QString host, int port)
{
	net->setProxy(host, port);
}

