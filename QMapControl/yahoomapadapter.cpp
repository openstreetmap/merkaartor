#include "yahoomapadapter.h"

YahooMapAdapter::YahooMapAdapter()
	: TileMapAdapter("png.maps.yimg.com", "/png?v=3.1.0&x=%2&y=%3&z=%1", 256, 17,1)
{
	int zoom = max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom;
	numberOfTiles = pow(2, zoom+1.0);
}
YahooMapAdapter::YahooMapAdapter(QString host, QString url)
	: TileMapAdapter(host, url, 256, 17,1)
{
	int zoom = max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom;
	numberOfTiles = pow(2, zoom+1.0);
}
YahooMapAdapter::~YahooMapAdapter()
{
}

bool YahooMapAdapter::isValid(int x, int y, int z) const
{
	return true;
}

int YahooMapAdapter::tilesonzoomlevel(int zoomlevel) const
{
	return int(pow(2, zoomlevel+1.0));
}

int YahooMapAdapter::getyoffset(int y) const
{
	int zoom = max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom;

	int tiles = int(pow(2, zoom+0.0));
	y = y*(-1)+tiles-1;
	return int(y);
}
