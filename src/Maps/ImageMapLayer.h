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

	virtual void setVisible(bool b);
	virtual LayerWidget* newWidget(void);
	virtual void updateWidget();
	virtual int size() const;

	virtual bool toXML(QDomElement& xParent, QProgressDialog & progress);
	static ImageMapLayer* fromXML(MapDocument* d, const QDomElement& e, QProgressDialog & progress);

	virtual /* const */ LayerType classType() {return MapLayer::ImageMapLayerType;}
	virtual const LayerGroups classGroups() {return(MapLayer::Default);}

	virtual bool arePointsDrawable() {return false;}

	virtual void drawImage(QPixmap& thePix, Projection& theProjection);

private:
	WMSMapAdapter* wmsa;
	TileMapAdapter* tmsa;

protected:
	ImageMapLayerPrivate* p;

};

#endif


