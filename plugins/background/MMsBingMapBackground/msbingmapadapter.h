#ifndef MSLIVEMAP_H
#define MSLIVEMAP_H

#include "mapadapter.h"
#include "IMapAdapterFactory.h"
#include "IMapWatermark.h"

struct BingProvider
{
    QString name;
    int zoomMin;
    int zoomMax;
    QRectF bbox;
};

//! MapAdapter for Ms Bing Maps
/*!
 * This is a conveniece class, which extends and configures a TileMapAdapter
 *	@author Kai Winter <kaiwinter@gmx.de>
*/
class MsBingMapAdapter : public MapAdapter, public IMapWatermark
{
    Q_OBJECT
    Q_INTERFACES(IMapAdapter IMapWatermark)
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    Q_PLUGIN_METADATA(IID "be.merkaartor.msbingmapadapter" FILE "msbingmapadapter.json")
#endif

public:
    //! constructor
    /*!
         * This construct a Google Adapter
         */
    MsBingMapAdapter();
    virtual ~MsBingMapAdapter();

    virtual QPoint		coordinateToDisplay(const QPointF&) const;
    virtual QPointF	displayToCoordinate(const QPoint&) const;

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
    QString	getHost		() const;

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
    virtual int 		getMinZoom	(const QRectF &bbox) const;

    //! returns the max zoom value
    /*!
         * @return the max zoom value
         */
    virtual int		getMaxZoom	(const QRectF &bbox) const;

    //! returns the current zoom
    /*!
         * @return the current zoom
         */
    virtual int 		getZoom		() const;

    //! returns the source tag to be applied when drawing over this map
    /*!
         * @return the source tag
         */
    virtual QString	getSourceTag() const;
    virtual void setSourceTag (const QString& value);

    //! returns the Url of the usage license
    /*!
         * @return the Url of the usage license
         */
    virtual QString	getLicenseUrl() const;

    virtual int		getAdaptedZoom() const;
    virtual int 	getAdaptedMinZoom(const QRectF &bbox) const;
    virtual int		getAdaptedMaxZoom(const QRectF &bbox) const;

    virtual QString getQuery(const QRectF& , const QRectF& , const QRect&) const { return ""; }
    virtual bool isTiled() const { return true; }

    virtual QRectF	getBoundingbox() const;
    virtual int getTilesWE(int zoom) const;
    virtual int getTilesNS(int zoom) const;

    virtual void cleanup() {}

    virtual bool toXML(QXmlStreamWriter& /*stream*/) { return true; }
    virtual void fromXML(QXmlStreamReader& /*xParent*/) {}
    virtual QString toPropertiesHtml() {return "";}

    virtual void setSettings(QSettings* aSet) {theSets = aSet;}

    virtual IImageManager* getImageManager();
    virtual void setImageManager(IImageManager* anImageManager);

    //IMapWatermark
    virtual QString getAttributionsHtml(const QRectF& bbox, const QRect& screen);
    virtual QString getLogoHtml();

protected:
    virtual void zoom_in();
    virtual void zoom_out();
    virtual QString getQuery(int x, int y, int z) const;
    virtual bool isValid(int x, int y, int z) const;
    virtual QPixmap getPixmap(const QRectF& /*wgs84Bbox*/, const QRectF& /*projBbox*/, const QRect& /*size*/) const { return QPixmap(); }
    virtual QMenu* getMenu() const { return NULL; }

private:
    virtual QString getQ(double longitude, double latitude, int zoom) const;
    double getMercatorLatitude(double YCoord) const;
    double getMercatorYCoord(double lati) const;

    IImageManager* theImageManager;
    int srvNum;
    QString theSource;
    bool isLoaded;
    QList<BingProvider> theProviders;
    QSettings* theSets;

private slots:
    void on_adapterDataFinished(QNetworkReply* rply);
};

class MsBingMapAdapterFactory : public QObject, public IMapAdapterFactory
{
    Q_OBJECT
    Q_INTERFACES(IMapAdapterFactory)

public:
    //! Creates an instance of the actual plugin
    /*!
     * @return  a pointer to the MapAdapter
     */
    IMapAdapter* CreateInstance() {return new MsBingMapAdapter(); }

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
