#include "maplayer.h"

IMapLayer::IMapLayer(QString layername, MapAdapter* mapadapter, bool takeevents)
 : Layer(layername, mapadapter, Layer::MapLayer, takeevents)
{
}


IMapLayer::~IMapLayer()
{
}


