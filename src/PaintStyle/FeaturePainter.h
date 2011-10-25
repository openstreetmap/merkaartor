#ifndef MERKAARTOR_FEATUREPAINTER_H_
#define MERKAARTOR_FEATUREPAINTER_H_

#include <QtCore/QString>
#include <QtGui/QColor>
#include <QFont>

#include <QList>
#include <QPair>
#include <QPointF>

#include <Painter.h>
#include "TagSelector.h"

class MapView;
class Feature;
class Projection;
class Relation;
class Way;
class TagSelector;
class Node;
class QPainter;
class QPainterPath;
class QFont;
class QDomElement;

class FeaturePainter : public Painter
{
public:
    FeaturePainter();
    FeaturePainter(const FeaturePainter& f);
    FeaturePainter& operator=(const FeaturePainter& F);
    FeaturePainter(const Painter& f);
    FeaturePainter& operator=(const Painter& F);
    ~FeaturePainter();

    void setSelector(const QString& aName);
    void setSelector(TagSelector* aSelector);
    TagSelectorMatchResult matchesTag(const Feature* F, const MapView* V) const;

    QString toXML(QString filename) const;
    static FeaturePainter fromXML(const QDomElement& e, QString filename);

    virtual void drawBackground(Node *N, QPainter *thePainter, MapView *theView) const;
    virtual void drawBackground(Way* R, QPainter* thePainter, MapView* theView) const;
    virtual void drawBackground(Relation* R, QPainter* thePainter, MapView* theView) const;
    virtual void drawForeground(Node *N, QPainter *thePainter, MapView *theView) const;
    virtual void drawForeground(Way* R, QPainter* thePainter, MapView* theView) const;
    virtual void drawForeground(Relation* R, QPainter* thePainter, MapView* theView) const;
    virtual void drawTouchup(Way* R, QPainter* thePainter, MapView* theView) const;
    virtual void drawTouchup(Node* R, QPainter* thePainter, MapView* theView) const;
    virtual void drawLabel(Way* R, QPainter* thePainter, MapView* theView) const;
    virtual void drawPointLabel(QPointF C, QString str, QString strBG, QPainter* thePainter, MapView* theView) const;
    virtual void drawLabel(Node* Pt, QPainter* thePainter, MapView* theView) const;

public:
    TagSelector* theTagSelector;
};

#endif

