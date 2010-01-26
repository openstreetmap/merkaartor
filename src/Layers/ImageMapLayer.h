#ifndef IMAGEMAPLAYER_H_
#define IMAGEMAPLAYER_H_

#include "Layer.h"

class MapView;
class ImageMapLayerPrivate;
class Projection;
class IImageManager;

struct Tile
{
	Tile(int i, int j, double priority)
		: i(i), j(j), priority(priority)
	{}

	int i, j;
	double priority;

	bool operator<(const Tile& rhs) const { return priority < rhs.priority; }
};

class ImageMapLayer : public OsbLayer
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
	static ImageMapLayer* fromXML(Document* d, const QDomElement& e, QProgressDialog & progress);

	virtual /* const */ LayerType classType() {return Layer::ImageLayerType;}
	virtual const LayerGroups classGroups() {return(Layer::Default);}

	virtual bool arePointsDrawable() {return false;}

	virtual void drawImage(QPixmap& thePix);
	virtual void forceRedraw(MapView& theView, QRect rect, QPoint delta);
	virtual void draw(MapView& theView, QRect& rect);
	virtual void zoom(double zoom, const QPoint& pos, const QRect& rect);

	IImageManager* getImageManger();
private:
	QRect drawTiled(MapView& theView, QRect& rect) const;
	QRect drawFull(MapView& theView, QRect& rect) const;

signals:
	void imageRequested(ImageMapLayer*);
	void imageReceived(ImageMapLayer*);
	void loadingFinished(ImageMapLayer*);

private slots:
	void on_imageRequested();
	void on_imageReceived();
	void on_loadingFinished();

protected:
	ImageMapLayerPrivate* p;
};

#endif


