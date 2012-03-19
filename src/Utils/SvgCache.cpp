#include "SvgCache.h"

#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtGui/QPainter>
#include <QtSvg/QSvgRenderer>

QImage* getSVGImageFromFile(const QString& aName, int Size)
{
    static QMap<QPair<QString, int>, QImage> Cache;
    QPair<QString, int> Key(aName,Size);
    if (!Cache.contains(Key))
    {
        QImage result(Size, Size, QImage::Format_ARGB32_Premultiplied);
        result.fill(Qt::transparent);
        if (Size) {
            if (aName.right(4).toUpper() == ".SVG") {
                QPainter p(&result);
                QSvgRenderer Monet(aName);
                Monet.render(&p,QRectF(0,0,Size,Size));
            } else {
                result = QImage(aName);
                result = result.scaledToWidth(Size);
            }
        }
        Cache[Key] = result;
    }
    return &(Cache[Key]);
}

