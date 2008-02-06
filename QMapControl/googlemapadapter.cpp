#include "googlemapadapter.h"

GoogleMapAdapter::GoogleMapAdapter()
	: TileMapAdapter("mt2.google.com", "/mt?n=404&x=%2&y=%3&zoom=%1", 256, 17, 0)
// 	: TileMapAdapter("tile.openstreetmap.org", "/%1/%2/%3.png", 256, 0, 17)
{
}

GoogleMapAdapter::~GoogleMapAdapter()
{
}


