//
// C++ Interface: MapRenderer
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MAPRENDERER_H
#define MAPRENDERER_H

#include "FeaturePainter.h"

#include <QPainter>
#include <QTransform>
#include <QList>

#include "Feature.h"
#include "IRenderer.h"

class Document;
class PaintStylePrivate;
class MapRenderer;

class PaintStyleLayer
{
public:
    PaintStyleLayer() {}
    PaintStyleLayer(MapRenderer* ar) { r = ar; }
    virtual void draw(Way* R) = 0;
    virtual void draw(Node* Pt) = 0;
    virtual void draw(Relation* R) = 0;

protected:
    MapRenderer* r;
};

class BackgroundStyleLayer : public PaintStyleLayer
{
public:
    BackgroundStyleLayer() {}
    BackgroundStyleLayer(MapRenderer* ar)
        : PaintStyleLayer(ar) {}
    virtual void draw(Way* R);
    virtual void draw(Node* Pt);
    virtual void draw(Relation* R);
};

class ForegroundStyleLayer : public PaintStyleLayer
{
public:
    ForegroundStyleLayer() {}
    ForegroundStyleLayer(MapRenderer* ar)
        : PaintStyleLayer(ar) {}
    virtual void draw(Way* R);
    virtual void draw(Node* Pt);
    virtual void draw(Relation* R);
};

class TouchupStyleLayer : public PaintStyleLayer
{
public:
    TouchupStyleLayer() {}
    TouchupStyleLayer(MapRenderer* ar)
        : PaintStyleLayer(ar) {}
    virtual void draw(Way* R);
    virtual void draw(Node* Pt);
    virtual void draw(Relation* R);
};

class LabelStyleLayer : public PaintStyleLayer
{
public:
    LabelStyleLayer() {}
    LabelStyleLayer(MapRenderer* ar)
        : PaintStyleLayer(ar) {}
    virtual void draw(Way* R);
    virtual void draw(Node* Pt);
    virtual void draw(Relation* R);
};

class MapRenderer
{
public:
    MapRenderer();

    void render(
            QPainter* P,
            const QMap<RenderPriority, QSet <Feature*> >& theFeatures,
            const QRectF& pViewport,
            const QRect& screen,
            const qreal pixelPerM,
            const RendererOptions& options
    );
//    void print(
//            QPainter* P,
//            QMap<RenderPriority, QSet <Feature*> > theFeatures,
//            const RendererOptions& options,
//            MapView* aView
//    );

    CoordBox theViewport;
    QRect theScreen;
    QTransform theTransform;
    qreal thePixelPerM;
    qreal NodeWidth;

    QPainter* thePainter;
    RendererOptions theOptions;
    GlobalPainter theGlobalPainter;

    QPoint toView(Node *aPt) const;

protected:
    BackgroundStyleLayer bglayer;
    ForegroundStyleLayer fglayer;
    TouchupStyleLayer tchuplayer;
    LabelStyleLayer lbllayer;

};

#endif // MAPRENDERER_H
