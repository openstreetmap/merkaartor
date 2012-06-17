#include "SvgCache.h"

#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtGui/QPainter>
#include <QtSvg/QSvgRenderer>
#include <QFileInfo>
#include <QDebug>

QImage* getSVGImageFromFile(const QString& aName, int Size)
{
    static QMap<QPair<QString, int>, QImage> Cache;
    QPair<QString, int> Key(aName,Size);
    if (!Cache.contains(Key))
    {
        QFileInfo fi(aName);
        if (fi.suffix().toUpper() == "SVG") {
            if (!Size)
                Size = 16;
            QImage result(Size, Size, QImage::Format_ARGB32_Premultiplied);
            result.fill(Qt::transparent);
            QPainter p(&result);
            QSvgRenderer Monet(aName);
            Monet.render(&p,QRectF(0,0,Size,Size));
            Cache[Key] = result;
        } else {
            QImage result(aName);
            if (Size)
                result = result.scaledToWidth(Size, Qt::SmoothTransformation);
            Cache[Key] = result;
        }
    }
    return &(Cache[Key]);
}

