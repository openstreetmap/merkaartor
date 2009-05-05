#ifndef IMAGEMAPLAYER_H_
#define IMAGEMAPLAYER_H_

#include "Maps/MapLayer.h"
#include <geometry/geometries/cartesian2d.hpp>
#include <geometry/geometries/latlong.hpp>
#include <geometry/projections/projection.hpp>

class ImageMapLayerPrivate;
class Projection;

class ImageMapLayer : public OsbMapLayer
{
	Q_OBJECT
public:
    //ImageMapLayer() : layermanager(0) {}
	ImageMapLayer(const QString& aName, LayerManager* aLayerMgr=NULL);
	virtual ~ImageMapLayer();

	Layer* imageLayer();
	void setMapAdapter(const QUuid& theAdapterUid);
	LayerManager* layermanager;
	QString projection() const;

	virtual void setVisible(bool b);
	virtual LayerWidget* newWidget(void);
	virtual void updateWidget();
	virtual int size() const;

	virtual bool toXML(QDomElement& xParent, QProgressDialog & progress);
	static ImageMapLayer* fromXML(MapDocument* d, const QDomElement& e, QProgressDialog & progress);

	virtual /* const */ LayerType classType() {return MapLayer::ImageMapLayerType;}
	virtual const LayerGroups classGroups() {return(MapLayer::Default);}

	virtual bool arePointsDrawable() {return false;}

	virtual void drawImage(QPixmap& thePix, QPoint delta);
	virtual void forceRedraw(Projection& theProjection, QRect rect);
	virtual void drawWMS(Projection& theProjection, QRect& rect);
	virtual void zoom(double zoom, const QPoint& pos, const QRect& rect);

private:
	WMSMapAdapter* wmsa;
	TileMapAdapter* tmsa;
	projection::projection<geometry::point_ll_deg, geometry::point_2d> *theProj;

protected:
	ImageMapLayerPrivate* p;

private slots:
	void requestFinished(int id, bool error);

signals:
	void imageReceived();
};

#endif


