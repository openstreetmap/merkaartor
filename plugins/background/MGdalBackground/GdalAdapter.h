//***************************************************************
// CLass: %CLASS%
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//******************************************************************

#ifndef GDALADAPTER_H
#define GDALADAPTER_H

#include "IMapAdapter.h"

#include <QLocale>

class GDALDataset;
class GDALColorTable;

class GdalImage
{
public:
	QPixmap theImg;
	double adfGeoTransform[6];
};

class GdalAdapter : public QObject, public IMapAdapter
{
	Q_OBJECT
	Q_INTERFACES(IMapAdapter)

public:
	enum TiffType
	{
		Unknown,
		Rgb,
		Rgba,
		Palette_Gray,
		Palette_RGBA,
		Palette_CMYK,
		Palette_HLS
	};

	GdalAdapter();
	virtual ~GdalAdapter();

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
	virtual int		getTileSize	() const { return -1; }

	//! returns the min zoom value
	/*!
	 * @return the min zoom value
	 */
	virtual int 		getMinZoom	() const { return -1; }

	//! returns the max zoom value
	/*!
	 * @return the max zoom value
	 */
	virtual int		getMaxZoom	() const { return -1; }

	//! returns the current zoom
	/*!
	 * @return the current zoom
	 */
	virtual int 		getZoom		() const { return -1; }

	virtual int		getAdaptedZoom() const { return -1; }
	virtual int 	getAdaptedMinZoom() const { return -1; }
	virtual int		getAdaptedMaxZoom() const { return -1; }

	virtual void	zoom_in() {}
	virtual void	zoom_out() {}

	virtual bool	isValid(int, int, int) const { return true; }
	virtual QString getQuery(int, int, int)  const { return ""; }
	virtual QString getQuery(const QRectF& , const QRectF& , const QRect& ) const { return ""; }
	virtual QPixmap getPixmap(const QRectF& wgs84Bbox, const QRectF& projBbox, const QRect& size) const ;

	//! translates a world coordinate to display coordinate
	/*!
	 * The calculations also needs the current zoom. The current zoom is managed by the MapAdapter, so this is no problem.
	 * To divide model from view the current zoom should be moved to the layers.
	 * @param  coordinate the world coordinate
	 * @return the display coordinate (in widget coordinates)
	 */
	virtual QPoint		coordinateToDisplay(const QPointF& ) const { return QPoint(); }

	//! translates display coordinate to world coordinate
	/*!
	 * The calculations also needs the current zoom. The current zoom is managed by the MapAdapter, so this is no problem.
	 * To divide model from view the current zoom should be moved to the layers.
	 * @param  point the display coordinate
	 * @return the world coordinate
	 */
	virtual QPointF	displayToCoordinate(const QPoint& )  const { return QPointF(); }

	virtual QRectF	getBoundingbox() const;

	virtual bool isTiled() const { return false; }
	virtual QString projection() const;

	virtual QMenu* getMenu() const;

	virtual IImageManager* getImageManager();
	virtual void setImageManager(IImageManager* anImageManager);

public slots:
	void onLoadImage();

private:
	QMenu* theMenu;

	GDALDataset       *poDataset;
	QString imageFilename;
	QString theProjection;
	QRect thePicRect;
	QRectF theBbox;

	QList<GdalImage> theImages;

//	TiffType theType;
//	int bandCount;
//	int ixR, ixG, ixB, ixA;
//	GDALColorTable * colTable;
};

#endif // GDALADAPTER_H
