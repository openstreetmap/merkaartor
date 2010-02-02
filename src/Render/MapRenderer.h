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

#include "PaintStyle/PaintStyle.h"

#include <QPainter>
#include <QTransform>
#include <QList>

#include "Feature.h"

class Document;
class PaintStylePrivate;
class MapRenderer;

class PaintStyleLayer
{
public:
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
	BackgroundStyleLayer(MapRenderer* ar)
		: PaintStyleLayer(ar) {}
	virtual void draw(Way* R);
	virtual void draw(Node* Pt);
	virtual void draw(Relation* R);
};

class ForegroundStyleLayer : public PaintStyleLayer
{
public:
	ForegroundStyleLayer(MapRenderer* ar)
		: PaintStyleLayer(ar) {}
	virtual void draw(Way* R);
	virtual void draw(Node* Pt);
	virtual void draw(Relation* R);
};

class TouchupStyleLayer : public PaintStyleLayer
{
public:
	TouchupStyleLayer(MapRenderer* ar)
		: PaintStyleLayer(ar) {}
	virtual void draw(Way* R);
	virtual void draw(Node* Pt);
	virtual void draw(Relation* R);
};

class LabelStyleLayer : public PaintStyleLayer
{
public:
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
			QMap<RenderPriority, QSet <Feature*> > theFeatures,
			MapView* aView
	);

	MapView* theView;
	QPainter* thePainter;
};

#endif // MAPRENDERER_H
