#ifndef MERKAARTOR_PRIMITIVEPAINTER_H_
#define MERKAARTOR_PRIMITIVEPAINTER_H_

#include <QtCore/QString>
#include <QtGui/QColor>
#include <QFont>

#include <QList>
#include <QPair>
#include <QPointF>

#include <Painter.h>
#include "TagSelector.h"

#include "math.h"

class MapView;
class IFeature;
class TagSelector;
class QPainter;
class QPainterPath;
class QFont;
class QDomElement;

class PrimitivePainter : public Painter
{
public:
    PrimitivePainter();
    PrimitivePainter(const PrimitivePainter& f);
    PrimitivePainter& operator=(const PrimitivePainter& F);
    PrimitivePainter(const Painter& f);
    PrimitivePainter& operator=(const Painter& F);
    ~PrimitivePainter();

    void setSelector(const QString& aName);
    void setSelector(TagSelector* aSelector);
    TagSelectorMatchResult matchesTag(const IFeature* F, qreal PixelPerM) const;

    void drawBackground(QPainterPath* R, QPainter* thePainter, qreal PixelPerM) const;
    void drawForeground(QPainterPath* R, QPainter* thePainter, qreal PixelPerM) const;
    void drawTouchup(QPainterPath* R, QPainter* thePainter, qreal PixelPerM) const;
    void drawTouchup(QPointF* R, QPainter* thePainter, qreal PixelPerM) const;
    void drawLabel(QPainterPath* R, QPainter* thePainter, qreal PixelPerM, QString str, QString strBg = QString()) const;
    void drawPointLabel(QPointF C, QString str, QString strBG, QPainter* thePainter, qreal PixelPerM) const;
    void drawLabel(QPointF* Pt, QPainter* thePainter, qreal PixelPerM, QString str, QString strBg = QString()) const;

public:
    TagSelector* theTagSelector;

private:
    static inline qreal angToRad(qreal a)
    {
        return a*M_PI/180.;
    }
};

#endif

