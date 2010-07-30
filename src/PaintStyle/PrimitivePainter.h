#ifndef MERKAARTOR_PRIMITIVEPAINTER_H_
#define MERKAARTOR_PRIMITIVEPAINTER_H_

#include <QtCore/QString>
#include <QtGui/QColor>
#include <QFont>

#include <QList>
#include <QPair>
#include <QPointF>

#include <Painter.h>
#include "Utils/TagSelector.h"

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
    TagSelectorMatchResult matchesTag(const IFeature* F, double PixelPerM) const;

    void drawBackground(QPainterPath* R, QPainter* thePainter, double PixelPerM) const;
    void drawForeground(QPainterPath* R, QPainter* thePainter, double PixelPerM) const;
    void drawTouchup(QPainterPath* R, QPainter* thePainter, double PixelPerM) const;
    void drawTouchup(QPointF* R, QPainter* thePainter, double PixelPerM) const;
    void drawLabel(QPainterPath* R, QPainter* thePainter, double PixelPerM, QString str, QString strBg = QString()) const;
    void drawPointLabel(QPointF C, QString str, QString strBG, QPainter* thePainter, double PixelPerM) const;
    void drawLabel(QPointF* Pt, QPainter* thePainter, double PixelPerM, QString str, QString strBg = QString()) const;

public:
    TagSelector* theTagSelector;

private:
    static inline double angToRad(double a)
    {
        return a*M_PI/180.;
    }
};

#endif

