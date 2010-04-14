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
#include <QNetworkProxy>
#include <QNetworkAccessManager>

#include "IMapAdapter.h"

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

        qDebug() << "Coord: " << tllat << ", " << tllon << ", " << brlat << "," << brlon;
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

        qDebug() << "Size: " << ox << ", " << oy << ", " << x1 << "," << y1;

        sw = x1 - ox;
        sh = y1 - oy;

        qDebug() << "---- " << sw << ", " << sh;

    }
    if (msg.startsWith("ReqSize")) {
        QStringList tokens = msg.split(" ");
        Q_ASSERT(tokens.size() == 3);

        int w, h;
        w = int(tokens[1].toDouble());
        h = int(tokens[2].toDouble());

        qDebug() << "ReqSize: " << w << ", " << h ;
    }
}

void BrowserWebPage::launchRequest ( const QUrl & url )
{
    sw = sh = 0;
    mainFrame()->load(url);
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
    :QObject(parent), emptyPixmap(QPixmap(1,1)), errorPixmap(QPixmap(512,512)), page(0)
{
    errorPixmap.fill(Qt::gray);
    QPainter P(&errorPixmap);
    P.fillRect(0, 0, 511, 511, QBrush(Qt::red, Qt::DiagCrossPattern));
    emptyPixmap.fill(Qt::transparent);

    if (QPixmapCache::cacheLimit() <= 20000)
    {
        QPixmapCache::setCacheLimit(20000); // in kb
    }

    page = new BrowserWebPage();
    page->setViewportSize(QSize(1024, 1024));

    connect(page, SIGNAL(loadFinished(bool)), this, SLOT(pageLoadFinished(bool)));

    timeoutTimer = new QTimer();
    connect(timeoutTimer, SIGNAL(timeout()), this, SLOT(timeout()));
    timeoutTimer->setInterval(30000);
}
#endif // BROWSERIMAGEMANAGER_IS_THREADED


BrowserImageManager::~BrowserImageManager()
{
    delete timeoutTimer;
}

QPixmap BrowserImageManager::getImage(IMapAdapter* anAdapter, int x, int y, int z)
{
    QString url = anAdapter->getQuery(x, y, z);
    return getImage(anAdapter, url);
}

QPixmap BrowserImageManager::getImage(IMapAdapter* anAdapter, QString url)
{
//	QPixmap pm(emptyPixmap);
    QPixmap pm;

    QString host = anAdapter->getHost();
    QString strHash = QString("%1%2").arg(anAdapter->getName()).arg(url);
    QString hash = QString(strHash.toAscii().toBase64());
    if (hash.size() > 255) {
        QCryptographicHash crypt(QCryptographicHash::Md5);
        crypt.addData(hash.toLatin1());
        hash = QString(crypt.result().toHex());
    }

    // is image in picture cache
    if (QPixmapCache::find(hash, pm)) {
        qDebug() << "BrowserImageManager::QPixmapCache hit!";
        return pm;
    }

    // disk cache?
    if (anAdapter->isTiled() && useDiskCache(hash + ".png")) {
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
    qDebug() << "BrowserImageManager::launchRequest: " << QString(R.host).append(R.url) << " Hash: " << R.hash;

    QUrl u = QUrl( R.url);

    page->networkAccessManager()->setProxy(M_PREFS->getProxy(u));
    page->launchRequest(u);
    requestActive = true;
#ifndef BROWSERIMAGEMANAGER_IS_THREADED
    timeoutTimer->start();
#endif
}

void BrowserImageManager::pageLoadFinished(bool ok)
{
    mutex.lock();

    timeoutTimer->stop();

    if (loadingRequests.isEmpty()) {
        mutex.unlock();
        return;
    }

    LoadingRequest R = loadingRequests.dequeue();
    requestActive = false;

    if (ok && page->sw && page->sh) {
        qDebug() << "BrowserImageManager::pageLoadFinished: " << " Hash: " << R.hash;
        QPixmap pt(page->sw, page->sh);
        QPainter P(&pt);
        page->mainFrame()->render(&P, QRegion(0,0,page->sw,page->sh));
        P.end();

        if (page->sw != BROWSER_TILE_SIZE || page->sh != BROWSER_TILE_SIZE) {
            QPixmap tmpPx = pt.scaled(QSize(BROWSER_TILE_SIZE, BROWSER_TILE_SIZE));
            pt = tmpPx;
        }

        receivedImage(pt, R.hash);
    } else {
        loadingRequests.enqueue(R);
        qDebug() << "BrowserImageManager::pageLoadFinished - Error: " << " Hash: " << R.hash;
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
    if (!(pixmap.isNull())) {
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
}

void BrowserImageManager::abortLoading()
{
    //qDebug() << "BrowserImageManager::abortLoading";
    page->triggerAction(QWebPage::Stop);
    if (!loadingRequests.isEmpty()) {
        LoadingRequest R = loadingRequests.dequeue();
        loadingRequests.clear();
        loadingRequests.enqueue(R);
    }
    loadingQueueEmpty();
}

#ifdef BROWSERIMAGEMANAGER_IS_THREADED
void BrowserImageManager::run()
{
    page = new BrowserWebPage();
    page->setViewportSize(QSize(1024, 1024));

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
        if ((requestDuration++) > 100) {
            requestDuration = 0;
            page->triggerAction(QWebPage::Stop);
            qDebug() << "BrowserImageManager Timeout";
        }
    }

    mutex.unlock();
}
#else
void BrowserImageManager::timeout()
{
    qDebug() << "BrowserImageManager::timeout";
    page->triggerAction(QWebPage::Stop);
    pageLoadFinished(false);
}
#endif // BROWSERIMAGEMANAGER_IS_THREADED
