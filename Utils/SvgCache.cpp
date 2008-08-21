#include "SvgCache.h"

#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtGui/QPainter>
#include <QtSvg/QSvgRenderer>

QPixmap getPixmapFromFile(const QString& aName, unsigned int Size)
{
	static QMap<QPair<QString, unsigned int>, QPixmap> Cache;
	QPair<QString, unsigned int> Key(aName,Size); 
	if (!Cache.contains(Key))
	{
		QPixmap result(Size, Size);
		if (Size) {
			if (aName.right(4).toUpper() == ".SVG") {
				result.fill(Qt::transparent);
				QPainter p(&result);
				QSvgRenderer Monet(aName);
				Monet.render(&p,QRectF(0,0,Size,Size));	
			} else {
				result = QPixmap(aName);
				result = result.scaledToWidth(Size);
			}
		} else {
			result = QPixmap(aName);
		}
		Cache[Key] = result;
	}
	return Cache[Key];
}

