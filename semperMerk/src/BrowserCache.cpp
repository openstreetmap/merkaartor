#include "BrowserCache.h"

#include <QIODevice>
#include <QNetworkCacheMetaData>

BrowserCache::BrowserCache(QObject *parent) :
    QAbstractNetworkCache(parent)
{
    m_cache = new QCache<int, BrowserCacheItem>(3000000);
}

BrowserCache::~BrowserCache ()
{
    delete m_cache;
}

qint64 BrowserCache::cacheSize() const
{
    return m_cache->size();
}

QIODevice * BrowserCache::data(const QUrl &url)
{
    BrowserCacheItem* a = m_cache->take(qHash(url.toString()));
    if (!a)
        return NULL;

    QBuffer* buf = new QBuffer(&a->data);
    buf->open(QIODevice::ReadWrite);
    return buf;
}

void BrowserCache::insert(QIODevice *device)
{
    if (!m_devices.contains(device)) {
        delete device;
        return;
    }
    BrowserCacheItem* c = m_devices.take(device);
    m_cache->insert(qHash(c->meta.url().toString()), c, sizeof(BrowserCacheItem) + c->data.size());
    delete device;
}

QNetworkCacheMetaData BrowserCache::metaData(const QUrl &url)
{
    BrowserCacheItem* c = m_cache->object(qHash(url.toString()));
    if (c)
        return c->meta;
    else {
        QNetworkCacheMetaData ret;
        return ret;
    }

}

QIODevice * BrowserCache::prepare(const QNetworkCacheMetaData &metaData)
{
    BrowserCacheItem* c = new BrowserCacheItem;
    c->meta = metaData;
    QBuffer* buf = new QBuffer(&c->data);
    buf->open(QIODevice::ReadWrite);
    m_devices[buf] = c;
    return buf;
}

bool BrowserCache::remove(const QUrl &url)
{
    return m_cache->remove(qHash(url.toString()));
}

void BrowserCache::updateMetaData(const QNetworkCacheMetaData &metaData)
{
    BrowserCacheItem* c = m_cache->object(qHash(metaData.url().toString()));
    if (c) {
        c->meta = metaData;
    }
}

void BrowserCache::clear()
{
    m_cache->clear();
}


