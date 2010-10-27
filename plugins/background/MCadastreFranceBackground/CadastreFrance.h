/***************************************************************************
 *   Copyright (C) 2010 by Chris Browet                                    *
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
#ifndef CADASTREFRANCEADAPTER_H
#define CADASTREFRANCEADAPTER_H

#include "IMapAdapterFactory.h"
#include "IMapAdapter.h"

#include <QLocale>

#include "city.h"

class CadastreFranceAdapter : public IMapAdapter
{
    Q_OBJECT
    Q_INTERFACES(IMapAdapter)

public:
    CadastreFranceAdapter();
    virtual ~CadastreFranceAdapter();

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

    //! returns the type of this MapAdapter
    /*!
     * @return  the type of this MapAdapter
     */
    virtual IMapAdapter::Type	getType		() const;

    //! returns the host of this MapAdapter
    /*!
     * @return  the host of this MapAdapter
     */
    virtual QString	getHost		() const;

    //! returns the size of the tiles
    /*!
     * @return the size of the tiles
     */
    virtual int		getTileSizeW	() const;
    virtual int		getTileSizeH	() const;

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
    virtual int 		getZoom		() const ;

    //! returns the source tag to be applied when drawing over this map
    /*!
     * @return the source tag
     */
    virtual QString	getSourceTag		() const { return ""; }

    //! returns the Url of the usage license
    /*!
     * @return the Url of the usage license
     */
    virtual QString	getLicenseUrl() const {return "";}

    virtual int		getAdaptedZoom() const;
    virtual int 	getAdaptedMinZoom() const;
    virtual int		getAdaptedMaxZoom() const;

    virtual void	zoom_in();
    virtual void	zoom_out();

    virtual bool	isValid(int, int, int) const;
    virtual QString getQuery(int, int, int) const;
    virtual QString getQuery(const QRectF& , const QRectF& , const QRect& ) const;
    virtual QPixmap getPixmap(const QRectF& wgs84Bbox, const QRectF& projBbox, const QRect& size) const ;

    virtual QString projection() const;
    virtual QRectF	getBoundingbox() const;

    virtual bool isTiled() const;
    virtual int getTilesWE(int) const;
    virtual int getTilesNS(int) const;

    virtual QMenu* getMenu() const;

    virtual IImageManager* getImageManager();
    virtual void setImageManager(IImageManager* anImageManager);

    virtual void cleanup();

    virtual bool toXML(QDomElement xParent);
    virtual void fromXML(const QDomElement xParent);
    virtual QString toPropertiesHtml();

    virtual void setSettings(QSettings* aSet);

public slots:
    void onGrabCity();
    void cityTriggered(QAction* act);

private slots:
    void resultsAvailable(QMap<QString,QString> results);

private:
    QLocale loc;
    IImageManager* theImageManager;

    QMenu* theMenu;
    QSettings* theSettings;
    QRectF theCoordBbox;

    int current_zoom;
    int min_zoom;
    int max_zoom;
    QList<qreal> Resolutions;

    QString m_code;
    QString m_department;
    City m_city;

private:
    void initializeCity(QString name);

protected:
    void updateMenu();
};

class CadastreFranceAdapterFactory : public QObject, public IMapAdapterFactory
{
    Q_OBJECT
    Q_INTERFACES(IMapAdapterFactory)

public:
    //! Creates an instance of the actual plugin
    /*!
     * @return  a pointer to the MapAdapter
     */
    IMapAdapter* CreateInstance() {return new CadastreFranceAdapter(); }

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
