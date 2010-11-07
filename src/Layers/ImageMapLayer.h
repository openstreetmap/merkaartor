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

class ImageMapLayer : public Layer
{
    Q_OBJECT
public:
    //ImageMapLayer() : layermanager(0) {}
    ImageMapLayer(const QString& aName);
    virtual ~ImageMapLayer();

    IMapAdapter* getMapAdapter();
    void setMapAdapter(const QUuid& theAdapterUid, const QString& server = QString());
    QString projection() const;

    virtual void setVisible(bool b);
    virtual LayerWidget* newWidget(void);
    virtual void updateWidget();
    CoordBox boundingBox();
    virtual int size() const;

    virtual bool toXML(QDomElement& xParent, bool asTemplate, QProgressDialog * progress);
    virtual QString toPropertiesHtml();
    static ImageMapLayer* fromXML(Document* d, const QDomElement& e, QProgressDialog * progress);

    virtual /* const */ LayerType classType() const {return Layer::ImageLayerType;}
    virtual const LayerGroups classGroups() const {return(Layer::Map);}

    virtual bool arePointsDrawable() {return false;}

    virtual void drawImage(QPainter* P);
    virtual void forceRedraw(MapView& theView, QRect rect);
    virtual void draw(MapView& theView, QRect& rect);

    virtual void pan(QPoint delta);
    virtual void zoom(double zoom, const QPoint& pos, const QRect& rect);
    virtual void zoom_in();
    virtual void zoom_out();
    virtual int getCurrentZoom();
    virtual void setCurrentZoom(MapView& theView, const CoordBox& viewport, const QRect& rect);
    virtual qreal pixelPerM();

    bool isTiled();

    IImageManager* getImageManger();
private:
    QRect drawTiled(MapView& theView, QRect& rect);
    QRect drawFull(MapView& theView, QRect& rect);

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


