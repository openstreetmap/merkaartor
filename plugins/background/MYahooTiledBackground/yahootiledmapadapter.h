/***************************************************************************
 *   Copyright (C) 2008 by Chris Browet                                    *
 *   cbro@semperpax.com                                                    *
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
#ifndef YAHOOTILEDMAPADAPTER_H
#define YAHOOTILEDMAPADAPTER_H

#include "IMapAdapter.h"

#include <QLocale>

//! MapAdapter for WMS servers
/*!
 * Use this derived MapAdapter to display maps from WMS servers
 *	@author Kai Winter <kaiwinter@gmx.de>
*/
class YahooTiledMapAdapter : public QObject, public IMapAdapter
{
	Q_OBJECT
	Q_INTERFACES(IMapAdapter)

public:
	//! constructor
	/*!
	 * This construct a Yahoo Adapter
	 */
	YahooTiledMapAdapter();
	virtual ~YahooTiledMapAdapter();


	//! returns the unique identifier (Uuid) of this MapAdapter
	/*!
	 * @return  the unique identifier (Uuid) of this MapAdapter
	 */
	virtual QUuid	getId		() const;

	//! returns the type of this MapAdapter
	/*!
	 * @return  the type of this MapAdapter
	 */
	virtual IMapAdapter::Type	getType		() const;

	//! returns the name of this MapAdapter
	/*!
	 * @return  the name of this MapAdapter
	 */
	virtual QString	getName		() const;

	//! returns the host of this MapAdapter
	/*!
	 * @return  the host of this MapAdapter
	 */
	virtual QString	getHost		() const;

	//! returns the size of the tiles
	/*!
	 * @return the size of the tiles
	 */
	virtual int		getTileSize	() const;

	//! returns the min zoom value
	/*!
	 * @return the min zoom value
	 */
	virtual int 		getMinZoom	() const;

	//! returns the max zoom value
	/*!
	 * @return the max zoom value
	 */
	virtual int		getMaxZoom	() const;

	//! returns the current zoom
	/*!
	 * @return the current zoom
	 */
	virtual int 		getZoom		() const;

	virtual int		getAdaptedZoom()   const;
	virtual int 	getAdaptedMinZoom	() const;
	virtual int		getAdaptedMaxZoom	() const;

	virtual void	zoom_in();
	virtual void	zoom_out();

	virtual bool	isValid(int x, int y, int z) const;
	virtual QString getQuery(int x, int y, int z) const;
        virtual QString getQuery(const QRectF& /* wgs84Bbox */, const QRectF& /* projBbox */, const QRect& /* size */) const  { return ""; }
        virtual QPixmap getPixmap(const QRectF& /* wgs84Bbox */, const QRectF& /* projBbox */, const QRect& /* size */) const { return QPixmap(); };

	//! translates a world coordinate to display coordinate
	/*!
	 * The calculations also needs the current zoom. The current zoom is managed by the MapAdapter, so this is no problem.
	 * To divide model from view the current zoom should be moved to the layers.
	 * @param  coordinate the world coordinate
	 * @return the display coordinate (in widget coordinates)
	 */
	virtual QPoint		coordinateToDisplay(const QPointF& coordinate) const;

	//! translates display coordinate to world coordinate
	/*!
	 * The calculations also needs the current zoom. The current zoom is managed by the MapAdapter, so this is no problem.
	 * To divide model from view the current zoom should be moved to the layers.
	 * @param  point the display coordinate
	 * @return the world coordinate
	 */
	virtual QPointF	displayToCoordinate(const QPoint& point) const;

	virtual bool isTiled() const { return true; };
	virtual QString projection() const;

	virtual QMenu* getMenu() const { return NULL; }

	virtual IImageManager* getImageManager();
	virtual void setImageManager(IImageManager* anImageManager);

protected:
	virtual int tilesonzoomlevel(int zoomlevel) const;
	virtual int getyoffset(int y) const;

private:
	QLocale loc;
	IImageManager* theImageManager;

	QString	host;
	QString	serverPath;
	int	tilesize;
	int min_zoom;
	int max_zoom;
	int current_zoom;
	double numberOfTiles;

	virtual QString getQ(QPointF ul, QPointF br) const;
};

#endif
