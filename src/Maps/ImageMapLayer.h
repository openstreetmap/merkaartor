#ifndef IMAGEMAPLAYER_H_
#define IMAGEMAPLAYER_H_

#include "Maps/MapLayer.h"

class ImageMapLayerPrivate;
class Projection;

struct Tile
{
	Tile(int i, int j, double priority)
	    : i(i), j(j), priority(priority)
	{}

	int i, j;
	double priority;

	bool operator<(const Tile& rhs) const { return priority < rhs.priority; }
};

class ImageMapLayer : public OsbMapLayer
{
	Q_OBJECT
public:
    //ImageMapLayer() : layermanager(0) {}
	ImageMapLayer(const QString& aName);
	virtual ~ImageMapLayer();

	void setMapAdapter(const QUuid& theAdapterUid, const QString& server = QString());
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
	virtual void forceRedraw(const Projection& mainProj, QRect rect);
	virtual void draw(const Projection& mainProj, QRect& rect);
	virtual void zoom(double zoom, const QPoint& pos, const QRect& rect);

private:
	void drawTiled(const Projection& mainProj, QRect& rect) const;
	void drawFull(const Projection& mainProj, QRect& rect) const;

protected:
	ImageMapLayerPrivate* p;
};

#endif


