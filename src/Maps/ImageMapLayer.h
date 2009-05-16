#ifndef IMAGEMAPLAYER_H_
#define IMAGEMAPLAYER_H_

#include "Maps/MapLayer.h"

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
	virtual void draw(Projection& theProjection, QRect& rect);
	virtual void zoom(double zoom, const QPoint& pos, const QRect& rect);

private:
	TileMapAdapter* tmsa;

protected:
	ImageMapLayerPrivate* p;
};

#endif


