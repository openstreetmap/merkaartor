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
		QPixmap result(Size,Size);
		QPainter p(&result);
		QSvgRenderer Monet(aName);
		Monet.render(&p,QRectF(0,0,Size,Size));	
		Cache[Key] = result;
	}
	return Cache[Key];
}

