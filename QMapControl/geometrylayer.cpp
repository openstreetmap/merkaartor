#include "geometrylayer.h"

GeometryLayer::GeometryLayer(QString layername, MapAdapter* mapadapter, bool takeevents)
	: Layer(layername, mapadapter, Layer::GeometryLayer, takeevents)
{
}


GeometryLayer::~GeometryLayer()
{
}


