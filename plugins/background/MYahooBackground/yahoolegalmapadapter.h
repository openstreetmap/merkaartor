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
#ifndef YAHOOLEGALMAPADAPTER_H
#define YAHOOLEGALMAPADAPTER_H

#include "IMapAdapterFactory.h"
#include "IMapAdapter.h"

#include <QLocale>

class YahooLegalMapAdapter : public IMapAdapter
{
    Q_OBJECT
    Q_INTERFACES(IMapAdapter)
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    Q_PLUGIN_METADATA(IID "be.merkaartor.YahooLegalMapAdapter" FILE "YahooLegalMapAdapter.json")
#endif

public:
    YahooLegalMapAdapter();
    virtual ~YahooLegalMapAdapter();

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
    virtual int		getTileSizeW	() const { return -1; }
    virtual int		getTileSizeH	() const { return -1; }

    //! returns the min zoom value
    /*!
     * @return the min zoom value
     */
    virtual int 		getMinZoom	(const QRectF &) const { return -1; }

    //! returns the max zoom value
    /*!
     * @return the max zoom value
     */
    virtual int		getMaxZoom	(const QRectF &) const { return -1; }

    //! returns the current zoom
    /*!
     * @return the current zoom
     */
    virtual int 		getZoom		() const { return -1; }

    //! returns the source tag to be applied when drawing over this map
    /*!
     * @return the source tag
     */
    virtual QString	getSourceTag		() const { return "Yahoo"; }
    virtual void setSourceTag (const QString& ) {};

    //! returns the Url of the usage license
    /*!
     * @return the Url of the usage license
     */
    virtual QString	getLicenseUrl() const {return "";}

    virtual int		getAdaptedZoom() const { return -1; }
    virtual int 	getAdaptedMinZoom(const QRectF &) const { return -1; }
    virtual int		getAdaptedMaxZoom(const QRectF &) const { return -1; }

    virtual void	zoom_in() {}
    virtual void	zoom_out() {}

    virtual bool	isValid(int, int, int) const { return true; }
    virtual QString getQuery(int, int, int)  const { return ""; }
    virtual QString getQuery(const QRectF& wgs84Bbox, const QRectF& projBbox, const QRect& size) const ;
    virtual QPixmap getPixmap(const QRectF& /* wgs84Bbox */, const QRectF& /* projBbox */, const QRect& /* size */) const { return QPixmap(); }

    virtual QString projection() const;
    virtual QRectF	getBoundingbox() const {return QRectF();}

    virtual bool isTiled() const { return false; }
    virtual int getTilesWE(int) const { return -1; }
    virtual int getTilesNS(int) const { return -1; }

    virtual QMenu* getMenu() const { return NULL; }

    virtual IImageManager* getImageManager();
    virtual void setImageManager(IImageManager* anImageManager);

    virtual void cleanup() {}

    virtual bool toXML(QXmlStreamWriter& /*stream*/) { return true; }
    virtual void fromXML(QXmlStreamReader& /*xParent*/) {}
    virtual QString toPropertiesHtml() {return "";}

    virtual void setSettings(QSettings* /*aSet*/) {}

private:

    QLocale loc;
    IImageManager* theImageManager;
};

class YahooLegalMapAdapterFactory : public QObject, public IMapAdapterFactory
{
    Q_OBJECT
    Q_INTERFACES(IMapAdapterFactory)

public:
    //! Creates an instance of the actual plugin
    /*!
     * @return  a pointer to the MapAdapter
     */
    IMapAdapter* CreateInstance() {return new YahooLegalMapAdapter(); }

    //! returns the unique identifier (Uuid) of this MapAdapter
    /*!
     * @return  the unique identifier (Uuid) of this MapAdapter
     */
    virtual QUuid	getId		() const;

    //! returns the name of this MapAdapter
    /*!
     * @return  the name of this MapAdapter
     */
    virtual QString	getName		() const;
};

#endif
