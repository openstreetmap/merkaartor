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
#ifndef MAPADAPTER_H
#define MAPADAPTER_H

#include <QObject>
#include <QSize>
#include <QPoint>
#include <QPointF>
#include <QLocale>
#include <QDebug>
#include <cmath>

#include "IImageManager.h"
#include "imagemanager.h"
#include "IMapAdapter.h"

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
class MapAdapter : public QObject, public IMapAdapter
{
    //friend class ImageManager;
    //friend class BrowserImageManager;
    //friend class Layer;

    Q_OBJECT
    Q_INTERFACES(IMapAdapter)

public:
    virtual ~MapAdapter();

    //! returns the host of this MapAdapter
    /*!
     * @return  the host of this MapAdapter
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

    virtual int		getAdaptedZoom()const;
    virtual int 	getAdaptedMinZoom	() const;
    virtual int		getAdaptedMaxZoom	() const;

    virtual QMenu* getMenu() const { return NULL; }

    virtual IImageManager* getImageManager();
    virtual void setImageManager(IImageManager* anImageManager);

protected:
    QString name;
    MapAdapter(const QString& host, const QString& serverPath, int tilesize, int minZoom = 0, int maxZoom = 0);
    virtual void zoom_in() = 0;
    virtual void zoom_out() = 0;
    virtual bool 		isValid(int x, int y, int z) const = 0;
    virtual QString getQuery(int x, int y, int z) const = 0;

    QSize size;
    QString	host;
    QString	serverPath;
    int		tilesize;
    int min_zoom;
    int max_zoom;
    int current_zoom;

    int param1;
    int param2;
    int param3;
    int param4;
    int param5;
    int param6;

    QString sub1;
    QString sub2;
    QString sub3;
    QString sub4;
    QString sub5;
    QString sub6;

    int order[3][2];

    int middle_x;
    int middle_y;

    QLocale loc;

    IImageManager* theImageManager;
};

#endif
