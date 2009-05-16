/***************************************************************************
 *   Copyright (C) 2007 by Kai Winter   *
 *   kaiwinter@gmx.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "layer.h"
#include "Preferences/MerkaartorPreferences.h"

Layer::Layer(QString layername, IMapAdapter* mapadapter, enum LayerType layertype, bool takeevents)
	:visible(true), layername(layername), layertype(layertype), mapAdapter(mapadapter), takeevents(takeevents), offscreenViewport(QRect(0,0,0,0))
{
// 	qDebug() << "creating new Layer: " << layername << ", type: " << contents;
// 	qDebug() << this->layertype;
}

Layer::~Layer()
{
	//delete mapAdapter;
}

void Layer::setSize(QSize size)
{
	this->size = size;
	screenmiddle = QPoint(size.width()/2, size.height()/2);

}

QString Layer::getLayername() const
{
	return layername;
}

IMapAdapter* Layer::getMapAdapter() const
{
	return mapAdapter;
}

void Layer::setVisible(bool visible)
{
	this->visible = visible;
	emit(updateRequest());
}

bool Layer::isVisible() const
{
	return visible;
}
void Layer::zoomIn() const
{
	mapAdapter->zoom_in();
}
void Layer::zoomOut() const
{
	mapAdapter->zoom_out();
}

void Layer::drawYourImage(QPainter* painter, const QPoint mapmiddle_px) const
{
	if (layertype == MapLayer)
	{
		_draw(painter, mapmiddle_px);
	}
}

void Layer::_draw(QPainter* painter, const QPoint mapmiddle_px) const
{
	// screen middle rotieren...

	int i, j;

	int tilesize = mapAdapter->getTileSize();
	int cross_x = int(mapmiddle_px.x())%tilesize;		// position on middle tile
	int cross_y = int(mapmiddle_px.y())%tilesize;
 	//qDebug() << mapmiddle_px;
 	//qDebug() << screenmiddle << " - " << cross_x << ", " << cross_y;

		// calculate how many surrounding tiles have to be drawn to fill the display
	int space_left = screenmiddle.x() - cross_x;
	int tiles_left = space_left/tilesize;
	if (space_left>0)
		tiles_left+=1;

	int space_above = screenmiddle.y() - cross_y;
	int tiles_above = space_above/tilesize;
	if (space_above>0)
		tiles_above+=1;

	int space_right = screenmiddle.x() - (tilesize-cross_x);
	int tiles_right = space_right/tilesize;
	if (space_right>0)
		tiles_right+=1;

	int space_bottom = screenmiddle.y() - (tilesize-cross_y);
	int tiles_bottom = space_bottom/tilesize;
	if (space_bottom>0)
		tiles_bottom+=1;

// 	int tiles_displayed = 0;
	int mapmiddle_tile_x = mapmiddle_px.x()/tilesize;
	int mapmiddle_tile_y = mapmiddle_px.y()/tilesize;

	const QPoint from =	QPoint((-tiles_left+mapmiddle_tile_x)*tilesize, (-tiles_above+mapmiddle_tile_y)*tilesize);
	const QPoint to =		QPoint((tiles_right+mapmiddle_tile_x+1)*tilesize, (tiles_bottom+mapmiddle_tile_y+1)*tilesize);

	offscreenViewport = QRect(from, to);

	QList<Tile> tiles;

	for (i=-tiles_left+mapmiddle_tile_x; i<=tiles_right+mapmiddle_tile_x; i++)
	{
		for (j=-tiles_above+mapmiddle_tile_y; j<=tiles_bottom+mapmiddle_tile_y; j++)
		{
#ifdef Q_CC_MSVC
			double priority = _hypot(i - mapmiddle_tile_x, j - mapmiddle_tile_y);
#else
			double priority = hypot(i - mapmiddle_tile_x, j - mapmiddle_tile_y);
#endif
			tiles.append(Tile(i, j, priority));
		}
	}

	qSort(tiles);

	for (QList<Tile>::const_iterator tile = tiles.begin(); tile != tiles.end(); ++tile)
	{
		if (mapAdapter->isValid(tile->i, tile->j, mapAdapter->getZoom()))
		{
			QPixmap pm = mapAdapter->getImageManager()->getImage(mapAdapter, tile->i, tile->j, mapAdapter->getZoom());
			if (!pm.isNull())
				painter->drawPixmap(((tile->i-mapmiddle_tile_x)*tilesize)-cross_x+size.width(),
							((tile->j-mapmiddle_tile_y)*tilesize)-cross_y+size.height(),
													pm);

			if (MerkaartorPreferences::instance()->getDrawTileBoundary()) {
				painter->drawRect(((tile->i-mapmiddle_tile_x)*tilesize)-cross_x+size.width(),
						  ((tile->j-mapmiddle_tile_y)*tilesize)-cross_y+size.height(),
											tilesize, tilesize);
			}

// 			if (QCoreApplication::hasPendingEvents())
// 				QCoreApplication::processEvents();
		}
	}

	return;

/* The rest of the code will never be reached. Can it be removed?
	// PREFETCHING
	int upper = mapmiddle_tile_y-tiles_above-1;
	int right = mapmiddle_tile_x+tiles_right+1;
	int left = mapmiddle_tile_x-tiles_right-1;
	int lower = mapmiddle_tile_y+tiles_bottom+1;

	j = upper;
	for (i=left; i<=right; i++)
	{
		if (mapAdapter->isValid(i, j, mapAdapter->getZoom()))
			mapAdapter->getImageManager()->prefetchImage(mapAdapter, i, j, mapAdapter->getZoom());
	}
	j = lower;
	for (i=left; i<=right; i++)
	{
		if (mapAdapter->isValid(i, j, mapAdapter->getZoom()))
			mapAdapter->getImageManager()->prefetchImage(mapAdapter, i, j, mapAdapter->getZoom());
	}
	i = left;
	for (j=upper+1; j<=lower-1; j++)
	{
		if (mapAdapter->isValid(i, j, mapAdapter->getZoom()))
			mapAdapter->getImageManager()->prefetchImage(mapAdapter, i, j, mapAdapter->getZoom());
	}
	i = right;
	for (j=upper+1; j<=lower-1; j++)
	{
		if (mapAdapter->isValid(i, j, mapAdapter->getZoom()))
			mapAdapter->getImageManager()->prefetchImage(mapAdapter, i, j, mapAdapter->getZoom());
	}
*/
}

QRect Layer::getOffscreenViewport() const
{
	return offscreenViewport;
}

Layer::LayerType Layer::getLayertype() const
{
	return layertype;
}

void Layer::setMapAdapter(IMapAdapter* mapadapter)
{
	mapAdapter = mapadapter;
}
