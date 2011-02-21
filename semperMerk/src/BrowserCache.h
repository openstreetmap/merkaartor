#ifndef BROWSERCACHE_H
#define BROWSERCACHE_H

#include <QAbstractNetworkCache>
#include <QCache>
#include <QBuffer>
#include <QHash>

struct BrowserCacheItem {
    QNetworkCacheMetaData meta;
    QByteArray data;
};

class BrowserCache : public QAbstractNetworkCache
{
    Q_OBJECT
public:
    explicit BrowserCache(QObject *parent = 0);
    virtual	~BrowserCache ();

public:
    virtual qint64	cacheSize () const;
    virtual QIODevice *	data ( const QUrl & url );
    virtual void	insert ( QIODevice * device );
    virtual QNetworkCacheMetaData	metaData ( const QUrl & url );
    virtual QIODevice *	prepare ( const QNetworkCacheMetaData & metaData );
    virtual bool	remove ( const QUrl & url );
    virtual void	updateMetaData ( const QNetworkCacheMetaData & metaData );

public slots:
    virtual void	clear ();

private:
    QCache<int, BrowserCacheItem>* m_cache;
    QHash<QIODevice*, BrowserCacheItem*> m_devices;

};

#endif // BROWSERCACHE_H
