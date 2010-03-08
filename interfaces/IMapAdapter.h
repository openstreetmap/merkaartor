#ifndef IMAPADAPTER_H
#define IMAPADAPTER_H

#include <QObject>
#include <QString>
#include <QPoint>
#include <QPointF>
#include <QUuid>
#include <QMenu>

#include "IImageManager.h"

//! Used to fit map servers into QMapControl
/*!
 * MapAdapters are needed to convert between world- and display coordinates.
 * This calculations depend on the used map projection.
 * There are two ready-made MapAdapters:
 *  - TileMapAdapter, which is ready to use for OpenStreetMap or Google (Mercator projection)
 *  - WMSMapAdapter, which could be used for the most WMS-Server (some servers show errors, because of image ratio)
 *
 * MapAdapters are also needed to form the HTTP-Queries to load the map tiles.
 * The maps from WMS Servers are also divided into tiles, because those can be better cached.
 *
 * @see TileMapAdapter, @see WMSMapAdapter
 *
 *	@author Kai Winter <kaiwinter@gmx.de>
*/
class IMapAdapter
{
public:
	enum Type
	{
		NetworkBackground,
		BrowserBackground,
		DirectBackground
	};

	virtual ~IMapAdapter() {};

	//! returns the unique identifier (Uuid) of this MapAdapter
	/*!
	 * @return  the unique identifier (Uuid) of this MapAdapter
	 */
	virtual QUuid	getId		() const = 0;

	//! returns the type of this MapAdapter
	/*!
	 * @return  the type of this MapAdapter
	 */
	virtual IMapAdapter::Type	getType		() const = 0;

	//! returns the name of this MapAdapter
	/*!
	 * @return  the name of this MapAdapter
	 */
	virtual QString	getName		() const = 0;

	//! returns the host of this MapAdapter
	/*!
	 * @return  the host of this MapAdapter
	 */
	virtual QString	getHost		() const = 0;

	//! returns the size of the tiles
	/*!
	 * @return the size of the tiles
	 */
	virtual int		getTileSize	() const = 0;

	//! returns the min zoom value
	/*!
	 * @return the min zoom value
	 */
	virtual int 		getMinZoom	() const = 0;

	//! returns the max zoom value
	/*!
	 * @return the max zoom value
	 */
	virtual int		getMaxZoom	() const = 0;

	//! returns the current zoom
	/*!
	 * @return the current zoom
	 */
	virtual int 		getZoom		() const = 0;

	virtual int		getAdaptedZoom()   const = 0;
	virtual int 	getAdaptedMinZoom	() const = 0;
	virtual int		getAdaptedMaxZoom	() const = 0;

	virtual void	zoom_in() = 0;
	virtual void	zoom_out() = 0;

	virtual bool	isValid(int x, int y, int z) const = 0;
	virtual QString getQuery(int x, int y, int z) const = 0;
	virtual QString getQuery(const QRectF& wgs84Bbox, const QRectF& projBbox, const QRect& size) const = 0;
	virtual QPixmap getPixmap(const QRectF& wgs84Bbox, const QRectF& projBbox, const QRect& size) const = 0;

	//! translates a world coordinate to display coordinate
	/*!
	 * The calculations also needs the current zoom. The current zoom is managed by the MapAdapter, so this is no problem.
	 * To divide model from view the current zoom should be moved to the layers.
	 * @param  coordinate the world coordinate
	 * @return the display coordinate (in widget coordinates)
	 */
	virtual QPoint		coordinateToDisplay(const QPointF& coordinate) const = 0;

	//! translates display coordinate to world coordinate
	/*!
	 * The calculations also needs the current zoom. The current zoom is managed by the MapAdapter, so this is no problem.
	 * To divide model from view the current zoom should be moved to the layers.
	 * @param  point the display coordinate
	 * @return the world coordinate
	 */
	virtual QPointF	displayToCoordinate(const QPoint& point) const = 0;

	virtual QRectF	getBoundingbox() const = 0;

	virtual bool isTiled() const = 0;
	virtual QString projection() const = 0;

	virtual QMenu* getMenu() const = 0;

	virtual IImageManager* getImageManager() = 0;
	virtual void setImageManager(IImageManager* anImageManager) = 0;
};

Q_DECLARE_INTERFACE ( IMapAdapter,
					  "com.cbsoft.Merkaartor.IMapAdapter/1.4" )

#endif
