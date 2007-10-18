#include "PaintStyle.h"

PaintStyle::~PaintStyle(void)
{
}

CascadingPaintStyleLayer::CascadingPaintStyleLayer()
: Next(0)
{
}

void CascadingPaintStyleLayer::setNextLayer(PaintStyle* n)
{
	Next = n;
}

PaintStyle* CascadingPaintStyleLayer::firstLayer()
{
	return this;
}

PaintStyle* CascadingPaintStyleLayer::nextLayer()
{
	return Next;
}

void EmptyPaintStyle::draw(Road *)
{
}

void EmptyPaintStyle::draw(TrackPoint*)
{
}
