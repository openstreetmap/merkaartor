#ifndef IMAPWATERMARK_H
#define IMAPWATERMARK_H

#include <QPixmap>

class IMapWatermark
{
public:
    virtual QPixmap getWatermark(const QRectF& bbox, const QRect& screen) = 0;
};

Q_DECLARE_INTERFACE ( IMapWatermark,
                      "com.cbsoft.Merkaartor.IMapWatermark/1.0" )

#endif
