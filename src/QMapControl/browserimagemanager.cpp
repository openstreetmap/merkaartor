/***************************************************************************
 *   Copyright (C) 2008 by Chris Browet                                    *
 *   cbro@semperpax.com                                                    *
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
#include "browserimagemanager.h"
#include "Preferences/MerkaartorPreferences.h"

#include <QApplication>
#include <QDateTime>
#include <QPixmapCache>
#include <QPainter>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QTimer>
#include <QMutex>

#define MAX_REQ 1
#define BROWSER_TILE_SIZE 512
#define str(x) # x

void BrowserWebPage::javaScriptConsoleMessage ( const QString & message, int lineNumber, const QString & sourceID )
{
	Q_UNUSED(message)
	Q_UNUSED(lineNumber)
	Q_UNUSED(sourceID)
	// FIXME Seems to crash at times (see http://trac.openstreetmap.org/ticket/1194)
	//QString s = QString("%1 at %2, %3").arg(message).arg(QString::number(lineNumber)).arg(sourceID);
	//printf("%s\n", s);
}

void BrowserWebPage::javaScriptAlert ( QWebFrame * frame, const QString & msg ) 
{
	Q_UNUSED(frame)
	//QMessageBox::information(NULL, tr("Javascript alert"), msg);

	if (msg.startsWith("Coord")) {

		tllat = 90.0;
		tllon = -180.0;
		brlat = -90.0;
		brlon = 180.0;
		
		QStringList tokens = msg.split(" ");
		Q_ASSERT(tokens.size() == 5);

		tllat = tokens[1].toDouble();
		tllon = tokens[2].toDouble();
		brlat = tokens[3].toDouble();
		brlon = tokens[4].toDouble();

		qDebug() << tllon << ", " << tllat << ", " << brlon << "," << brlat;
	} else
	if (msg.startsWith("Size")) {

		sw = BROWSER_TILE_SIZE;
		sh = BROWSER_TILE_SIZE;

		QStringList tokens = msg.split(" ");
		Q_ASSERT(tokens.size() == 5);

		int ox, oy, x1, y1;
		ox = int(tokens[1].toDouble());
		oy = int(tokens[2].toDouble());
		x1 = int(tokens[3].toDouble());
		y1 = int(tokens[4].toDouble());

		qDebug() << ox << ", " << oy << ", " << x1 << "," << y1;

		sw = x1 - ox;
		sh = y1 - oy;

		qDebug() << sw << ", " << sh;
	}
}


BrowserImageManager* BrowserImageManager::m_BrowserImageManagerInstance = 0;
QMutex mutex;

#ifdef BROWSERIMAGEMANAGER_IS_THREADED
BrowserImageManager::BrowserImageManager(QObject* parent)
	:QThread(parent), emptyPixmap(QPixmap(1,1)), requestActive(false), page(0)
{
	emptyPixmap.fill(Qt::transparent);

	if (QPixmapCache::cacheLimit() <= 20000)
	{
		QPixmapCache::setCacheLimit(20000);	// in kb
	}
}
#else
BrowserImageManager::BrowserImageManager(QObject* parent)
    :QObject(parent), emptyPixmap(QPixmap(1,1)), page(0)
{   
    emptyPixmap.fill(Qt::transparent);
    
    if (QPixmapCache::cacheLimit() <= 20000)
    {   
        QPixmapCache::setCacheLimit(20000); // in kb
    }
    
    page = new BrowserWebPage();
    page->setViewportSize(QSize(1024, 1024));

    connect(page, SIGNAL(loadFinished(bool)), this, SLOT(pageLoadFinished(bool)));
}
#endif // BROWSERIMAGEMANAGER_IS_THREADED


BrowserImageManager::~BrowserImageManager()
{
}

//QPixmap BrowserImageManager::getImage(const QString& host, const QString& url)
QPixmap BrowserImageManager::getImage(IMapAdapter* anAdapter, int x, int y, int z)
{
// 	qDebug() << "BrowserImageManager::getImage";
	QPixmap pm(emptyPixmap);

	QString host = anAdapter->getHost();
	QString url = anAdapter->getQuery(x, y, z);
	QString strHash = QString("%1;%2;%3;%4;%5").arg(anAdapter->getName()).arg(QString::number(x)).arg(QString::number(y)).arg(QString::number(z)).arg(anAdapter->getTileSize());
	QString hash = QString(strHash.toAscii().toBase64());
	if (hash.size() > 255) {
		QCryptographicHash crypt(QCryptographicHash::Md5);
		crypt.addData(hash.toLatin1());
		hash = QString(crypt.result().toHex());
	}

	// is image in picture cache
	if (QPixmapCache::find(hash, pm))
		return pm;

	// disk cache?
    if (useDiskCache(hash + ".png")) {
		if (pm.load(cacheDir.absolutePath() + "/" + hash + ".png")) {
			QPixmapCache::insert(hash, pm);
			return pm;
		}
	}
	if (M_PREFS->getOfflineMode())
		return pm;

	LoadingRequest LR(hash, host, url);
	if (loadingRequests.contains(LR))
		return pm;

	loadingRequests.enqueue(LR);
	emit(imageRequested());

#ifndef BROWSERIMAGEMANAGER_IS_THREADED
	if (loadingRequests.size() <= MAX_REQ)
		launchRequest();
#endif

	return pm;
}

void BrowserImageManager::launchRequest()
{
	if (loadingRequests.isEmpty())
		return;
//	LoadingRequest* R = loadingRequests.dequeue();
	LoadingRequest R = loadingRequests.head();
	qDebug() << "getting: " << QString(R.host).append(R.url);

	QUrl u = QUrl( R.url);

	page->mainFrame()->load(u);
	requestActive = true;
}

void BrowserImageManager::pageLoadFinished(bool ok)
{
	mutex.lock();

	if (loadingRequests.isEmpty()) {
		mutex.unlock();
		return;
	}

	LoadingRequest R = loadingRequests.dequeue();
	requestActive = false;

	if (ok) {
		QPixmap pt(page->sw, page->sh);
		QPainter P(&pt);
		page->mainFrame()->render(&P, QRegion(0,0,page->sw,page->sh));
		P.end();
		
		if (page->sw != BROWSER_TILE_SIZE || page->sh != BROWSER_TILE_SIZE) {
			QPixmap tmpPx = pt.scaled(QSize(BROWSER_TILE_SIZE, BROWSER_TILE_SIZE));
			pt = tmpPx;
		}

		receivedImage(pt, R.hash);
	}

	mutex.unlock();

	if (loadingRequests.isEmpty())
		loadingQueueEmpty();
#ifndef BROWSERIMAGEMANAGER_IS_THREADED
	else
		launchRequest();
#endif
}

void BrowserImageManager::slotLoadProgress(int p)
{
	if (!(p < 100)) {
	}
}

//QPixmap BrowserImageManager::prefetchImage(const QString& host, const QString& url)
QPixmap BrowserImageManager::prefetchImage(IMapAdapter* anAdapter, int x, int y, int z)
{
	QString host = anAdapter->getHost();
	QString url = anAdapter->getQuery(x, y, z);
	QString strHash = QString("%1;%2;%3;%4;%5").arg(anAdapter->getName()).arg(QString::number(x)).arg(QString::number(y)).arg(QString::number(z)).arg(anAdapter->getTileSize());
	QString hash = QString(strHash.toAscii().toBase64());

	prefetch.append(hash);
	return getImage(anAdapter, x, y, z);
}

void BrowserImageManager::receivedImage(const QPixmap& pixmap, const QString& hash)
{
// 	qDebug() << "BrowserImageManager::receivedImage";
	if (pixmap.isNull()) {
		QPixmap pm(256, 256);
		pm.fill(Qt::gray);
		QPixmapCache::insert(hash, pm);
	} else {
		QPixmapCache::insert(hash, pixmap);
		QString strHash = QByteArray::fromBase64(hash.toAscii());

		if (cacheMaxSize && !strHash.startsWith("Yahoo")) {
			pixmap.save(cacheDir.absolutePath() + "/" + hash + ".png");
			QFileInfo info(cacheDir.absolutePath() + "/" + hash + ".png");
			cacheInfo.append(info);
			cacheSize += info.size();

			adaptCache();
		}
	}

	if (prefetch.contains(hash))
	{
		prefetch.removeAt(prefetch.indexOf(hash));
	}
	emit(imageReceived());
}

void BrowserImageManager::loadingQueueEmpty()
{
	emit(loadingFinished());
// 	((Layer*)this->parent())->removeZoomImage();
// 	qDebug() << "size of image-map: " << images.size();
// 	qDebug() << "size: " << QPixmapCache::cacheLimit();
}

void BrowserImageManager::abortLoading()
{
	if (!loadingRequests.isEmpty()) {
		LoadingRequest R = loadingRequests.dequeue();
		loadingRequests.clear();
		loadingRequests.enqueue(R);
	}
}

void BrowserImageManager::setProxy(QString host, int port)
{
	if (!host.isEmpty()) {
		proxy.setType(QNetworkProxy::HttpCachingProxy);
		proxy.setHostName(host);
		proxy.setPort(port);
	}
	page->networkAccessManager()->setProxy(proxy);
}

#ifdef BROWSERIMAGEMANAGER_IS_THREADED
void BrowserImageManager::run()
{
	page = new BrowserWebPage();
	page->setViewportSize(QSize(1024, 1024));
	page->networkAccessManager()->setProxy(proxy);

	QTimer theTimer;
	theTimer.start(100);
	connect(page, SIGNAL(loadFinished(bool)), this, SLOT(pageLoadFinished(bool)));
	connect (&theTimer, SIGNAL(timeout()), this, SLOT(checkRequests()), Qt::DirectConnection);

	exec();

	delete page;
}

void BrowserImageManager::checkRequests() 
{
	mutex.lock();

	if (!requestActive) {
		requestDuration = 0;
		launchRequest();
	} else {
		if ((requestDuration++) > 50) {
			requestDuration = 0;
			page->triggerAction(QWebPage::Stop);
			qDebug() << "BrowserImageManager Timeout";
		}
	}
	
	mutex.unlock();
}
#endif // BROWSERIMAGEMANAGER_IS_THREADED
