#ifndef IMAGEMAPLAYER_H_
#define IMAGEMAPLAYER_H_

#include "Layer.h"

#define NONE_ADAPTER_UUID QUuid("{8F5D3625-F987-45c5-A50B-17D88384F97D}")
#define SHAPE_ADAPTER_UUID QUuid("{AFB0324E-34D0-4267-BB8A-CF56CD2D7012}")
#define WMS_ADAPTER_UUID QUuid("{E238750A-AC27-429e-995C-A60C17B9A1E0}")
#define TMS_ADAPTER_UUID QUuid("{CA8A07EC-A466-462b-929F-3805BC9DEC95}")
#define BING_ADAPTER_UUID QUuid("2a888701-1a93-4040-9b34-1e5339f67f43}")

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

    virtual bool toXML(QXmlStreamWriter& stream, bool asTemplate, QProgressDialog * progress);
    virtual QString toPropertiesHtml();
    static ImageMapLayer* fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * progress);

    virtual /* const */ LayerType classType() const {return Layer::ImageLayerType;}
    virtual const LayerGroups classGroups() const {return(Layer::Map);}

    virtual void drawImage(QPainter* P);
    virtual void forceRedraw(MapView& theView, QTransform& aTransform, QRect rect);
    virtual void draw(MapView& theView, QRect& rect);

    virtual void pan(QPoint delta);
    virtual void zoom(double zoom, const QPoint& pos, const QRect& rect);
    virtual void zoom_in();
    virtual void zoom_out();
    virtual int getCurrentZoom();
    virtual void setCurrentZoom(MapView& theView, const CoordBox& viewport, const QRect& rect);
    virtual qreal pixelPerCoord();
    virtual QTransform getCurrentAlignmentTransform();

    bool isTiled();

    IImageManager* getImageManger();
    virtual void setEnabled(bool b);
    void resetAlign();

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


