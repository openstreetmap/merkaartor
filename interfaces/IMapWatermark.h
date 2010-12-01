#ifndef IMAPWATERMARK_H
#define IMAPWATERMARK_H

#include <QPixmap>
#include <QUrl>
#include <QStringList>

class IMapWatermark
{
public:
    virtual QString getLogoHtml() = 0;
    virtual QString getAttributionsHtml(const QRectF& bbox, const QRect& screen) = 0;
};

Q_DECLARE_INTERFACE ( IMapWatermark,
                      "com.cbsoft.Merkaartor.IMapWatermark/1.0" )

#endif
