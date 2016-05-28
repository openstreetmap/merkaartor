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
#include "MerkaartorPreferences.h"
#include "IMapAdapter.h"

#include <QDateTime>
#include <QCryptographicHash>

ImageManager* ImageManager::m_ImageManagerInstance = 0;

ImageManager::ImageManager(QObject* parent)
    :QObject(parent), emptyPixmap(QPixmap(1,1)), net(new MapNetwork(this))
{
    emptyPixmap.fill(Qt::transparent);

#ifndef _MOBILE
    m_dataCache.setMaxCost(20000000); // 20mb
#else
    m_dataCache.setMaxCost(5000000); // 5mb
#endif
}

ImageManager::~ImageManager()
{
    net->abortLoading();
    delete net;
}

QByteArray ImageManager::getData(IMapAdapter* anAdapter, const QString &url)
{
    QString host = anAdapter->getHost();
    QString strHash = anAdapter->getName() + url;
    QString hash = QString(strHash.toLatin1().toBase64());
    if (hash.size() > 255) {
        QCryptographicHash crypt(QCryptographicHash::Md5);
        crypt.addData(hash.toLatin1());
        hash = QString(crypt.result().toHex());
    }

    QByteArray ba;
    if (m_dataCache.contains(hash)) {
        QBuffer* buf = m_dataCache.object(hash);
        return buf->data();
    }

    // currently loading?
    if (!net->isLoading(hash))
    {
        // load from net, add empty image
        net->load(hash, host, url);
        emit(dataRequested());
        return ba;
    }
    return ba;
}

QImage ImageManager::getImage(IMapAdapter* anAdapter, const QString &url)
{
// 	qDebug() << "ImageManager::getImage";

    QString host = anAdapter->getHost();
    QString strHash = anAdapter->getName() + url;
    QString hash = QString(strHash.toLatin1().toBase64());
    if (hash.size() > 255) {
        QCryptographicHash crypt(QCryptographicHash::Md5);
        crypt.addData(hash.toLatin1());
        hash = QString(crypt.result().toHex());
    }

    /*	QPixmap pm(anAdapter->getTileSize(), anAdapter->getTileSize());
        pm.fill(Qt::black);*/
    //	QPixmap pm(emptyPixmap);
    QImage pm;

    // is image in picture cache
    if (m_dataCache.contains(hash)) {
        pm.loadFromData(m_dataCache.object(hash)->data());
        return pm;
    }

    // disk cache?
    if (anAdapter->isTiled() && useDiskCache(hash + ".png")) {
        if (pm.load(cacheDir.absolutePath() + "/" + hash + ".png")) {
            return pm;
        }
    }

    if (M_PREFS->getOfflineMode())
        return pm;

    // currently loading?
    if (!net->isLoading(hash))
    {
        // load from net, add empty image
        net->load(hash, host, url);
        emit(dataRequested());
        return pm;
    }
    return pm;
}

//QPixmap ImageManager::prefetchImage(const QString& host, const QString& url)
QImage ImageManager::prefetchImage(IMapAdapter* anAdapter, int x, int y, int z)
{
    QString host = anAdapter->getHost();
    QString url = anAdapter->getQuery(x, y, z);
    QString strHash = QString("%1%2").arg(anAdapter->getName()).arg(url);
    QString hash = QString(strHash.toLatin1().toBase64());

    prefetch.append(hash);
    return getImage(anAdapter, anAdapter->getQuery(x, y, z));
}

void ImageManager::receivedData(const QByteArray& ba, const QHash<QString, QString>& headers, const QString& hash)
{
// 	qDebug() << "ImageManager::receivedImage";

    QImage img = QImage::fromData(ba);
    foreach (QString k, headers.keys()) {
        img.setText(k, headers[k]);
    }
    QBuffer* buf = new QBuffer();
    buf->open(QIODevice::WriteOnly);
    img.save(buf, "PNG");
    buf->close();
    m_dataCache.insert(hash, buf, buf->data().size());
    if (cacheMaxSize || cachePermanent) {

        if (!img.isNull()) {
            img.save(cacheDir.absolutePath() + "/" + hash + ".png");
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
    emit(dataReceived());
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

void ImageManager::setCacheDir(const QDir& path)
{
    cacheDir = path;
    cacheSize = 0;
    if (!cacheDir.exists()) {
        cacheDir.mkpath(cacheDir.absolutePath());
    } else {
        cacheInfo = cacheDir.entryInfoList(QDir::Files, QDir::Time | QDir::Reversed);
        for (int i=0; i<cacheInfo.size(); i++) {
            cacheSize += cacheInfo[i].size();
        }
    }
}

QDir ImageManager::getCacheDir()
{
    return cacheDir;
}

void ImageManager::setCacheMaxSize(int max)
{
    cacheMaxSize = max*1024*1024;
}
