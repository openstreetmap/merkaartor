#include "osmmapadapter.h"

OSMMapAdapter::OSMMapAdapter()
// 	: TileMapAdapter("192.168.8.1", "/img/img_cache.php/%1/%2/%3.png", 256, 0, 17)
	: TileMapAdapter("tile.openstreetmap.org", "/%1/%2/%3.png", 256, 0, 17)
{
}

OSMMapAdapter::~OSMMapAdapter()
{
}


