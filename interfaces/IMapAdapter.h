#ifndef IMAPADAPTER_H
#define IMAPADAPTER_H

#include <QObject>
#include <QString>
#include <QPoint>
#include <QPointF>
#include <QUuid>
#include <QMenu>
#include <QXmlStreamWriter>
#include <QDomElement>
#include <QSettings>

class IFeature;
class IProjection;
class IImageManager;

class IMapAdapter : public QObject
{
    Q_OBJECT

public:
    enum Type
    {
        NetworkBackground,
        NetworkDataBackground,
        BrowserBackground,
        DirectBackground,
        VectorBackground
    };

    virtual ~IMapAdapter() {}

    //! returns the unique identifier (Uuid) of this MapAdapter
    /*!
     * @return  the unique identifier (Uuid) of this MapAdapter
     */
    virtual QUuid	getId		() const = 0;

    //! returns the name of this MapAdapter
    /*!
     * @return  the name of this MapAdapter
     */
    virtual QString	getName		() const = 0;

    //! returns the type of this MapAdapter
    /*!
     * @return  the type of this MapAdapter
     */
    virtual IMapAdapter::Type	getType		() const = 0;

    //! returns the host of this MapAdapter
    /*!
     * @return  the host of this MapAdapter
     */
    virtual QString	getHost		() const = 0;

    //! returns the size of the tiles
    /*!
     * @return the size of the tiles
     */
    virtual int		getTileSizeW	() const = 0;
    virtual int		getTileSizeH	() const = 0;

    //! returns the min zoom value
    /*!
     * @return the min zoom value
     */
    virtual int 		getMinZoom	(const QRectF &) const = 0;

    //! returns the max zoom value
    /*!
     * @return the max zoom value
     */
    virtual int		getMaxZoom	(const QRectF &) const = 0;

    //! returns the current zoom
    /*!
     * @return the current zoom
     */
    virtual int 		getZoom		() const = 0;

    //! returns the source tag to be applied when drawing over this map
    /*!
     * @return the source tag
     */
    virtual QString	getSourceTag		() const = 0;
    virtual void setSourceTag (const QString& value) = 0;

    //! returns the Url of the usage license
    /*!
     * @return the Url of the usage license
     */
    virtual QString	getLicenseUrl() const = 0;

    virtual int		getAdaptedZoom()   const = 0;
    virtual int 	getAdaptedMinZoom	(const QRectF &) const = 0;
    virtual int		getAdaptedMaxZoom	(const QRectF &) const = 0;

    virtual void	zoom_in() = 0;
    virtual void	zoom_out() = 0;

    virtual bool	isValid(int x, int y, int z) const = 0;
    virtual QString getQuery(int x, int y, int z) const = 0;
    virtual QString getQuery(const QRectF& wgs84Bbox, const QRectF& projBbox, const QRect& size) const = 0;
    virtual QPixmap getPixmap(const QRectF& wgs84Bbox, const QRectF& projBbox, const QRect& size) const = 0;
    virtual const QList<IFeature*>* getPaths(const QRectF& /*wgs84Bbox*/, const IProjection* /*projection*/) const { return NULL; }

    virtual QString projection() const = 0;
    virtual QRectF	getBoundingbox() const = 0;

    virtual bool isTiled() const = 0;
    virtual int getTilesWE(int zoom) const = 0;
    virtual int getTilesNS(int zoom) const = 0;

    virtual QMenu* getMenu() const = 0;

    virtual IImageManager* getImageManager() = 0;
    virtual void setImageManager(IImageManager* anImageManager) = 0;

    virtual void cleanup() = 0;

    virtual bool toXML(QXmlStreamWriter& ) = 0;
    virtual void fromXML(const QDomElement xParent) = 0;
    virtual QString toPropertiesHtml() = 0;

    virtual void setSettings(QSettings* aSet) = 0;

signals:
    void forceRefresh();
    void forceZoom();
    void forceProjection();

};

Q_DECLARE_INTERFACE ( IMapAdapter,
                      "com.cbsoft.Merkaartor.IMapAdapter/1.12" )

#endif
